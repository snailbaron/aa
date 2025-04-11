#pragma once

#include <exception>
#include <source_location>
#include <string>

namespace aa {

class Error : public std::exception {
public:
    Error() = default;

    explicit Error(
        std::string message,
        std::source_location sl = std::source_location::current());

    const char* what() const noexcept override;

private:
    std::string _message;
};

} // namespace aa
