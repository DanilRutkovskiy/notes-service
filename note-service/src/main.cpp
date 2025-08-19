
#include <drogon/drogon.h>
#include <config.hpp>
//#include <prometheus/exposer.h>
//#include <prometheus/registry.h>
//#include <prometheus/counter.h>


int main()
{
    //prometheus::Exposer exposer{"0.0.0.0:9091"};
    //const auto registry = std::make_shared<prometheus::Registry>();
    //auto& counter = prometheus::BuildCounter().Name("requests_total").Help("Total HTTP requests").Register(*registry);
    //exposer.RegisterCollectable(registry);

    spdlog::set_level(spdlog::level::info);
    
    drogon::app()
    .addListener(Config::noteServiceHost.data(), Config::noteServicePort)
    .setThreadNum(std::thread::hardware_concurrency() - 1)
    .loadConfigFile("./drogon-config.json")
    .run();

    return 0;
}