#include <config.hpp>
#include <drogon/drogon.h>

int main()
{
    spdlog::set_level(spdlog::level::info);
    
    drogon::app()
    .addListener(Config::authServiceHost.data(), Config::authServicePort)
    .setThreadNum(std::thread::hardware_concurrency() - 1)
    .loadConfigFile("./auth-service-drogon-db-config.json")
    .run();

    return 0;
}