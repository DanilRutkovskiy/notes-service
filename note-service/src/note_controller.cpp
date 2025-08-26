#include "note_controller.h"

#include <drogon/HttpResponse.h>
#include <config.hpp>
#include <TemplateParser.hpp>

NoteController::NoteController()
{
    m_redis = redisConnect(Config::redisHost.data(), Config::redisPort);
    if (m_redis == nullptr || m_redis->err)
    {
        spdlog::error("Redis connection error: {}", m_redis ? m_redis->errstr : "context is not allocated");
        throw std::runtime_error("Redis connection failed");
    }
    redisCommand(m_redis, "AUTH password");//TODO - change to real authentication

    std::string err;
    auto conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    conf->set("bootstrap.servers", Config::kafkaConnection.data(), err);
    m_kafkaProducer = std::unique_ptr<RdKafka::Producer>(RdKafka::Producer::create(conf, err));
    if (!m_kafkaProducer)
    {
        spdlog::error("Kafka producer creation failed: {}", err);
        throw std::runtime_error("Kafka producer creation failed");
    }
}

void NoteController::createNote(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    const auto json = req->getJsonObject();

    if (!json)
    {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k400BadRequest);
        resp->setBody("Invalid JSON");
        callback(resp);
        return;
    }

    PostBody body;
    auto error = TemplateParser::parse(*json, body);
    if(error)
    {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k400BadRequest);
        resp->setBody(error.fullWhat());
        callback(resp);
        return;
    }

    auto cb = std::make_shared<std::function<void(const drogon::HttpResponsePtr&)>>(std::move(callback));

    const auto dbClient = drogon::app().getDbClient();

    dbClient->execSqlAsync("INSERT INTO notes(id, user_id, title, content) VALUES($1, $2, $3, $4) "
                           "RETURNING id", 
        [cb](const drogon::orm::Result& result)
        {
            Json::Value json;
            json["id"] = result[0]["id"].as<std::string>();
            auto resp = drogon::HttpResponse::newHttpJsonResponse(std::move(json));
            resp->setStatusCode(drogon::k201Created);
            (*cb)(resp);
        },
        [cb](const drogon::orm::DrogonDbException& ex)
        {
            spdlog::error("Database error: {}", ex.base().what());
            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->setStatusCode(drogon::k500InternalServerError);
            resp->setBody("Error creating note");
            (*cb)(resp);
        },
        drogon::utils::getUuid(),
        body.userId,
        body.title,
        body.content
    );
}

void NoteController::readNote(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string&& noteId)
{
    auto cb = std::make_shared<std::function<void(const drogon::HttpResponsePtr&)>>(std::move(callback));
    const auto dbClient = drogon::app().getDbClient();
    dbClient->execSqlAsync("SELECT user_id, title, content FROM notes WHERE id = $1",
        [cb, noteId](const drogon::orm::Result& result)
        {
            Json::Value json;
            if(result.empty())
            {
                auto resp = drogon::HttpResponse::newHttpResponse();
                resp->setStatusCode(drogon::k404NotFound);
                resp->setBody(std::format("Can not find a note with id = {}", noteId));
                (*cb)(resp);
            }

            const auto& record = result[0];
            json = TemplateParser::toJson(PostBody::fromSqlRecord(record));
            auto resp = drogon::HttpResponse::newHttpJsonResponse(std::move(json));
            resp->setStatusCode(drogon::k200OK);
            (*cb)(resp);
        },
        [cb](const drogon::orm::DrogonDbException& ex)
        {
            spdlog::error("Database error: {}", ex.base().what());
            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->setStatusCode(drogon::k500InternalServerError);
            resp->setBody("Error creating note");
            (*cb)(resp);
        },
        std::move(noteId)
    );
}

void NoteController::updateNote(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string&& noteId)
{
    const auto dbClient = drogon::app().getDbClient();
    const auto json = req->getJsonObject();
    
    std::string sql = "UPDATE notes SET ";
    std::string value;

    if (json->getMemberNames().size() > 1)
    {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k400BadRequest);
        resp->setBody("Can not update more than one parameter at a time");
        callback(resp);
        return;
    }

    if(json->isMember("title"))
    {
        sql += " title = $1 ";
        value = (*json)["title"].as<std::string>();
    }

    if(json->isMember("content"))
    {
        sql += " content = $1 ";
        value = (*json)["content"].as<std::string>();
    }

    sql += " WHERE id = $2 RETURNING id";

    auto cb = std::make_shared<std::function<void(const drogon::HttpResponsePtr&)>>(std::move(callback));

    dbClient->execSqlAsync(std::move(sql), 
    [cb, noteId](const drogon::orm::Result& result)
        {
            Json::Value json;
            if(result.empty())
            {
                auto resp = drogon::HttpResponse::newHttpResponse();
                resp->setStatusCode(drogon::k404NotFound);
                resp->setBody(std::format("Can not find a note with id = {}", noteId));
                (*cb)(resp);
                return;
            }
            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->setStatusCode(drogon::k200OK);
            (*cb)(resp);
        }, 
        [cb](const drogon::orm::DrogonDbException& ex)
        {
            spdlog::error("Database error: {}", ex.base().what());
            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->setStatusCode(drogon::k500InternalServerError);
            resp->setBody("Error updating note");
            (*cb)(resp);
        }, 
        std::move(value), 
        std::move(noteId));
}

void NoteController::deleteNote(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string &&noteId)
{
    const auto dbClient = drogon::app().getDbClient();
    const auto json = req->getJsonObject();

    auto cb = std::make_shared<std::function<void(const drogon::HttpResponsePtr&)>>(std::move(callback));

    dbClient->execSqlAsync("DELETE FROM notes WHERE id = $1", 
    [cb, noteId](const drogon::orm::Result& result)
        {
            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->setStatusCode(drogon::k200OK);
            (*cb)(resp);
        }, 
        [cb](const drogon::orm::DrogonDbException& ex)
        {
            spdlog::error("Database error: {}", ex.base().what());
            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->setStatusCode(drogon::k500InternalServerError);
            resp->setBody("Error updating note");
            (*cb)(resp);
        }, 
        std::move(noteId));
}

int64_t NoteController::currentTimestamp() const
{
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}
