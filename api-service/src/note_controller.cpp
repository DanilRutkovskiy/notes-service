#include "note_controller.h"

#include <drogon/HttpResponse.h>
#include <json/json.h>
#include <spdlog/spdlog.h>

NoteController::NoteController()
{
    m_redis = redisConnect("redis", 6379);
    if (m_redis == nullptr || m_redis->err)
    {
        spdlog::error("Redis connection error: {}", m_redis ? m_redis->errstr : "context is not allocated");
        throw std::runtime_error("Redis connection failed");
    }
    redisCommand(m_redis, "AUTH password");//TODO - change to real authentication

    std::string err;
    auto conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    conf->set("bootstrap.servers", "kafka:9092", err);
    m_kafkaProducer = std::unique_ptr<RdKafka::Producer>(RdKafka::Producer::create(conf, err));
    if (!m_kafkaProducer)
    {
        spdlog::error("Kafka producer creation failed: {}", err);
        throw std::runtime_error("Kafka producer creation failed");
    }
}

void NoteController::createNote(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
}

void NoteController::readNote(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
}
