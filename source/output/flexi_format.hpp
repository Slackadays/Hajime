/*  Hajime, the ultimate startup script.
    Copyright (C) 2022 Slackadays and other contributors to Hajime on GitHub.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.*/
    
#pragma once

#define FMT_HEADER_ONLY
#include <fmt/format.h>

#include <string>

std::string flexi_format(const auto& format, const auto&... args) {
    #if defined(__APPLE__) || defined(_WIN32) || defined(_WIN64)
    return fmt::vformat(fmt::detail::to_string_view(format), fmt::make_format_args(args...));
    #else
    return fmt::vformat(fmt::to_string_view(format), fmt::make_format_args(args...));
    #endif
}