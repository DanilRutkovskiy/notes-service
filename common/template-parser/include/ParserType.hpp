#pragma once
#include<boost/mp11.hpp>

namespace TemplateParser::ParserType
{
    /**
     * @brief CustomType - шаблонная структура для создания пользовательских типов
     * с указанными ограничениями
     */
    template<typename T, typename ...Req>
    struct CustomType;

    /**
     * @brief Специализация шаблонная, нужно только для того, чтобы  можно было
     * узнать, является ли произвольный тип T типом CustomType<S>
     */
    template<>
    struct CustomType<void> {};

    /**
     * @brief CustomType - шаблонная структура для создания пользовательских типов
     * с указанными ограничениями
     * @Type - внутринний тип. Тип, над которым сделали CustomType
     * @requirements - набор ограничений
     */
    template<typename T, typename ...Req>
    struct CustomType : CustomType<void>
    {
    public:
        using Type = T;
        using requirements = boost::mp11::mp_list<Req...>;

    private:
        T value;

    public:
        operator T&() & {return value;}
        operator T() && {return std::move(value);}

        auto *operator->();
        const auto *operator->() const;

        auto &operator*();
        const auto &operator*() const;

        auto operator ==(const CustomType& other) const { return this->value == other.value;};
        auto operator !=(const CustomType& other) const { return !(*this == other);};

        const auto& operator=(const T& newValue);
        const auto& operator=(T&& newValue);

        CustomType() = default;
        CustomType(const T& newValue){value = newValue;};
        CustomType(T&& newValue){value = std::move(newValue);};
    };

    /**
     * @brief Специализация для 'наследования' ограничений от другого CustomType
     *
     * @Type - внутринний тип. Тип, над которым сделали CustomType
     * @requirements - набор ограничений
     * @example:
     *
     *      using T1 = CustomType<int, minValue<1>>;
     *      using T2 = CustomType<T1, maxValue<10>>;
     *      // T1::requirements == {minValue<1>};
     *      // T2::requirements == {minValue<1>, maxValue<2>};
     *
     */
    template<typename T, typename ...ReqOld, typename ...ReqNew>
    struct CustomType<CustomType<T, ReqOld...>, ReqNew...> : CustomType<T, ReqOld..., ReqNew...> {};

    /**
     * @brief Мета функция, которая проверяет является ли тип `T` базовым к типу CustomType<void>
     * @value `value` равно `true`, если возможно произвести static_cast к типу CustomType<void>.
     * Во всех остальных случаях `false`
     */
    template<typename T>
    concept isCustomType = std::is_base_of_v<CustomType<void>, T>;

    template<typename T>
    struct customTypeCastImpl
    {
        using type = T;
    };

    template<typename T, typename ...Args>
    struct customTypeCastImpl<CustomType<T, Args...>>
    {
        using type = customTypeCastImpl<T>::type;
    };

    template<template <typename ...> typename Container, typename ...Args>
    struct customTypeCastImpl<Container<Args...>>
    {
        using type = Container<typename customTypeCastImpl<Args>::type...>;
    };

    /**
     * @brief Функция возвращает измененый тип T. Где в каждом аргементе шаблона этого типа
     * произведена замена CustomType<S> -> S.
     * @example
     *
     *      std::vector<CustomType<int>> v1; // decltype(v1) = std::vector<CustomType<int>, std::allocator<CustomType<int>>\n
     *
     *      auto &v2 = customTypeCast(v1); // decltype(v2) = std::vector<int, std::allocator<int>>\n
     */
    template<typename T>
    constexpr inline auto &customTypeCast(T &t)
    {
        using newType = typename customTypeCastImpl<std::remove_reference_t<T>>::type;
        return reinterpret_cast<newType &>(t);
    }

    template<typename T>
    constexpr inline auto &customTypeCast(const T &t)
    {
        using newType = typename customTypeCastImpl<std::remove_reference_t<T>>::type;
        return reinterpret_cast<const newType &>(t);
    }

    template<typename T, typename ...Req>
    auto *CustomType<T, Req...>::operator->()
    {
        return &customTypeCast(value);
    }

    template<typename T, typename ...Req>
    const auto *CustomType<T, Req...>::operator->() const
    {
        return &customTypeCast(value);
    }

    template<typename T, typename ...Req>
    const auto& CustomType<T, Req...>::operator=(const T& newValue)
    {
        value = newValue;
        return *this;
    };

    template<typename T, typename ...Req>
    const auto& CustomType<T, Req...>::operator=(T&& newValue)
    {
        value = std::move(newValue);
        return *this;
    };

    template<typename T, typename ...Req>
    auto &CustomType<T, Req...>::operator*()
    {
        return value;
    }

    template<typename T, typename ...Req>
    const auto &CustomType<T, Req...>::operator*() const
    {
        return value;
    }

    template<typename T>
    struct GetFromParsing;

    template<>
    struct GetFromParsing<void> {};

    /**
     * @brief Тип для пометки, что описываемый член структуры был получен из JSON во время парсинга
     */
    template<typename T>
    struct GetFromParsing : GetFromParsing<void>
    {
    public:
        using Type = T;

    public:
        bool fromParsing = false;

    private:
        T value;

    public:
        GetFromParsing() = default;
        GetFromParsing(const T& newValue) : value(newValue) {}
        GetFromParsing(T&& newValue) : value(std::move(newValue)) {}

        GetFromParsing(const GetFromParsing&) = default;
        GetFromParsing(GetFromParsing&&) = default;
        GetFromParsing& operator=(const GetFromParsing&) = default;
        GetFromParsing& operator=(GetFromParsing&&) = default;

        operator T&() & { return value; }
        operator T() && { return std::move(value); }
        operator const T&() const & { return value; }

        T* operator->() { return &value; }
        const T* operator->() const { return &value; }

        T& operator*() & { return value; }
        const T& operator*() const & { return value; }
        T&& operator*() && { return std::move(value); }

        GetFromParsing& operator=(const T& newValue)
        {
            value = newValue;
            return *this;
        }

        GetFromParsing& operator=(T&& newValue)
        {
            value = std::move(newValue);
            return *this;
        }

        bool operator==(const GetFromParsing& other) const
        {
            return value == other.value;
        }

        bool operator!=(const GetFromParsing& other) const
        {
            return !(*this == other);
        }
    };

    /**
     * @brief Мета функция, которая проверяет является ли тип `T` базовым к типу GetFromParsing<void>
     * @value `value` равно `true`, если возможно произвести static_cast к типу GetFromParsing<void>.
     * Во всех остальных случаях `false`
     */
    template<typename T>
    concept isGetFromParsing = std::is_base_of_v<GetFromParsing<void>, T>;
}
