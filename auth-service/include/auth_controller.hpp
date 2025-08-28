#pragma once
#include <argon2.h>
#include <drogon/HttpController.h>
#include <hiredis/hiredis.h>
#include <librdkafka/rdkafkacpp.h>

struct User 
{
    std::string email;
    std::string password;
    std::string role;
    BOOST_DESCRIBE_CLASS(User, (), (email, password, role), (), ())
};

class AuthController : public drogon::HttpController<AuthController> 
{
public:
    AuthController();
public:
    METHOD_LIST_BEGIN
        ADD_METHOD_TO(AuthController::registerUser, "/register", drogon::Post);
    METHOD_LIST_END
    void registerUser(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback);

private:
    redisContext* m_redis;
    std::unique_ptr<RdKafka::Producer> m_kafkaProducer;
    const std::string m_kafkaTopic = "auth-topic";
};