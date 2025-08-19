#pragma once

#include "ParserType.hpp"
#include "ParseError.hpp"
#include <json/json.h>
#include <boost/mp11.hpp>
#include <boost/describe.hpp>
#include <format>

/**
 *  Этот файл содержит набор функций и классов для шаблонной сериализации и десериализации.
 *  В описании ниже будут встречаться такие термины как:
 *  мета-функция - это шаблонный класс или структура
 */
namespace TemplateParser
{
    /**
     * @brief Тип для пометки того, что T должно быть получено из json
     */
    template<typename T>
    struct JsonWrapper
    {
    private:
        T value;

    public:
        JsonWrapper() = default;
        template<typename U>
        JsonWrapper(U &&newValue) : value{newValue} {}
        operator T&() & {return value;}
        operator T() && {return std::move(value);}
        auto *operator->() {return &value;}
        const auto *operator->() const {return &value;}
        auto &operator*() {return value;}
        const auto &operator*() const {return value;}
    };

    template<typename T>
    struct isJsonWrapper : std::false_type {};

    template<typename T>
    struct isJsonWrapper<JsonWrapper<T>> : std::true_type {};

    template<typename T>
    struct ParserType::customTypeCastImpl<JsonWrapper<T>>
    {
        using type = customTypeCastImpl<T>::type;
    };

    template<typename T, typename Y>
    struct ParseImpl;

    template<typename T, typename Y>
    inline ParseError parse(T &&src, Y &&dst)
    {
        return ParseImpl<std::remove_const_t<std::remove_reference_t<T>>,
                         std::remove_const_t<std::remove_reference_t<Y>>>::doParse(std::forward<T>(src), dst);
    }
    /**
     * @brief Парсит исходный объект и конвертирует его в long long.
     *
     * @tparam T Тип исходного объекта.
     * @tparam En Условие включения, определяемое с использованием SFINAE на основе наличия определенной функции-члена в типе исходного объекта.
     * @param src Исходный объект для парсинга.
     * @param dst Переменная назначения для хранения результата парсинга.
     * @return true Если парсинг прошел успешно.
     * @return false Если парсинг не удался.
     */
    template<typename T> requires Concepts::hasToInt<T>
    struct ParseImpl<T, long long>
    {
        static ParseError doParse(const T &src, long long &dst)
        {
            try
            {
                dst = src.asInt64();
            }
            catch(const Json::Exception& e)
            {
                return ParseError("error parse value to int");
            }
            
            return {};
        }
    };

    /**
     * @brief Парсит исходный объект и присваивает его значение переменной назначения того же типа.
     *
     * @tparam T Тип исходного объекта.
     * @tparam Y Тип переменной назначения.
     * @param src Исходный объект для парсинга.
     * @param dst Переменная назначения для хранения результата парсинга.
     * @return true Всегда возвращает true, так как присваивание всегда успешно.
     */
    template<typename T, typename Y> requires std::same_as<T, Y>
    struct ParseImpl<T, Y>
    {
        template<typename U>
        static ParseError doParse(U &&src, Y &dst)
        {
            dst = std::forward<U>(src);
            return {};
        }
    };

    /**
     * @brief Парсит исходный объект и конвертирует его в ushort.
     *
     * @tparam T Тип исходного объекта.
     * @tparam En Условие включения, определяемое с использованием SFINAE на основе наличия определенной функции-члена в типе исходного объекта.
     * @param src Исходный объект для парсинга.
     * @param dst Переменная назначения для хранения результата парсинга.
     * @return true Если парсинг прошел успешно.
     * @return false Если парсинг не удался.
     */
    template<typename T> requires Concepts::hasToUShort<T>
    struct ParseImpl<T, ushort>
    {
        static ParseError doParse(const T &src, ushort &dst)
        {
            try
            {
                dst = src.asUInt();
            }
            catch(const Json::Exception& e)
            {
                return ParseError("error parse value to ushort");
            }

            return {};
        }
    };

    template<typename T> requires Concepts::hasToFloat<T>
    struct ParseImpl<T, float>
    {
        static ParseError doParse(const T &src, float &dst)
        {
            bool isOk;
            dst = src.toFloat(&isOk);
            return isOk ? ParseError() : ParseError("error parse value to float");
        }
    };

    /**
     * @brief Парсит исходный объект и конвертирует его в QString.
     *
     * @tparam T Тип исходного объекта.
     * @tparam En Условие включения, определяемое с использованием SFINAE на основе наличия определенной функции-члена в типе исходного объекта.
     * @param src Исходный объект для парсинга.
     * @param dst Переменная назначения для хранения результата парсинга.
     * @return true Всегда возвращает true, так как конвертация всегда успешна.
     */
    template<typename T> requires Concepts::hasToString<T> && (!Concepts::isJsonValue<T>)
    struct ParseImpl<T, std::string>
    {
        static ParseError doParse(const T &src, std::string &dst)
        {
            dst = src.toString();
            return ParseError{};
        }
    };

    template<typename T> requires Concepts::isJsonValue<T>
    struct ParseImpl<T, std::string>
    {
        static ParseError doParse(const T &src, std::string &dst)
        {
            if (src.type() == Json::ValueType::stringValue)
            {
                dst = src.asString();
            }
            else if (src.isNumeric())
            {
                dst = std::to_string(src.asDouble());
            }
            else
            {
                return ParseError{"Expected string or number"};
            }
            return ParseError{};
        }
    };

    /**
     * @brief Парсит исходный объект и конвертирует его в bool.
     *
     * @tparam T Тип исходного объекта.
     * @tparam En Условие включения, определяемое с использованием SFINAE на основе наличия определенной функции-члена в типе исходного объекта.
     * @param src Исходный объект для парсинга.
     * @param dst Переменная назначения для хранения результата парсинга.
     * @return true Всегда возвращает true, так как конвертация всегда успешна.
     */
    template<typename T> requires Concepts::hasToBool<T>
    struct ParseImpl<T, bool>
    {
        static ParseError doParse(const T &src, bool &dst)
        {
            dst = src.toBool();
            return {};
        }
    };

    /**
     * @brief Парсит исходный объект типа QString и конвертирует его в значение перечисления.
     *
     * @tparam T Тип значения перечисления.
     * @param src Исходный объект для парсинга.
     * @param dst Переменная назначения для хранения результата парсинга.
     * @return true Если парсинг прошел успешно.
     * @return false Если парсинг не удался.
     */
    template<typename T> requires std::is_enum_v<T>
    struct ParseImpl<std::string, T>
    {
        template<typename U>
        static ParseError doParse(U &&src, T &dst)
        {
            if (!stringTypeToEnum(src, dst))
            {
                return "parsing from string to enum";
            } 
            return {};
        }
    };

    /**
     * @brief Парсит исходный объект типа T и конвертирует его в значение перечисления.
     *
     * @tparam T Тип исходного значения
     * @tparam Y Тип значения перечисления.
     * @param src Исходный объект для парсинга.
     * @param dst Переменная назначения для хранения результата парсинга.
     * @return true Если парсинг прошел успешно.
     * @return false Если парсинг не удался.
     */
    template<typename T, typename Y> requires Concepts::hasToString<T> && std::is_enum_v<Y>
    struct ParseImpl<T, Y>
    {
        template<typename U>
        static ParseError doParse(U &&src, Y &dst)
        {
            std::string tmpString;
            if (parse(std::forward<U>(src), tmpString))
            {
                return "parsing to tempory string";
            } 
            if (!stringTypeToEnum(tmpString, dst)) 
            {
                return "parsing from tempory string to string";
            }
            return {};
        }
    };

    /**
     * @brief Парсит исходный объект и конвертирует его в std::optional.
     *
     * @tparam T Тип исходного объекта.
     * @tparam Y Тип значения std::optional.
     * @param src Исходный объект для парсинга.
     * @param dst Переменная назначения для хранения результата парсинга.
     * @return true Если парсинг прошел успешно.
     * @return false Если парсинг не удался.
     */
    template<typename T, typename Y>
    struct ParseImpl<T, std::optional<Y>>
    {
        template<typename U>
        static ParseError doParse(U &&src, std::optional<Y> &dst)
        {
            if constexpr (requires{ requires Concepts::hasIsNull<U>; })
            {
                if (src.isNull())
                    return {};
            }

            using TrueType = typename std::remove_reference_t<Y>;

            TrueType value;
            const auto error = parse(std::forward<U>(src), value);
            if (!error)
                dst = std::move(value);

            return error;
        }
    };

    /**
     * @brief Парсит исходный объект и конвертирует его в std::variant.
     * Парсит в каждый тип ...Args std::variant'а, пока парсинг не удастся
     *
     * @tparam T Тип исходного объекта.
     * @tparam Y Тип значения std::variant.
     * @param src Исходный объект для парсинга.
     * @param dst Переменная назначения для хранения результата парсинга.
     * @return true Если парсинг прошел успешно.
     * @return false Если парсинг не удался.
     */
    template<typename T, typename ...Args>
    struct ParseImpl<T, std::variant<Args...>>
    {
        template<typename U>
        static ParseError doParse(U &&src, std::variant<Args...> &dst)
        {
            // функция для парсинга в один какой-то тип
            const auto oneParse = [&](const T &src, auto * oneVariant)
            {
                //Экземпляр типа oneVariant
                std::remove_reference_t<decltype(*oneVariant)> value;
                const auto error = parse(src, value);
                if (!error)
                    dst = std::move(value);
                return error;
            };

            /*
             * Для примера. Если Args = {int, float, std::string},
             * То следующая строка развернется в следующее
             * oneParse(src, (int *)0) || oneParse(src, (false *)0) || oneParse(src, (std::string*)0) || false
             * Тут используется ленивое вычисление логических выражений. Если первый oneParse() вернет true,
             * то парсинг закончится, иначе вызавется следующий oneParse. И так до выражение || false.
             */
            std::array errors{oneParse(src, (Args *)0)...};
            if (std::any_of(begin(errors), end(errors), [](auto &value){return !bool(value);}))
                return {};

            ParseError error{"error parsing where in any of type :"};
            error.addSubErrors(std::move(errors));
            return error;
        }
    };

    /**
     * @brief Парсит исходный объект и вернет true, если QVariant.isNull() = true
     *
     * @param src Исходный объект для парсинга.
     * @param dst Переменная назначения. Не используется. Нужна только для перегрузки
     * @return true Если парсинг прошел успешно.
     * @return false Если парсинг не удался.
     */
    template<>
    struct ParseImpl<Json::Value, std::monostate>
    {
        static ParseError doParse(const Json::Value &src, [[maybe_unused]]std::monostate dst)
        {
            return src.isNull() ? ParseError{} : ParseError{"is not null"};
        }
    };

    /**
     * @brief Парсит исходный объект и конвертирует ее в объект типа T. После удачного парсинга
     * происходит проверка всех `requirements` для этого типа.
     *
     * @tparam T Тип объекта назначения.
     * @tparam Y Тип строки источника.
     * @enable Условие включения, определяемое на основе того, является ли тип `T` типом AusType<>
     * @param src Исходный объект для парсинга.
     * @param dst Переменная назначения для хранения результата парсинга.
     * @return true Если парсинг прошел успешно и все `requirements` вернули `true`.
     * @return false Если парсинг не удался или один из `requirements` вернул `false`.
     */
    template<typename T, typename Y> requires ParserType::isCustomType<Y>
    struct ParseImpl<T, Y>
    {
        template<typename U, typename SubType, typename ...Reqs>
        static ParseError doParse(U &&src, ParserType::CustomType<SubType, Reqs...> &dst)
        {
            auto error = parse(std::forward<U>(src), *dst);
            if (error)
                return error;

            bool isOk = true;
            boost::mp11::mp_for_each<typename Y::requirements>([&](auto I)
            {
                isOk = isOk and decltype(I)::check(*dst);
            });

            return isOk ? ParseError{} : ParseError{"error when check requirements"};
        }
    };

    template <typename T, typename Y> requires ParserType::isGetFromParsing<Y>
    struct ParseImpl<T, Y>
    {
        template <typename U, typename SubType> static ParseError doParse(U&& src, ParserType::GetFromParsing<SubType>& dst)
        {
            auto error = parse(std::forward<U>(src), *dst);
            if (error)
            {
                return error;
            }

            dst.fromParsing = true;

            return ParseError{};
        }
    };

    /**
     * @brief Парсит строку и конвертирует ее в контейнер.
     *
     * @tparam Container Тип контейнера.
     * @tparam En Условие включения, определяемое на основе проверки isContainer.
     * @param str Строка источника.
     * @param container Контейнер назначения.
     * @return true Если парсинг прошел успешно.
     * @return false Если парсинг не удался.
     */
    template<typename Container> requires Concepts::isContainer<Container> && (!Concepts::isString<Container>)
    struct ParseImpl<std::string, Container>
    {
        template<typename T>
        static ParseError doParse(T &&str, Container &container)
        {
            auto listElements = str.split(',');
            for (auto &&elem : listElements)
            {
                typename Container::value_type value;
                auto error = parse(std::move(elem), value);
                if (error)
                    return ParseError{"parsing error array"}.addSubError(std::move(error));

                container.emplace_back(std::move(value));
            }
            return {};
        }
    };

    /**
     * @brief Парсит значение QJsonValue и конвертирует его в контейнер.
     *
     * @tparam T Тип контейнера.
     * @param value Значение QJsonValue для парсинга.
     * @param container Контейнер назначения.
     * @return true Если парсинг прошел успешно.
     * @return false Если парсинг не удался.
     */
    template<typename T> requires Concepts::isContainer<T> && (!Concepts::isString<T>)
    struct ParseImpl<Json::Value, T>
    {
        static ParseError doParse(const Json::Value &value, T &container)
        {
            if (!value.isArray())
                return ParseError{"json value is not array"};

            container.reserve(value.size());
            for (Json::ArrayIndex i = 0; i < value.size(); ++i)
            {
                auto &elem = container.emplace_back();
                if (auto error = parse(value[i], elem);
                    error)
                {
                    return ParseError{"parsing error array:"}.addSubError(std::move(error));
                }
            }

            return ParseError{};
        }
    };

    /**
     * @brief Используется для проверки наличия необходимых полей в JSON src для заполнения структуры dst/
     * @param src - JSON, поля которого проверяем на соответствие полям структуры
     * @param dst - Структура, в которую десериализуем JSON
     * @return Возращаем все ошибки валидации,, если они были
     */
    template<typename S, typename T> requires (boost::describe::has_describe_members<T>::value)
    inline ParseError validate(const S& src, [[maybe_unused]]T& dst)
    {
        using namespace boost::mp11;
        using namespace boost::describe;

        //Генерация описания членов структуры dst(Можно получить имя и ссылку, а по ссылке тип)
        using D1 = describe_members<T, mod_public | mod_protected | mod_inherited>;
        const auto srcKeys = src.getMemberNames();

        std::array<ParseError, mp_size<D1>::value> errors;
        auto it = std::begin(errors);

        //Проход по всем члена структуры dst и поиск их эквивалента в src
        boost::mp11::mp_for_each<D1>([&](auto D)
        {
            ParseError &currentError = *it;
            auto &valueIntoStruct = dst.*D.pointer;
            using TrueType = std::remove_reference_t<decltype(valueIntoStruct)>;
            if (std::find(srcKeys.begin(), srcKeys.end(), D.name) == srcKeys.end())
            {
                //Отсутствие не опциональных членов не допустимо
                if (!(Concepts::isOptional<TrueType>::value || TemplateParser::ParserType::isGetFromParsing<TrueType>))
                {
                    currentError = std::format("parameter '{}': not found", D.name);
                }
                ++it;
            }
        });

        if (std::all_of(begin(errors), end(errors), [](auto& value) { return !static_cast<bool>(value); }))
        {
            return {};
        }

        return ParseError("not all required keys are found:").addSubErrors(std::move(errors));
    }

    /**
     * @brief Парсит объект типа `S` и преобразует ее в объект типа T, используя описание его членов.
     *
     * @tparam S Тип источника.
     * @tparam T Тип объекта назначения.
     * @param src Источник для парсинга.
     * @param dst Объект назначения для хранения результата.
     * @return true Если парсинг прошел успешно.
     * @return false Если парсинг не удался.
     * @throw функция сгенерирует исключение Adept::Exceptions::ValidationFailed, если не удалось провести
     * парсинг одного из полей. Или если поле не типа std::optional<> и не найдено по имени в объекте типа `S`,
     */
    template<typename S, typename T> requires (boost::describe::has_describe_members<T>::value)
    struct ParseImpl<S, T>
    {
        using D1 = boost::describe::describe_members<T, boost::describe::mod_public | boost::describe::mod_protected | boost::describe::mod_inherited>;
        using Parents = boost::describe::describe_bases<T, boost::describe::mod_public>;

        template<typename U>
        static ParseError doParse(U &&src, T &dst)
        {
            using namespace TemplateParser;
            using namespace boost::mp11;

            if (auto errorValidate = validate(src, dst);
                errorValidate)
            {
                return ParseError("error validate: ").addSubError(std::move(errorValidate));
            }

            std::array<ParseError, mp_size<D1>::value> errors;
            auto it = std::begin(errors);
            mp_for_each<D1>([&](auto D)
            {
                ParseError &currentError = *it;
                auto &valueIntoStruct = dst.*D.pointer;

                if (std::find(src.begin(), src.end(), D.name) == src.end())
                {
                    ++it;
                    return;
                }

                auto value = src[D.name];
                if (auto error = parse(std::move(value), valueIntoStruct);
                    error)
                {
                    currentError = std::format("parameter {}: error parsing", D.name);
                    currentError.addSubError(std::move(error));
                }

                ++it;
            });

            if (std::all_of(begin(errors), end(errors), [](auto& value) { return !static_cast<bool>(value); }))
            {
                return {};
            }

            return ParseError{"parsing error in object:"}.addSubErrors(std::move(errors));
        }
    };

    /**
     * @brief Парсит строку и конвертирует ее в JSON объект, после чего
     * этот JSON объект конвертирует в тип T.
     *
     * @tparam T Результирующий тип.
     * @param str Строка источника.
     * @param json Объект назначения.
     * @return true Если парсинг прошел успешно.
     * @return false Если парсинг не удался.
     */
    template<typename T> requires (isJsonWrapper<T>::value)
    struct ParseImpl<std::string, T>
    {
        static ParseError doParse(const std::string &str, T &json)
        {
            Json::Value root;
            Json::CharReaderBuilder builder;
            std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
            std::string errors;
            if (!reader->parse(str.c_str(), str.c_str() + str.size(), &root, &errors)) 
            {
                return ParseError{"parse error to json: " + errors};
            }

            return parse(root, *json);
        }
    };

    /**
     * @brief Функция создает std::optional<T> из std::variant<T, ...>.
     * @enable std::variant<> должен иметь один из типов std::monostate или
     * набор возможных значений меньше или равен 2
     * @example
     *
     *      std::variant<int, std::string> v1{2};
     *
     *      auto v2 = createOptional(v1); // decltype(v2) = std::optional<int>, v.has_value() == true
     *
     *      v1 = std::string();
     *
     *      auto v2 = createOptional(v1); // decltype(v2) = std::optional<int>, v.has_value() == false
     *
     *      std::variant<int, std::monostate> v3;
     *
     *      auto v2 = createOptional(v3); // decltype(v2) = std::optional<int>, v.has_value() == false
     */
    template<template <typename ...> typename Variant, typename ...Args>
    requires (std::is_same_v<Args, std::monostate> || ... || false) && (sizeof...(Args) <= 2)
    inline auto createOptional(Variant<Args...> &variant)
    {
        auto value = std::get_if<0>(&variant);
        std::optional<std::remove_reference_t<decltype(*value)>> result;
        if (value != nullptr)
            result = *value;

        return result;
    }

    template<typename T>
    struct ToStringImpl;

    template<typename T>
    concept canCallToString = requires(T a)
    {
        toString(a);
    };

    template<typename T> requires (!std::is_enum_v<T>)
    inline auto toString(const T &src, const std::string &shielding = "", const bool doToUpper = false)
    {
        using typeSrc = decltype(src);
        return ToStringImpl<std::remove_cvref_t<typeSrc>>::doToString(src, shielding, doToUpper);
    }

    /**
     * @brief Преобразует std::optional в строку QString.
     *
     * @tparam T Тип значения std::optional.
     * @param val std::optional для преобразования.
     * @param shielding Строка для экранирования.
     * @param doToUpper Приводит все символы строки к верхнему регистру.
     * @return QString Преобразованная строка.
     */
    template<typename T>
    struct ToStringImpl<std::optional<T>>
    {
        static auto doToString(const std::optional<T> &val, const std::string &shielding = "", const bool doToUpper = false)
        {
            if (val.has_value())
            {
                const auto dd = toString(val.value(), shielding, doToUpper);
            }
            return val.has_value() ? toString(val.value(), shielding, doToUpper) : "NULL";
        }

    };

    /**
     * @brief Преобразует std::variant в строку QString.
     *
     * @tparam T Тип значения std::optional.
     * @param variant std::variant для преобразования.
     * @param shielding Строка для экранирования.
     * @param doToUpper Приводит все символы строки к верхнему регистру.
     * @return QString Преобразованная строка.
     */
    template<typename ...Args>
    struct ToStringImpl<std::variant<Args...>>
    {
        static auto doToString(const std::variant<Args...> &variant, const std::string &shielding = "", const bool doToUpper = false)
        {
            return std::visit([&shielding, &doToUpper](auto &&value)
            {
                auto result = toString(value, shielding);
                if (doToUpper)
                {
                    return result.toUpper();
                }
                else
                {
                    return result;
                }
            }, variant);
        }
    };

    template<typename T> requires ParserType::isCustomType<T>
    struct ToStringImpl<T>
    {
        static auto doToString(const T &src, const std::string &shielding = "", const bool doToUpper = false)
        {
            return toString(*src, shielding, doToUpper);
        }
    };
}
