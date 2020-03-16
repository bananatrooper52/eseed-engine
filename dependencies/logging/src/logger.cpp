#include <eseed/logging/logger.hpp>

using namespace esdl;

Logger::Logger() {
    outputs.push_back(&std::cout);
}

Logger::Logger(std::ostream* destination) {
    outputs.push_back(destination);
}

Logger::Logger(std::vector<std::ostream*> outputs) 
: outputs(outputs) {}

bool Logger::isLevelEnabled(LogLevel level) const {
    return level >= minLogLevel;
}

void Logger::setMinLogLevel(LogLevel level) {
    minLogLevel = level;
}

std::string Logger::getLogLevelString(LogLevel level) {
    switch (level) {
    case LogLevelTrace: return "TRACE";
    case LogLevelDebug: return "DEBUG";
    case LogLevelInfo: return "INFO";
    case LogLevelWarn: return "WARN";
    case LogLevelError: return "ERROR";
    case LogLevelFatal: return "FATAL";
    default: return "?";
    }
}

void Logger::println(
    const std::string &level, 
    const std::string &line
) const {
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