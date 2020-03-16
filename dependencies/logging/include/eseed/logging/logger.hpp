#pragma once

#include <ctime>
#include <vector>
#include <ostream>
#include <iostream>
#include <memory>
#include <eseed/logging/format.hpp>

namespace esdl {

class Logger {
public:
    enum LogLevel {
        LogLevelTrace,
        LogLevelDebug,
        LogLevelInfo,
        LogLevelWarn,
        LogLevelError,
        LogLevelFatal
    };

    // Construct logger to std::cout
    Logger();

    // Construct logger to a single output
    Logger(std::ostream* output);

    // Construct logger to multiple outputs 
    Logger(std::vector<std::ostream*> outputs);

    // Check if "level" is equal to or above the minimum log level
    bool isLevelEnabled(LogLevel level) const;

    // All log levels at and above "level" will be outputted 
    void setMinLogLevel(LogLevel level);

    // For the most verbose and insignificant of details
    template <typename... Ts>
    void trace(const std::string& format, const Ts&... args) const {
        return printlnLevel(LogLevelTrace, format, args...);
    }

    // For minor details to help with debugging
    template <typename... Ts>
    std::string debug(const std::string& format, const Ts&... args) const {
        return printlnLevel(LogLevelDebug, format, args...);
    }

    // For general information
    template <typename... Ts>
    std::string info(const std::string& format, const Ts&... args) const {
        return printlnLevel(LogLevelInfo, format, args...);
    }

    // For unexpected but non-threatening circumstances
    template <typename... Ts>
    std::string warn(const std::string& format, const Ts&... args) const {
        return printlnLevel(LogLevelWarn, format, args...);
    }

    // For a recoverable problem
    template <typename... Ts>
    std::string error(const std::string& format, const Ts&... args) const {
        return printlnLevel(LogLevelError, format, args...);
    }

    // For a problem that cannot be recovered from
    template <typename... Ts>
    std::string fatal(const std::string& format, const Ts&... args) const {
        return printlnLevel(LogLevelFatal, format, args...);
    }

    // Assert a condition, crash the program and output a message if false
    template <typename... Ts>
    void fatalAssert(
        bool condition, 
        const std::string& format, 
        const Ts&... args
    ) const {
        if (!condition) {
            fatal(format, args...);
            std::terminate();
        }
    }

private:
    LogLevel minLogLevel = LogLevelInfo;
    std::vector<std::ostream*> outputs;

    static std::string getLogLevelString(LogLevel level);

    // Condense arguments to pass to println function
    template <typename... Ts>
    std::string printlnLevel(
        LogLevel level, 
        const std::string& format, 
        const Ts&... args
    ) const {
        std::string line = esdl::format(format, args...);
        if (isLevelEnabled(level)) println(getLogLevelString(level), line);
        return line;
    }
    
    // Print a line of text prefixed with date and level
    void println(const std::string& level, const std::string& line) const;
};

inline Logger mainLogger;

}