#pragma once

#include <drogon/HttpFilter.h>
#include "Utils.hpp"
using namespace drogon;

class JwtAuthFilter : public HttpFilter<JwtAuthFilter>
{
public:
    static constexpr bool isAutoCreation = false;

    void doFilter(const HttpRequestPtr &req,
                  FilterCallback &&fcb,
                  FilterChainCallback &&fccb) override
    {
        auto authHeader = req->getHeader("Authorization");
        if (authHeader.empty() || authHeader.size() < 8 || authHeader.substr(0, 7) != "Bearer ")
        {
            auto res = HttpResponse::newHttpResponse();
            res->setStatusCode(k401Unauthorized);
            res->setBody("Missing or invalid Authorization header");
            fcb(res);
            return;
        }

        auto token = authHeader.substr(7);

        try
        {
            auto decoded = Utils::verifyJwt(token);

            if (decoded->has_payload_claim("sub"))
            {
                req->getAttributes()->insert("userId", decoded->get_payload_claim("sub").as_string());
            }

            fccb();
        }
        catch (const std::exception &e)
        {
            auto res = HttpResponse::newHttpResponse();
            res->setStatusCode(k401Unauthorized);
            res->setBody(std::string("Invalid token: ") + e.what());
            fcb(res);
        }
    }
};

