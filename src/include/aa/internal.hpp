#pragma once

#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

namespace aa {
namespace internal {

inline bool startsWith(const std::string& string, const std::string& prefix)
{
    if (string.length() < prefix.length()) {
        return false;
    }

    for (size_t i = 0; i < prefix.size(); i++) {
        if (string.at(i) != prefix.at(i)) {
            return false;
        }
    }

    return true;
}

inline std::string join(
    const std::vector<std::string>& strings, const std::string delimiter)
{
    auto stream = std::ostringstream{};

    auto it = strings.begin();
    if (it != strings.end()) {
        stream << *it++;
        for (; it != strings.end(); ++it) {
            stream << delimiter << *it;
        }
    }

    return std::move(stream).str();
}

template <
    class T,
    class = std::enable_if<std::is_default_constructible<T>::value>>
T fromString(const std::string& string)
{
    auto stream = std::istringstream{string};
    auto value = T{};
    stream >> value;
    return value;
}

template<class...>
struct conjunction : std::true_type {};

template<class B1>
struct conjunction<B1> : B1 {};

template<class B1, class... Bn>
struct conjunction<B1, Bn...>
    : std::conditional<bool(B1::value), conjunction<Bn...>, B1>::type {};

}} // namespace aa::internal
