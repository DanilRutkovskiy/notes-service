#pragma once
#include <argon2.h>
#include <vector>
#include <random>
#include <regex>
#include <drogon/HttpController.h>

namespace Utils
{
    inline std::string hashPassword(const std::string& password)
    {
        const uint32_t t_cost = 3;
        const uint32_t m_cost = 1 << 16;
        const uint32_t parallelism = 1;

        std::vector<uint8_t> salt(16);
        std::random_device rd;
        for (auto& b : salt) 
        {
            b = static_cast<uint8_t>(rd());
        }

        char encoded[128]; 
        int result = argon2id_hash_encoded
        (
            t_cost, 
            m_cost, 
            parallelism, 
            password.data(), 
            password.size(),
            salt.data(), salt.size(),
            32, // hash length
            encoded, 
            sizeof(encoded)
        );

        if (result != ARGON2_OK) 
        {
            throw std::runtime_error(argon2_error_message(result));
        }

        return std::string(encoded);
    }

    inline bool verifyPassword(const std::string &hash, const std::string &password) 
    {
        int result = argon2id_verify(hash.c_str(), password.data(), password.size());
        return result == ARGON2_OK;
    }

    inline bool isValidEmail(const std::string& email)
    {
        static const std::regex emailRegex
        (
            R"(^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,63}$)"
        );

        return std::regex_match(email, emailRegex);
    }
    
    class ExceptionCatcher : public drogon::HttpFilterBase
    {
    public:
        static constexpr bool isAutoCreation = false;

        void doFilter(const drogon::HttpRequestPtr& req, drogon::FilterCallback &&fcb, drogon::FilterChainCallback &&fccb) override 
        {
            try 
            {
                fccb();
            }
            catch (const drogon::orm::DrogonDbException& ex)
            {
                spdlog::error("Database exception: {}", ex.base().what());
                auto resp = drogon::HttpResponse::newHttpResponse();
                resp->setBody(std::format("Database exception: {}", *ex.base().what()));
                resp->setStatusCode(drogon::k500InternalServerError);
                fcb(resp);
            }
            catch (const std::exception &ex)
            {
                spdlog::error("Common exception: {}", ex.what());
                auto resp = drogon::HttpResponse::newHttpResponse();
                resp->setStatusCode(drogon::k500InternalServerError);
                resp->setBody(std::format("Common exception: {}", ex.what()));
                fcb(resp);
            } 
            catch (...) 
            {
                spdlog::error("Unknown exception");
                auto resp = drogon::HttpResponse::newHttpResponse();
                resp->setStatusCode(drogon::k500InternalServerError);
                resp->setBody("Unknown exception");
                fcb(resp);
            }
        }
    };
    
}