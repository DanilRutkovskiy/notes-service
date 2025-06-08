#pragma once

#include <drogon/HttpController.h>
#include <hiredis/hiredis.h>
#include <librdkafka/rdkafkacpp.h>
#include <string>

class NoteController : public drogon::HttpController<NoteController>
{
public:
    NoteController();
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(NoteController::createNote, "/notes/create", drogon::Post);
    ADD_METHOD_TO(NoteController::readNote, "/notes/read/{id}", drogon::Get);
    METHOD_LIST_END

    void createNote(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    void readNote(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback);

private:
    redisContext* m_redis;
    std::unique_ptr<RdKafka::Producer> m_kafkaProducer;
    const std::string m_kafkaTopic = "notes-topic";
};