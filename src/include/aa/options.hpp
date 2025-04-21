#pragma once

#include "error.hpp"
#include "internal.hpp"

#include <vector>
#include <algorithm>
#include <memory>
#include <ostream>
#include <string>

namespace aa {

struct OptionData {
    virtual ~OptionData() = default;
    virtual void parseValue(std::string) = 0;

    std::vector<std::string> flags;
    bool expectsValue = false;
    bool required = false;
    int count = 0;
    std::string metavar = "VALUE";
    std::string help;
};

template <class T>
struct TypedOptionData final : OptionData {
    void parseValue(std::string s) override
    {
        values.push_back(internal::fromString<T>(s));
    }

    std::vector<T> values;
};

template <>
struct TypedOptionData<void> final : OptionData {
    void parseValue(std::string) override
    {
        FAIL("TypedOptionData<void>::parseValue should not be called");
    }
};

class Flag final {
public:
    explicit Flag(std::shared_ptr<TypedOptionData<void>> data = nullptr)
        : _data(std::move(data))
    { }

    Flag help(std::string message)
    {
        _data->help = std::move(message);
        return *this;
    }

    int operator*() const
    {
        return _data->count;
    }

    operator int() const
    {
        return **this;
    }

private:
    std::shared_ptr<TypedOptionData<void>> _data;
};

template <class T>
class Option final {
public:
    explicit Option(std::shared_ptr<TypedOptionData<T>> data)
        : _data(std::move(data))
    {
        ASSERT(_data);
    }

    Option metavar(std::string name)
    {
        _data->metavar = std::move(name);
        return *this;
    }

    Option required()
    {
        _data->required = true;
        return *this;
    }

    Option help(std::string message)
    {
        _data->help = std::move(message);
        return *this;
    }

    Option init(T&& x)
    {
        _data->values.push_back(std::forward<T>(x));
        return *this;
    }

    const std::vector<T>& all() const
    {
        return _data->values;
    }

    const T& first() const
    {
        if (all().empty()) {
            FAIL("attempting to access empty option " +
                internal::join(_data->flags, ","));
        }
        return all().front();
    }

    const T& last() const
    {
        if (all().empty()) {
            FAIL("attempting to access empty option " +
                internal::join(_data->flags, ","));
        }
        return all().back();
    }

    const T& operator*() const
    {
        return last();
    }

    operator const T&() const
    {
        return **this;
    }

    const T* operator->() const
    {
        return &**this;
    }

private:
    std::shared_ptr<TypedOptionData<T>> _data;
};

template <class T>
std::ostream& operator<<(std::ostream& out, const Option<T>& option)
{
    return out << *option;
}

} // namespace aa

#if defined(__cpp_lib_format)

#include <format>

namespace std {

template <class T, class CharT>
struct formatter<aa::Option<T>, CharT> {
    template <class ParseContext>
    constexpr ParseContext::iterator parse(ParseContext& ctx)
    {
        return formatter<T, CharT>{}.parse(ctx);
    }

    template <class FmtContext>
    FmtContext::iterator format(
        const aa::Option<T>& option, FmtContext& ctx) const
    {
        return formatter<T, CharT>{}.format(*option, ctx);
    }
};

} // namespace std

#endif
