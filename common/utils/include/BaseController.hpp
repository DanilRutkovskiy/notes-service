#pragma once

#include <drogon/HttpController.h>
#include <string>

template<typename T>
class BaseController : public drogon::HttpController<T>
{
protected:
    std::string getUserId(const drogon::HttpRequestPtr &req)
    {
        return req->getAttributes()->get<std::string>("userId"); 
    }

private:
    int64_t currentTimestamp() const
    {
        using namespace std::chrono;
        return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    }
};