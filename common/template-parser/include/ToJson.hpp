#pragma once

#include "TemplateParser.hpp"

namespace TemplateParser
{
    template<typename T>
    struct ToJsonImpl;


    template<typename T>
    inline Json::Value toJson(T &&src)
    {
        return ToJsonImpl<std::remove_const_t<std::remove_reference_t<T>>>::doToJson(std::forward<T>(src));
    }

    template<typename T> requires (boost::describe::has_describe_members<T>::value)
    struct ToJsonImpl<T>
    {
        using D1 = boost::describe::describe_members<T, boost::describe::mod_public | boost::describe::mod_protected>;
        using parentD1 = boost::describe::describe_bases<T, boost::describe::mod_public>;

        template<typename U>
        static Json::Value doToJson(U &&src)
        {
            Json::Value obj;

            boost::mp11::mp_for_each<parentD1>([&](auto D)
            {
                using B = typename decltype(D)::type;
                const auto json = toJson(static_cast<const B &>(src));
                for (const auto& key : json.getMemberNames())
                {
                    obj[key] = json[key];
                }
            });

            boost::mp11::mp_for_each<D1>([&](auto D)
            {
                obj[D.name] = toJson(std::forward<U>(src).*D.pointer);
            });
            return obj;
        }
    };

    template<typename T> requires(std::is_constructible_v<Json::Value, T> && !std::is_enum_v<T>)
    struct ToJsonImpl<T>
    {
        template<typename U>
        static Json::Value doToJson(U &&src)
        {
            return Json::Value{std::forward<U>(src)};
        }
    };

    template<typename T> requires Concepts::isContainer<T> && (!Concepts::isString<T>)
    struct ToJsonImpl<T>
    {
        static Json::Value doToJson(const T &src)
        {
            Json::Value array;
            for (const auto& element : src)
                array.append(toJson(element));

            return array;
        }
    };

    template<typename T>
    struct ToJsonImpl<std::optional<T>>
    {
        template<typename U>
        static Json::Value doToJson(U &&opt)
        {
            return opt.has_value()
                ? Json::Value{toJson(std::forward<U>(opt).value())}
                : Json::Value{Json::nullValue};
        }
    };

    template<typename T> requires(std::is_enum_v<T>)
    struct ToJsonImpl<T>
    {
        template<typename U>
        static Json::Value doToJson(U &&src)
        {
            const auto enumString = toString(std::forward<U>(src));
            return toJson(enumString);
        }
    };

    template<typename T, typename ... Args>
    struct ToJsonImpl<ParserType::CustomType<T, Args...>>
    {
        template<typename U>
        static Json::Value doToJson(U &&src)
        {
            return toJson(*src);
        }
    };

    template<typename ... Args>
    struct ToJsonImpl<std::variant<Args...>>
    {
        template<typename U>
        static Json::Value doToJson(U&& src)
        {
            return std::visit([](auto&& src){return toJson(std::forward<decltype(src)>(src));}, std::forward<U>(src));
        }
    };

    template<typename T> requires Concepts::hasToString<T>
    struct ToJsonImpl<T>
    {
        template<typename U>
        static Json::Value doToJson(U &&src)
        {
            return Json::Value(src.toString());
        }
    };

    template<typename First, typename Second>
    struct ToJsonImpl<std::pair<First, Second>>
    {
        template<typename U>
        static Json::Value doToJson(U &&src)
        {
            Json::Value obj;
            obj[toString(src.first)] = toJson(src.second);
            return obj;
        }
    };
}
