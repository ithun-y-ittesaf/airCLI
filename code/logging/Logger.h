#ifndef LOGGING_LOGGER_H
#define LOGGING_LOGGER_H

#include <string>

using namespace std;

namespace logging {
    class Logger {
    public:
        static void logCritical(const string &action, const string &details);
    };
}

#endif