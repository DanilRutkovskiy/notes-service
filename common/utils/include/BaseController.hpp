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
};