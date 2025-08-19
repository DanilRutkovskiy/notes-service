#pragma once

#include <json/json.h>

namespace TemplateParser::Concepts
{
    /**
     * @brief Мета функция, которая проверяет можно ли вызвать у экземпляра типа `T`
     *  метод begin().
     * @value `value` равно `true`, если возможно произвести вызов begin(). И `false`
     * во всех остальных случаях
     */
    template<typename T>
    concept isString = requires(T a)
    {
        a.begin();
        a.end();
        a == "Hello";
    };

    template<typename T>
    concept isContainer = requires(T a)
    {
        a.begin();
        a.end();
    };

    template<typename T>
    concept hasEmpty = requires(T a)
    {
        a.empty();
    };

    template<typename T>
    struct isOptional : public std::false_type {};

    template<typename T>
    struct isOptional<std::optional<T>> : public std::true_type{};

    template<typename T>
    struct isVariant : public std::false_type {};

    template<typename ...Args>
    struct isVariant<std::variant<Args...>> : public std::true_type{};

    template <typename T>
    concept isNumber = std::is_arithmetic_v<T> && !std::is_same_v<T, bool>;

    template<typename T>
    concept hasToLongLong = requires(T a)
    {
        a.toLongLong((bool *)0);
    };

    template<typename T>
    concept hasToInt = requires(T a)
    {
        a.toInt((bool *)0);
    };

    template<typename T>
    concept hasToUShort = requires(T a)
    {
        a.toUShort((bool *)0);
    };

    template<typename T>
    concept hasToFloat = requires(T a)
    {
        a.toFloat((bool *)0);
    };

    template<typename T>
    concept hasToString = requires(T a)
    {
        a.toString();
    };

    template<typename T>
    concept hasToBool = requires(T a)
    {
        a.toBool();
    };

    template<typename T>
    concept hasIsNull = requires(T a)
    {
        a.isNull();
    };

    template<typename T>
    concept hasToVariant = requires(T a)
    {
        a.toVariant();
    };

    template<typename T>
    concept isConvertibleToNumber = std::is_arithmetic_v<T>;

    template <typename T>
    concept isJsonValue = std::is_same_v<T, Json::Value>;
}
