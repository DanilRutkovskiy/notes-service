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

    if (!Utils::Email::isValidEmail(user.email))
    {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k400BadRequest);
        resp->setBody("Not a valid email");
        callback(std::move(resp));
        return;
    }

    auto userId = drogon::utils::getUuid();
    std::string passwordHash = Utils::Password::hashPassword(user.password);

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

void AuthController::loginUser(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    const auto body = req->getJsonObject();
    auto email = (*body)["email"].as<std::string>();
    auto password = (*body)["password"].as<std::string>();

    if (!Utils::Email::isValidEmail(email))
    {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k400BadRequest);
        resp->setBody("Not a valid email");
        callback(std::move(resp));
        return;
    }

    auto cb = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));
    auto transaction = drogon::app().getDbClient()->newTransaction();

    transaction->execSqlAsync
    (
        "SELECT id, password_hash FROM users WHERE email = $1",
        [cb, transaction, password = std::move(password)](const drogon::orm::Result& result)
        {
            if (result.empty())
            {
                auto resp = drogon::HttpResponse::newHttpResponse();
                resp->setStatusCode(drogon::k404NotFound);
                resp->setBody("User does not exists");
                (*cb)(resp);
                return;
            }

            auto passwordHash = result[0]["password_hash"].as<std::string>();
            if (!Utils::Password::verifyPassword(passwordHash, password))
            {
                auto resp = drogon::HttpResponse::newHttpResponse();
                resp->setStatusCode(drogon::k401Unauthorized);
                resp->setBody("Wrong password");
                (*cb)(resp);
                return;
            }

            auto userId = result[0]["id"].as<std::string>();

            auto accessToken = Utils::Jwt::generateJwt(userId, Utils::Jwt::TokenType::ACCESS);
            auto refreshToken = Utils::Jwt::generateJwt(userId, Utils::Jwt::TokenType::REFRESH, std::chrono::hours{24});

            transaction->execSqlAsync
            (
                "INSERT INTO refresh_tokens(user_id, token_hash) "
                "VALUES($1, $2) ON CONFLICT(user_id) DO UPDATE SET token_hash = EXCLUDED.token_hash;",
                [](const drogon::orm::Result& result){},
                [](const drogon::orm::DrogonDbException& ex){},
                userId,
                Utils::Password::hashPassword(refreshToken)
            );

            Json::Value respJson;
            respJson["accessToken"] = accessToken;
            respJson["refreshToken"] = accessToken;

            auto resp = drogon::HttpResponse::newHttpJsonResponse(respJson);
            resp->setStatusCode(drogon::k200OK);
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
        std::move(email)
    );
}

void AuthController::refreshToken(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    auto json = req->getJsonObject();
    if (!json || !(*json)["refresh_token"].isString())
    {
        auto resp = drogon::HttpResponse::newHttpJsonResponse(Json::Value("Missing refresh_token"));
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    auto refreshToken = (*json)["refresh_token"].asString();

    std::string userId;
    try
    {
        auto verifiedToken = Utils::Jwt::verifyJwt(refreshToken);
        if (verifiedToken.type != Utils::Jwt::TokenType::REFRESH)
        {
            throw std::runtime_error(std::format("Expected refresh token, got: {}", toString(verifiedToken.type)));
        }
        userId = verifiedToken.decoded.get_payload_claim("sub").as_string();
    }
    catch(const std::exception& e)
    {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k401Unauthorized);
        resp->setBody(std::format("Error verifying refresh token: {}", e.what()));
        callback(resp);
        return;
    }

    auto cb = std::make_shared<std::function<void(const drogon::HttpResponsePtr &)>>(std::move(callback));
    auto transaction = drogon::app().getDbClient()->newTransaction();

    transaction->execSqlAsync
    (
        "SELECT token_hash, user_id FROM refresh_tokens WHERE user_id = $1",
        [cb, transaction, refreshToken = std::move(refreshToken), userId = std::move(userId)]
        (const drogon::orm::Result& result)
        {
            if (result.empty())
            {
                auto resp = drogon::HttpResponse::newHttpResponse();
                resp->setStatusCode(drogon::k404NotFound);
                resp->setBody("Refresh token does not exists");
                (*cb)(resp);
                return;
            }

            auto tokenHash = result[0]["token_hash"].as<std::string>();

            if (!Utils::Password::verifyPassword(tokenHash, refreshToken))
            {
                auto resp = drogon::HttpResponse::newHttpResponse();
                resp->setStatusCode(drogon::k401Unauthorized);
                resp->setBody("Error refresh token verification");
                (*cb)(resp);
                return;
            }

            auto accessToken = Utils::Jwt::generateJwt(userId, Utils::Jwt::TokenType::ACCESS);
            auto refreshToken = Utils::Jwt::generateJwt(userId, Utils::Jwt::TokenType::REFRESH, std::chrono::hours{24});

            transaction->execSqlAsync
            (
                "INSERT INTO refresh_tokens(user_id, token_hash) "
                "VALUES($1, $2) ON CONFLICT(user_id) DO UPDATE SET token_hash = EXCLUDED.token_hash;",
                [](const drogon::orm::Result& result){},
                [](const drogon::orm::DrogonDbException& ex){},
                userId,
                Utils::Password::hashPassword(refreshToken)
            );

            Json::Value respJson;
            respJson["accessToken"] = accessToken;
            respJson["refreshToken"] = refreshToken;

            auto resp = drogon::HttpResponse::newHttpJsonResponse(respJson);
            resp->setStatusCode(drogon::k200OK);
            (*cb)(resp);
        },
        [cb](const drogon::orm::DrogonDbException& ex)
        {
            spdlog::error("Database error: {}", ex.base().what());
            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->setBody(std::format("Database error: {}", ex.base().what()));
            resp->setStatusCode(drogon::k500InternalServerError);
            (*cb)(resp);
        },
        std::move(userId)
    );
}
