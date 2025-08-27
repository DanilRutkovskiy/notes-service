#include <config.hpp>
#include <drogon/drogon.h>
#include <Utils.hpp>

int main()
{
    spdlog::set_level(spdlog::level::info);
    
    drogon::app()
        .registerFilter(std::make_shared<Utils::ExceptionCatcher>())
        .addListener(Config::authServiceHost.data(), Config::authServicePort)
        .setThreadNum(std::thread::hardware_concurrency() - 1)
        .loadConfigFile("./auth-service-drogon-db-config.json")
        .run();

    return 0;
}