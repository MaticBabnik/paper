#pragma once

#include <concepts>
#include <format>
#include <source_location>
#include <string_view>
#include <type_traits>

namespace hidden {
constexpr std::string_view RESET_COLOR = "\033[0m";
constexpr std::string_view ERROR       = "\033[1;31m";

struct panic_dynamic_string_view {
    template <class T>
        requires std::constructible_from<std::string_view, T>
    panic_dynamic_string_view(
        const T             &s,
        std::source_location loc = std::source_location::current()
    ) noexcept
        : s{s}, loc{loc} {}

    std::string_view     s;
    std::source_location loc;
};

template <class... Args> struct panic_format {
    template <class T>
    consteval panic_format(
        const T             &s,
        std::source_location loc = std::source_location::current()
    ) noexcept
        : fmt{s}, loc{loc} {}

    std::format_string<Args...> fmt;
    std::source_location        loc;
};

[[noreturn]]
void panic_impl(const char *s) noexcept;

[[noreturn]]
inline void panic(panic_dynamic_string_view s) noexcept {
    auto msg = std::format(
        "{}[FATAL @ {}:{}]{} {}\n",
        ERROR,
        s.loc.file_name(),
        s.loc.line(),
        RESET_COLOR,
        s.s
    );
    panic_impl(msg.c_str());
}

template <class... Args>
[[noreturn]]
void panic(
    panic_format<std::type_identity_t<Args>...> fmt,
    Args &&...args
) noexcept
    requires(sizeof...(Args) > 0)
{
    auto msg = std::format(
        "{}[FATAL @ {}:{}]{} {}\n",
        ERROR,
        fmt.loc.file_name(),
        fmt.loc.line(),
        RESET_COLOR,
        std::format(fmt.fmt, std::forward<Args>(args)...)
    );
    panic_impl(msg.c_str());
}

} // namespace hidden

using hidden::panic;