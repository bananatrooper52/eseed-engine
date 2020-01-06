#pragma once

#include <string>

namespace eseed::logging
{

template <typename... Ts>
std::string format(const std::string& format, const Ts&... args);

template <typename T, typename... Ts>
std::string format(size_t argIndex, const std::string& format, const T& arg, const Ts&... args);

template <typename T>
std::string format(size_t argIndex, const std::string& format, const T& arg);
    
}

#include "format.inl"