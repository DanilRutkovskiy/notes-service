#pragma once

namespace TemplateParser::Requirements
{
    /**
     * @brief Ограничение на минимальное значение объекта типа U
     */
    template<const auto MIN_VALUE>
    struct MinValue
    {
        template<typename U>
        static bool check(const U &value)
        {
            return MIN_VALUE <= value;
        }
    };

    /**
     * @brief Ограничение на максимальное значение объекта типа U
     */
    template<const auto MAX_VALUE>
    struct MaxValue
    {
        template<typename U>
        static bool check(const U &value)
        {
            return MAX_VALUE >= value;
        }
    };

    /**
     * @brief Ограничение на допустимые значение объекта типа U
     */
    template<const auto ...ALLOWED_VALUES>
    struct AllowedValues
    {
        template<typename U>
        static bool check(const U &value)
        {
            return ((value == ALLOWED_VALUES) || ... || false);
        }
    };

    /**
     * @brief Ограничение на запрещенные значение объекта типа U
     */
    template<const auto ... NOT_ALLOWED_VALUES>
    struct NotAllowedValues
    {
        template<typename U>
        static bool check(const U &value)
        {
            return ((value != NOT_ALLOWED_VALUES) && ... && true); 
        }
    };

    /**
     * @brief Ограничение на допустимые значение значение объекта типа U соответствующее
     * регулярному выражению
     */
    template<typename Reg>
    struct CheckRegular
    {
        template<typename U>
        static bool check(const U &value)
        {
            Reg reg;
            return reg(value);
        }
    };

    /**
     * @brief Ограничение на то, чтобы объект типа U был не пустой
     */
    struct NoEmpty
    {
        template<typename U>
        static bool check(const U &value)
        {
            return !value.empty();
        }
    };

    /**
     * @brief Ограничение на минимальный и максимальный размер контейнера типа U
     */
    template<const auto MIN_VALUE, const auto MAX_VALUE>
    struct CheckSize 
    {
        template<typename U>
        static bool check(const U &value)
        {
            return (MIN_VALUE <= value.size()) && (value.size() <= MAX_VALUE);
        }
    };
}
