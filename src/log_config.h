#ifndef __SYLAR_LOG_CONFIG_H__
#define __SYLAR_LOG_CONFIG_H__

#include <string>
#include <vector>

namespace sylar {

struct LogAppenderDefine {
    LogAppenderDefine() {}

    bool operator==(const LogAppenderDefine& oth) const {
        return type == oth.type &&
            level == oth.level &&
            formatter == oth.formatter;
    }

    std::string type;
    std::string file;
    std::string level = "UNKNOWN";
    std::string formatter;
};

struct LoggerDefine {
    LoggerDefine() {}

    bool operator==(const LoggerDefine& oth) const {
        return name == oth.name &&
            level == oth.level &&
            formatter == oth.formatter &&
            appenders == oth.appenders;
    }

    bool operator<(const LoggerDefine& oth) const {
        return name < oth.name;
    }

    std::string name;
    std::string level = "UNKNOWN";
    std::string formatter;
    std::vector<LogAppenderDefine> appenders;
};

}

#endif