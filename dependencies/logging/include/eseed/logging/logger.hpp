#pragma once

#include <ctime>
#include <vector>
#include <ostream>
#include <iostream>
#include <eseed/logging/format.hpp>

namespace eseed::logging
{

class Logger
{
public:
    enum LogLevel
    {
        LEVEL_TRACE,
        LEVEL_DEBUG,
        LEVEL_INFO,
        LEVEL_WARN,
        LEVEL_ERROR,
        LEVEL_FATAL
    };

    Logger()
    {
        outputs.push_back(&std::cout);
    }

    Logger(std::ostream destination);
    Logger(std::vector<std::ostream> outputs);

    template <typename... Ts>
    void trace(const std::string &format, const Ts &... args)
    {
        if (levelEnabled(LEVEL_TRACE))
            println("TRACE", format, args...);
    }

    template <typename... Ts>
    void debug(const std::string &format, const Ts &... args)
    {
        if (levelEnabled(LEVEL_DEBUG))
            println("DEBUG", format, args...);
    }

    template <typename... Ts>
    void info(const std::string &format, const Ts &... args)
    {
        if (levelEnabled(LEVEL_INFO))
            println("INFO", format, args...);
    }

    template <typename... Ts>
    void warn(const std::string &format, const Ts &... args)
    {
        if (levelEnabled(LEVEL_WARN))
            println("WARN", format, args...);
    }

    template <typename... Ts>
    void error(const std::string &format, const Ts &... args)
    {
        if (levelEnabled(LEVEL_ERROR))
            println("ERROR", format, args...);
    }

    template <typename... Ts>
    void fatal(const std::string &format, const Ts &... args)
    {
        if (levelEnabled(LEVEL_FATAL))
            println("FATAL", format, args...);
    }

    bool levelEnabled(LogLevel level)
    {
        return level >= minLogLevel;
    }

    void setMinLogLevel(LogLevel level)
    {
        minLogLevel = level;
    }

private:
    LogLevel minLogLevel = LEVEL_INFO;
    std::vector<std::ostream *> outputs;

    template <typename... Ts>
    void println(const std::string &level, const std::string &format, const Ts &... args)
    {
        std::string line = eseed::logging::format(format, args...);
        println(level, line);
    }

    void println(const std::string &level, const std::string &line)
    {
        time_t tt;
        time(&tt);
        tm ti;
        localtime_s(&ti, &tt);

        char timeStr[18];
        strftime(timeStr, sizeof(timeStr), "%y-%m-%d %H:%M:%S", &ti);

        std::string outLine = std::string(timeStr) + " [" + level + "]: " + line;
        for (auto out : outputs)
        {
            *out << outLine << std::endl;
        }
    }
};

} // namespace eseed::logging