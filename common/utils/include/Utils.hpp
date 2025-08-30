#pragma once
#include <argon2.h>
#include <vector>
#include <random>
#include <regex>
#include <jwt-cpp/jwt.h>
#include <jwt-cpp/traits/kazuho-picojson/traits.h>
#include <config.hpp>

namespace Utils
{
    namespace DateTime
    {
        inline int64_t currentTimestamp()
        {
            using namespace std::chrono;
            return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        }
    }

    namespace Password
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
    }

    namespace Email
    {
        inline bool isValidEmail(const std::string& email)
        {
            static const std::regex emailRegex
            (
                R"(^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,63}$)"
            );

            return std::regex_match(email, emailRegex);
        }
    }
    
    namespace Jwt
    {
        enum class TokenType
        {
            ACCESS,
            REFRESH
        };

        inline std::string toString(TokenType type)
        {
            switch(type)
            {
                case TokenType::ACCESS: return "access";
                case TokenType::REFRESH: return "refresh";
            }
        }

        inline bool fromString(const std::string& strType, TokenType& enumType)
        {
            if(toString(TokenType::ACCESS) == strType) return enumType = TokenType::ACCESS, true;
            if(toString(TokenType::REFRESH) == strType) return enumType = TokenType::REFRESH, true;

            return false;
        }

        inline std::string generateJwt(const std::string &userId, TokenType type, const std::chrono::hours duration = std::chrono::hours{1})
        {
                auto token = jwt::create<jwt::traits::kazuho_picojson>()
                .set_type("JWT")
                .set_issuer("auth-service")
                .set_payload_claim("type", jwt::traits::kazuho_picojson::value_type{toString(type)})
                .set_subject(userId)
                .set_issued_at(std::chrono::system_clock::now())
                .set_expires_at(std::chrono::system_clock::now() + duration)
                .sign(jwt::algorithm::hs256{Config::jwtSecretKey.data()});

            return token;
        }

        struct DecodedToken
        {
            TokenType type = TokenType::ACCESS;
            jwt::decoded_jwt<jwt::traits::kazuho_picojson> decoded;
        };

        inline DecodedToken verifyJwt(const std::string &token)
        {
            auto decoded = jwt::decode<jwt::traits::kazuho_picojson>(token);

            auto verifier = jwt::verify<jwt::traits::kazuho_picojson>()
                .allow_algorithm(jwt::algorithm::hs256{Config::jwtSecretKey.data()})
                .with_issuer("auth-service");

            verifier.verify(decoded);

            if (!decoded.has_payload_claim("sub"))
            {
                throw std::runtime_error("User id is not present in token");
            }

            TokenType tokenType;
            if (!decoded.has_payload_claim("type") || 
                !fromString(decoded.get_payload_claim("type").as_string(), tokenType))
            {
                throw std::runtime_error("Error getting token type");
            }

            DecodedToken result{.type = tokenType, .decoded = std::move(decoded)};

            return result;
        }
    }//namespace Jwt
}//namespace Utils