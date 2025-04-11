#include <aa/error.hpp>

#include <format>
#include <stacktrace>

namespace aa {

Error::Error(std::string message, std::source_location sl)
    : _message(std::format(
        "{}:{}:{} ({}): {}\n{}",
        sl.file_name(),
        sl.line(),
        sl.column(),
        sl.function_name(),
        std::move(message),
#if defined(__cpp_lib_stacktrace) && defined(__cpp_lib_formatters)
        std::stacktrace::current()
#else
        ""
#endif
    ))
{ }

const char* Error::what() const noexcept
{
    return _message.c_str();
}

} // namespace aa
