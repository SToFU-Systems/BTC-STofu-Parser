#pragma once

// Standard Library headers
#include <string>
#include <type_traits>
#include <string_view>
#include <format>
#include <utility>

// Project headers
#include "Common.h"
#include "Utils.h"


//================================================================================
// Function: convert_arg
// Description: Converts wchar types to UTF-8 string for formatting
//================================================================================
template<typename T>
auto convert_arg(INOUT T&& value)
{
    // Detect wchar-based types at compile time and convert them to UTF-8
    if constexpr (std::is_same_v<std::decay_t<T>, std::wstring>)
        return wideStringToUtf8(value);
    else if constexpr (std::is_same_v<std::decay_t<T>, std::wstring_view>)
        return wideStringToUtf8(value);
    else if constexpr (std::is_same_v<std::decay_t<T>, const wchar_t*>)
        return wideStringToUtf8(std::wstring_view(value));
    else
        return std::forward<T>(value);
}


//================================================================================
// Function: format_utf8
// Description: Formats mixed UTF-8 + wchar arguments into UTF-8 string
//================================================================================
template<typename... Args>
inline std::string format_utf8(std::format_string<decltype(convert_arg(std::declval<Args>()))...> fmt, Args&&... args)
{
    return std::format(fmt, convert_arg(std::forward<Args>(args))...);
}
