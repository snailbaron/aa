#pragma once

#include <exception>
#include <sstream>
#include <string>

namespace aa {

class Error : public std::exception {
public:
    Error() = default;

    explicit Error(
        const std::string& message,
        const std::string& file,
        int line,
        const std::string& function)
    {
        auto stream = std::ostringstream{};
        stream << file << ":" << line << " (" << function << "): " << message;
        _message = std::move(stream).str();
    }

    const char* what() const noexcept override
    {
        return _message.c_str();
    }

private:
    std::string _message;
};

#define FAIL(MESSAGE)                                         \
    do {                                                      \
        throw Error{(MESSAGE), __FILE__, __LINE__, __func__}; \
    } while (false)

#define ASSERT(CONDITION)                        \
    do {                                         \
        if (!(CONDITION)) {                      \
            throw Error{                         \
                "assertion failed: " #CONDITION, \
                __FILE__, __LINE__, __func__};   \
        }                                        \
    } while (false)

} // namespace aa
