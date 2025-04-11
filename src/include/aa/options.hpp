#pragma once

#include "error.hpp"

#include <algorithm>
#include <format>
#include <memory>
#include <optional>
#include <ostream>
#include <sstream>
#include <sstream>
#include <string>
#include <vector>

namespace aa {

namespace {

std::string join(
    const std::vector<std::string>& strings, const std::string delimiter)
{
    auto stream = std::ostringstream{};
    if (auto it = strings.begin(); it != strings.end()) {
        stream << *it++;
        for (; it != strings.end(); ++it) {
            stream << delimiter << *it;
        }
    }
    return std::move(stream).str();
}

} // namespace

struct FlagData final {
    int count = 0;
    std::string help;
};

struct OptionDataBase {
    virtual ~OptionDataBase() = default;
    virtual void parseValue(std::string) = 0;

    std::string metavar = "VALUE";
    bool required = false;
    std::string help;
    bool hasValue = false;
};

template <class T>
struct OptionData final : OptionDataBase {
    void parseValue(std::string s) override
    {
        auto x = T{};
        auto stream = std::istringstream{std::move(s)};
        stream >> x;
        value = x;
        hasValue = true;
    }

    std::optional<T> value;
};

template <class T>
class Option final {
public:
    explicit Option(std::shared_ptr<OptionData<T>> data = nullptr)
        : _data(std::move(data))
    { }

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
        _data->value = std::forward<T>(x);
        _data->hasValue = true;
        return *this;
    }

    const T& operator*() const
    {
        if (!_data->value) {
            throw Error{"aa: trying to use a value that was not set: "};
        }
        return *_data->value;
    }

    const T* operator->() const
    {
        return &**this;
    }

    operator const T&() const
    {
        return **this;
    }

private:
    std::shared_ptr<OptionData<T>> _data;
};

template <class T>
std::ostream& operator<<(std::ostream& out, const Option<T>& option)
{
    return out << *option;
}

class Flag final {
public:
    explicit Flag(std::shared_ptr<FlagData> data = nullptr)
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
    std::shared_ptr<FlagData> _data;
};

} // namespace aa

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

