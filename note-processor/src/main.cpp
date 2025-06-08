#include <librdkafka/rdkafkacpp.h>
#include <pqxx/pqxx>
#include <clickhouse/client.h>
#include <spdlog/spdlog.h>
#include <json/json.h>
#include <memory>

int main()
{
    std::string err;
    auto conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    conf->set("bootstrap.servers", "kafka:9092", err);
    conf->set("group.id", "note-processor-group", err);
    auto consumer = std::unique_ptr<RdKafka::KafkaConsumer>(RdKafka::KafkaConsumer::create(conf, err));
    if (!consumer)
    {
        spdlog::error("Kafka consumer creation failed: {}", err);
        return 1;
    }

    consumer->subscribe({"notes-topic"});

    pqxx::connection conn("host= postgres port=5432 dbname=notes-db user=user password=password");
    if (!conn.is_open())
    {
        spdlog::error("PotgreSQL connection failed");
        return 1;
    }

    clickhouse::Client clickhouseClient(clickhouse::ClientOptions().SetHost("clickhouse").SetPort(9000));

    while(true)
    {
        auto msg = consumer->consume(1000);
        if (msg->err() == RdKafka::ERR_NO_ERROR)
        {
            std::string payload(static_cast<char*>(msg->payload()), msg->len());
            Json::Value json;
            Json::Reader reader;
            reader.parse(payload, json);

            //TODO - make enum of actions
            std::string action = json["action"].asString();
            if (action == "create")
            {
                std::string noteId = json["note_id"].asString();
                std::string userId = json["user_id"].asString();
                std::string title = json["title"].asString();
                std::string content = json["content"].asString();

                pqxx::work tx(conn);
                tx.exec("INSERT INTO notes (id, user_id, title, content) "
                        "VALUES ($1, $2, $3, $4)", {noteId, userId, title, content});

                tx.commit();

                clickhouse::Block block;
                //block.AppendColumn("user_id", std::make_shared<clickhouse::ColumnUInt64>(static_cast<uint64_t>(std::stoul(userId))));
                //block.AppendColumn("event_type", std::make_shared<clickhouse::ColumnString>("create"));
                //block.AppendColumn("timestamp", std::make_shared<clickhouse::ColumnUInt32>(static_cast<uint32_t>(std::time(nullptr))));

                clickhouseClient.Insert("note_analytics", block);

                spdlog::info("Processed note id: {}", noteId);
            }
        }

        delete msg;
        consumer->commitSync();
    }

    return 0;
}