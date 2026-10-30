#include "log_config.h"
#include "config.h"

namespace sylar {

static Logger::ptr g_logger = SYLAR_LOG_NAME("root");
static ConfigVar<std::set<LoggerDefine> >::ptr g_log_config = Config::Lookup("log", std::set<LoggerDefine>(), "log config");

struct LogIniter {
    LogIniter() {
        g_log_config->addListener([](const std::set<LoggerDefine>& oldVal, const std::set<LoggerDefine>& newVal) {
            if (oldVal == newVal) {
                return;
            }

            for (auto it_new = newVal.begin(); it_new != newVal.end(); ++it_new) {
                auto it_old = oldVal.find(*it_new);
                if (it_old != oldVal.end()) {
                    if (*it_old == *it_new) {
                        continue;
                    }
                    // 修改
                    LoggerMgr::GetInstance().delLogger(it_new->name);
                }

                // 新增
                LogLevel::Level level = it_new->level == "UNKNOWN" ?
                    LogLevel::Level::DEBUG : LogLevel::FromString(it_new->level);
                Logger::ptr logger(new Logger(it_new->name, level));

                if (!it_new->formatter.empty()) {
                    logger->setFormatter(LogFormatter::ptr(new LogFormatter(it_new->formatter)));
                } else {
                    logger->setFormatter(LogFormatter::ptr(new LogFormatter));
                }

                for (auto& i : it_new->appenders) {
                    LogAppender::ptr appender;
                    LogLevel::Level level = i.level == "UNKNOWN" ?
                        LogLevel::Level::DEBUG : LogLevel::FromString(i.level);
                    if (i.type == "StdoutLogAppender") {
                        appender = std::make_shared<StdoutLogAppender>(level);
                    } else if (i.type == "FileLogAppender") {
                        appender = std::make_shared<FileLogAppender>(i.file, level);
                    }
                    logger->addAppender(appender);
                    if (!i.formatter.empty()) {
                        appender->setFormatter(LogFormatter::ptr(new LogFormatter(i.formatter)));
                    }
                }

                LoggerMgr::GetInstance().addLogger(logger->getName(), logger);
            }

            for (auto it_old = oldVal.begin(); it_old != oldVal.end(); ++it_old) {
                auto it_new = newVal.find(*it_old);
                if (it_new == newVal.end()) {
                    // 删除
                    LoggerMgr::GetInstance().delLogger(it_old->name);
                }
            }
        });

        const std::string file = "/home/zf/code/sylar/conf/log_config.yml";
        sylar::Config::LoadYaml(file);
    }
};

static LogIniter _logIniter;

}