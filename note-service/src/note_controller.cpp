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
    TemplateParser::unwrapParseError(std::move(error));

    const auto dbClient = drogon::app().getDbClient();

    dbClient->execSqlAsync("INSERT INTO notes(user_id, title, content) VALUES($1, $2, $3)", 
        [callback = std::move(callback)](const drogon::orm::Result& result)
        {
            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->setStatusCode(drogon::k201Created);
            resp->setBody("Note created successfully");
            callback(resp);
        },
        [callback = std::move(callback)](const drogon::orm::DrogonDbException& ex)
        {
            spdlog::error("Database error: {}", ex.base().what());
            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->setStatusCode(drogon::k500InternalServerError);
            resp->setBody("Error creating note");
            callback(resp);
        },
        body.userId,
        body.title,
        body.content);
}

void NoteController::readNote(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string noteId)
{
    /*
    redisReply* reply = static_cast<redisReply*>(redisCommand(m_redis, "HGETALL note:%s", noteId.c_str()));
    if (reply == nullptr || reply->elements == 0)
    {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k404NotFound);
        resp->setBody("Note not found");
        callback(resp);
    }

    Json::Value note;
    for (size_t i = 0; i < reply->elements; i += 2)
    {
        const std::string key = reply->element[i]->str;
        const std::string value = reply->element[i + 1]->str;
        note[key] = value;
    }
    freeReplyObject(reply);

    Json::StreamWriterBuilder writer;
    const auto resp = drogon::HttpResponse::newHttpJsonResponse(note);
    callback(resp);
    */
}

int64_t NoteController::currentTimestamp() const
{
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}
