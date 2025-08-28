#include "auth_controller.hpp"
#include <TemplateParser.hpp>
#include <Utils.hpp>
#include <config.hpp>

AuthController::AuthController()
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

void AuthController::createUser(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    Json::Value json = *req->getJsonObject();
    User user;
    auto error = TemplateParser::parse(json, user);
    if (error) 
    {
        Json::Value respJson = error.fullWhat();
        auto resp = drogon::HttpResponse::newHttpJsonResponse(std::move(respJson));
        resp->setStatusCode(drogon::k400BadRequest);
        callback(std::move(resp));
        return;
    }

    if (!Utils::isValidEmail(user.email))
    {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k400BadRequest);
        resp->setBody("Not a valid email");
        callback(std::move(resp));
        return;
    }

    auto userId = drogon::utils::getUuid();
    std::string passwordHash = Utils::hashPassword(user.password);

    auto cb = std::make_shared<std::function<void(const drogon::HttpResponsePtr&)>>(std::move(callback));

    auto transaction = drogon::app().getDbClient()->newTransaction();
    transaction->execSqlAsync
    (
        "INSERT INTO users (id, email, password_hash, role, is_active, created_at, updated_at) " 
        "VALUES ($1, $2, $3, $4, $5, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)",
        [cb](const drogon::orm::Result& result) 
        {
            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->setBody("User registered");
            resp->setStatusCode(drogon::k201Created);
            (*cb)(resp);
        },
        [cb](const drogon::orm::DrogonDbException& ex) 
        {
            spdlog::error("Database error: {}", ex.base().what());
            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->setBody("Error registering user");
            resp->setStatusCode(drogon::k500InternalServerError);
            (*cb)(resp);
        },
        userId, user.email, passwordHash, user.role, true
    );
}