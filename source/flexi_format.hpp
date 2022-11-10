#pragma once

#define FMT_HEADER_ONLY
#include <fmt/format.h>

#include <string>

std::string flexi_format(const auto& format, const auto&... args) {
    #if defined(__APPLE__)
    return fmt::vformat(fmt::detail::to_string_view(format), fmt::make_format_args(args...));
    #else
    return fmt::vformat(fmt::to_string_view(format), fmt::make_format_args(args...));
    #endif
}