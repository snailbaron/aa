#include <aa/error.hpp>

#include <format>

namespace aa {

Error::Error(std::string message, std::source_location sl)
    : _message(std::format(
        "{}:{}:{} ({}): {}",
        sl.file_name(),
        sl.line(),
        sl.column(),
        sl.function_name(),
        std::move(message)))
{ }

const char* Error::what() const noexcept
{
    return _message.c_str();
}

} // namespace aa
