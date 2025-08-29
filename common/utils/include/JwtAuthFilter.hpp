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
            if (!decoded)
            {
                auto res = HttpResponse::newHttpResponse();
                res->setStatusCode(k401Unauthorized);
                res->setBody("Token verification error");
                fcb(res);
                return;
            }

            if (!decoded->has_payload_claim("exp"))
            {
                auto res = HttpResponse::newHttpResponse();
                res->setStatusCode(k401Unauthorized);
                res->setBody("Expiration date is not set");
                fcb(res);
                return;
            }

            auto expirationDate = decoded->get_payload_claim("exp").as_date();
            auto currentDate = std::chrono::system_clock::now();
            if (currentDate > expirationDate)
            {
                auto res = HttpResponse::newHttpResponse();
                res->setStatusCode(k401Unauthorized);
                res->setBody("Token expired");
                fcb(res);
                return;
            }

            if (!decoded->has_payload_claim("sub"))
            {
                auto res = HttpResponse::newHttpResponse();
                res->setStatusCode(k401Unauthorized);
                res->setBody("Can not extract userId fron verification token");
                fcb(res);
                return;
            }

            req->getAttributes()->insert("userId", decoded->get_payload_claim("sub").as_string());

            fccb();
        }
        catch (const std::exception &e)
        {
            auto res = HttpResponse::newHttpResponse();
            res->setStatusCode(k401Unauthorized);
            res->setBody(std::format("Invalid token: {}", e.what()));
            fcb(res);
        }
        catch (...)
        {
            auto res = HttpResponse::newHttpResponse();
            res->setStatusCode(k401Unauthorized);
            res->setBody("Unknown error reading authorization token");
            fcb(res);
        }
    }
};

