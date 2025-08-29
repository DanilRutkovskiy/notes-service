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
        ADD_METHOD_TO(NoteController::createNote, "/notes", drogon::Post, "JwtAuthFilter");
        ADD_METHOD_TO(NoteController::readNote, "/notes/{id}", drogon::Get, "JwtAuthFilter");
        ADD_METHOD_TO(NoteController::updateNote, "/notes/{id}", drogon::Patch, "JwtAuthFilter");
        ADD_METHOD_TO(NoteController::deleteNote, "/notes/{id}", drogon::Delete, "JwtAuthFilter");
    METHOD_LIST_END

    void createNote(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    void readNote(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback, std::string&& noteId);
    void updateNote(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback, std::string&& noteId);
    void deleteNote(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback, std::string&& noteId);

private:
    int64_t currentTimestamp() const;

private:
    redisContext* m_redis;
    std::unique_ptr<RdKafka::Producer> m_kafkaProducer;
    const std::string m_kafkaTopic = "notes-topic";

    struct PostBody
    {
        std::string title;
        std::string content;


        static PostBody fromSqlRecord(const drogon::orm::Row& row)
        {
            PostBody result;
            result.content = row["content"].as<std::string>();
            result.title = row["title"].as<std::string>();

            return result;
        }

        BOOST_DESCRIBE_CLASS(PostBody, (),(title, content),(),());
    };
};