#include "auth_controller.hpp"
#include <TemplateParser.hpp>

void AuthController::registerUser(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    Json::Value json = *req->getJsonObject();
    User user;
    auto error = TemplateParser::parse(json, user);
    if (error) {
        Json::Value respJson;
        respJson["status"] = "error";
        respJson["message"] = error.fullWhat();
        auto resp = drogon::HttpResponse::newHttpJsonResponse(respJson);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    uuid_t uuid;
    uuid_generate_random(uuid);
    char uuid_str[37];
    uuid_unparse(uuid, uuid_str);
    std::string userId = uuid_str;
    //std::string passwordHash = bcrypt::generateHash(user.password);

    auto dbClient = drogon::app().getDbClient();
    dbClient->execSqlAsync(
        "INSERT INTO users (id, email, password_hash, role, is_active, created_at, updated_at) VALUES ($1, $2, $3, $4, $5, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP)",
        [callback = std::move(callback)](const drogon::orm::Result& result) {
            Json::Value json;
            json["status"] = "success";
            json["message"] = "User registered";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(json);
            resp->setStatusCode(drogon::k201Created);
            callback(resp);
        },
        [callback = std::move(callback)](const drogon::orm::DrogonDbException& ex) {
            spdlog::error("Database error: {}", ex.base().what());
            Json::Value json;
            json["status"] = "error";
            json["message"] = "Error registering user";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(json);
            resp->setStatusCode(drogon::k500InternalServerError);
            callback(resp);
        },
        userId, user.email, passwordHash, user.role, true
    );
}