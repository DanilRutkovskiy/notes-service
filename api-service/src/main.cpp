
#include <drogon/drogon.h>
//#include <prometheus/exposer.h>
#include <spdlog/spdlog.h>
//#include <prometheus/registry.h>
//#include <prometheus/counter.h>


int main()
{
    //prometheus::Exposer exposer{"0.0.0.0:9091"};
    //const auto registry = std::make_shared<prometheus::Registry>();
    //auto& counter = prometheus::BuildCounter().Name("requests_total").Help("Total HTTP requests").Register(*registry);
    //exposer.RegisterCollectable(registry);

    spdlog::set_level(spdlog::level::info);

    drogon::app().addListener("0.0.0.0", 8080).setThreadNum(4).run();

    return 0;
}