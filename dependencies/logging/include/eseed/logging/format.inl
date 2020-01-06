#pragma once

#include "format.hpp"

#include <sstream>

template <typename... Ts>
std::string eseed::logging::format(const std::string& format, const Ts&... args)
{
    return eseed::logging::format(0, format, args...);
}

template <typename T, typename... Ts>
std::string eseed::logging::format(size_t argIndex, const std::string& format, const T& arg, const Ts&... args)
{
    std::string f = eseed::logging::format(argIndex, format, arg);
    return eseed::logging::format(argIndex + 1, f, args...);
}

template <typename T>
std::string eseed::logging::format(size_t argIndex, const std::string& format, const T& arg)
{
    std::ostringstream out;
    bool readingArg = false;
    std::string argStr;

    bool autoArgRead = false;
    
    for (size_t i = 0; i < format.length(); i++)
    {
        const char& ch = format[i];

        if (ch == '{' && !readingArg)
        {
            readingArg = true;
            continue;
        }
        if (ch == '}' && readingArg)
        {
            readingArg = false;

            if (argStr == "" && !autoArgRead)
            {
                (std::ostream&)out << arg;
                autoArgRead = true;
            }
            else
            {
                out << "{" << argStr << "}";
            }

            argStr = "";
            
            continue;
        }

        if (readingArg)
        {
            argStr += ch;
        }
        else
        {
            out << ch;
        }
    }

    return out.str();
}