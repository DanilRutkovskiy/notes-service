#pragma once

#include "Concepts.hpp"

namespace TemplateParser
{
    class ParseError final : public std::exception
    {
    private:
        std::vector<ParseError> subErrors;
        std::optional<std::string> error;

    public:
        ParseError() = default;

        template<typename T> requires std::constructible_from<std::string, T>
        ParseError(T &&strError)
            : error{std::forward<T>(strError)}
        {
        }

        ParseError &addSubError(ParseError &&subError)
        {
            subErrors.emplace_back(std::move(subError));
            return *this;
        }

        template<typename T> requires Concepts::isContainer<T> && (!Concepts::isString<T>)
        ParseError &addSubErrors(T &&newSubErrors)
        {
            std::copy_if(std::move_iterator(std::begin(newSubErrors)),
                         std::move_iterator(std::end(newSubErrors)),
                         std::back_inserter(subErrors),
                         [](const auto &error) {return error;});

            return *this;
        }

        operator bool() const {return error.has_value() || !subErrors.empty();}

        const char *what() const noexcept override
        {
            return error.has_value() ? error.value().data() : "success";
        }

        std::string fullWhat() const
        {
            if (subErrors.empty())
                return what();

            std::queue<std::pair<size_t, std::reference_wrapper<const ParseError>>> queueErrors;
            queueErrors.emplace(0, *this);

            std::string fullError;
            while (!queueErrors.empty())
            {
                const auto &[level, currentError] = queueErrors.front();
                const auto &subErrors = currentError.get().subErrors;
                fullError += '\n';
                std::for_each(subErrors.begin(), subErrors.end(), [newLevel = level + 1, &queueErrors](const auto &error)
                    {
                        queueErrors.emplace(newLevel, error);
                    });
                std::fill_n(std::back_inserter(fullError), level, '-');
                fullError += '>';
                fullError += currentError.get().what();
                queueErrors.pop();
            }

            return fullError;
        }

        void unwrap()
        {
            *this = ParseError();
        }
    };

    inline void unwrapParseError(ParseError &&error)
    {
        if (error)
        {
            std::runtime_error ex{error.fullWhat().c_str()};
            error.unwrap();
            throw(ex);
        }
    }
}
