#pragma once

#include <cstddef>
#include <algorithm>

namespace esd::math
{

template <typename T>
inline T abs(const T &n)
{
    return n < 0 ? -n : n;
}

template <typename T>
inline T trunc(const T &n)
{
    return std::trunc(n);
}

template <typename T>
inline T floor(const T &n)
{
    return std::floor(n);
}

template <typename T>
inline T ceil(const T &n)
{
    return std::ceil(n);
}

template <typename T>
inline T round(const T &n)
{
    return std::round(n);
}

// "i" functions perform the operation directly to an integer type
// Considerably faster that normal floating point operations when
// special floating point behavior is not needed
// Warning: NaN becomes 0, Inifinity becomes 1, -Infinity becomes -1

template <typename I, typename T, typename = std::enable_if_t<std::is_integral_v<I>>>
inline I itrunc(const T &n)
{
    return (I)n;
}

template <typename I, typename T, typename = std::enable_if_t<std::is_integral_v<I>>>
inline I ifloor(const T &n)
{
    I ni = (I)n;
    return n < ni ? ni - 1 : ni;
}

template <typename I, typename T, typename = std::enable_if_t<std::is_integral_v<I>>>
inline I iceil(const T &n)
{
    I ni = (I)n;
    return n > ni ? ni + 1 : ni;
}

template <typename I, typename T, typename = std::enable_if_t<std::is_integral_v<I>>>
inline I iround(const T &n)
{
    I ni = (I)n;
    return n > 0 ? (n - ni >= 0.5 ? ni + 1 : ni) : (n - ni <= -0.5 ? ni - 1 : ni);
}

} // namespace eseed::math