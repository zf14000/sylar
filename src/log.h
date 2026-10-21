#ifndef __SYLAR_LOG_H__
#define __SYLAR_LOG_H__

#include "singleton_util.h"
#include <map>
#include <list>
#include <memory>
#include <string>
#include <cstdint>
#include <fstream>
#include <sstream>

#define SYLAR_LOG_ROOT() sylar::LoggerMgr::GetInstance().getRoot()
#define SYLAR_LOG_NAME(name) sylar::LoggerMgr::GetInstance().getLogger(name)

#define SYLAR_STREAM_LOG_LEVEL(level, logger) \
    if (level >= logger->getLevel()) \
        sylar::LogEventWrap(level, logger, sylar::LogEvent::ptr(new sylar::LogEvent( \
            1,  2,  __LINE__,  time(nullptr),  "thread-1",  __FILE__))).getSS()
#define SYLAR_STREAM_LOG_DEBUG(logger) SYLAR_STREAM_LOG_LEVEL(sylar::LogLevel::Level::DEBUG, logger)
#define SYLAR_STREAM_LOG_INFO(logger) SYLAR_STREAM_LOG_LEVEL(sylar::LogLevel::Level::INFO, logger)
#define SYLAR_STREAM_LOG_WARN(logger) SYLAR_STREAM_LOG_LEVEL(sylar::LogLevel::Level::WARN, logger)
#define SYLAR_STREAM_LOG_ERROR(logger) SYLAR_STREAM_LOG_LEVEL(sylar::LogLevel::Level::ERROR, logger)
#define SYLAR_STREAM_LOG_FATAL(logger) SYLAR_STREAM_LOG_LEVEL(sylar::LogLevel::Level::FATAL, logger)

#define SYLAR_MSG_LOG_LEVEL(level, logger, msg) \
    do { \
        if (level >= logger->getLevel()) \
            sylar::LogEventWrap(level, logger, sylar::LogEvent::ptr(new sylar::LogEvent( \
                1,  2,  __LINE__,  time(nullptr),  "thread-1",  __FILE__))).getSS() << msg; \
    } while (0)
#define SYLAR_MSG_LOG_DEBUG(logger, msg) SYLAR_MSG_LOG_LEVEL(sylar::LogLevel::Level::DEBUG, logger, msg)
#define SYLAR_MSG_LOG_INFO(logger, msg) SYLAR_MSG_LOG_LEVEL(sylar::LogLevel::Level::INFO, logger, msg)
#define SYLAR_MSG_LOG_WARN(logger, msg) SYLAR_MSG_LOG_LEVEL(sylar::LogLevel::Level::WARN, logger, msg)
#define SYLAR_MSG_LOG_ERROR(logger, msg) SYLAR_MSG_LOG_LEVEL(sylar::LogLevel::Level::ERROR, logger, msg)
#define SYLAR_MSG_LOG_FATAL(logger, msg) SYLAR_MSG_LOG_LEVEL(sylar::LogLevel::Level::FATAL, logger, msg)

#define SYLAR_FMT_LOG_LEVEL(level, logger, fmt, ...) \
    for (bool flag = true; flag; flag = false) { \
        if (level >= logger->getLevel()) \
            sylar::LogEventWrap(level, logger, sylar::LogEvent::ptr(new sylar::LogEvent( \
                1,  2,  __LINE__,  time(nullptr),  "thread-1",  __FILE__))).getEvent()->format(fmt, ##__VA_ARGS__); \
    }
#define SYLAR_FMT_LOG_DEBUG(logger, fmt, ...) SYLAR_FMT_LOG_LEVEL(sylar::LogLevel::Level::DEBUG, logger, fmt, ##__VA_ARGS__)
#define SYLAR_FMT_LOG_INFO(logger, fmt, ...) SYLAR_FMT_LOG_LEVEL(sylar::LogLevel::Level::INFO, logger, fmt, ##__VA_ARGS__)
#define SYLAR_FMT_LOG_WARN(logger, fmt, ...) SYLAR_FMT_LOG_LEVEL(sylar::LogLevel::Level::WARN, logger, fmt, ##__VA_ARGS__)
#define SYLAR_FMT_LOG_ERROR(logger, fmt, ...) SYLAR_FMT_LOG_LEVEL(sylar::LogLevel::Level::ERROR, logger, fmt, ##__VA_ARGS__)
#define SYLAR_FMT_LOG_FATAL(logger, fmt, ...) SYLAR_FMT_LOG_LEVEL(sylar::LogLevel::Level::FATAL, logger, fmt, ##__VA_ARGS__)

namespace sylar {

class Logger;
class LogLevel {
public:
    enum Level {
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        UNKNOWN
    };

    static std::string ToString(LogLevel::Level level);
    static LogLevel::Level FromString(const std::string& str);
};

class LogEvent {
public:
    using ptr = std::shared_ptr<LogEvent>;
    LogEvent(
        uint32_t threadId,
        uint32_t fiberId,
        uint32_t fileLine,
        time_t date,
        std::string threadName,
        std::string fileName);
    
    void format(const std::string& fmt, ...);
    void vformat(const std::string& fmt, va_list ap);

    LogLevel::Level getLevel() const { return m_level; }
    uint32_t getThreadId() const { return m_threadId; }
    uint32_t getFiberId() const { return m_fiberId; }
    uint32_t getFileLine() const { return m_fileLine; }
    time_t getDate() const { return m_date; }
    std::string getThreadName() const { return m_threadName; }
    std::string getLogName() const { return m_logName; }
    std::string getFileName() const { return m_fileName; }
    std::stringstream& getSS() { return m_ss; }
    std::string getMsg() { return m_ss.str(); }
    void setLevel(LogLevel::Level level) { m_level = level; }
    void setLogName(const std::string& logName) { m_logName = logName; }
private:
    LogLevel::Level m_level;
    uint32_t m_threadId;
    uint32_t m_fiberId;
    uint32_t m_fileLine;
    time_t m_date;
    std::string m_threadName;
    std::string m_logName;
    std::string m_fileName;
    std::stringstream m_ss;
};

class LogFormatter {
public:
    using ptr = std::shared_ptr<LogFormatter>;

    class FormatItem {
    public:
        using ptr = std::shared_ptr<FormatItem>;
        virtual void format(std::ostream& os, LogEvent::ptr event) = 0;
    };

    LogFormatter(const std::string& pattern = "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n");
    void initItems();
    void format(std::ostream& os, LogEvent::ptr event);
private:
    std::string m_pattern;
    std::list<FormatItem::ptr> m_items;
};

class LogAppender {
public:
    using ptr = std::shared_ptr<LogAppender>;
    LogAppender(LogLevel::Level level = LogLevel::Level::DEBUG);
    virtual void log(LogEvent::ptr event, std::shared_ptr<Logger> logger) = 0;
    LogLevel::Level getLevel() const { return m_level; }
    LogFormatter::ptr getFormatter() const { return m_formatter; }
    void setLevel(LogLevel::Level level) { m_level = level; }
    void setFormatter(LogFormatter::ptr formatter) { m_formatter = formatter; }
private:
    LogLevel::Level m_level;
    LogFormatter::ptr m_formatter;
};

class StdoutLogAppender : public LogAppender {
public:
    void log(LogEvent::ptr event, std::shared_ptr<Logger> logger) override;
};

class FileLogAppender : public LogAppender {
public:
    FileLogAppender(const std::string& file, LogLevel::Level level = LogLevel::Level::DEBUG);
    ~FileLogAppender();
    bool reopen();
    void log(LogEvent::ptr event, std::shared_ptr<Logger> logger) override;
private:
    std::string m_file;
    std::ofstream m_ofs;
};

class Logger : public std::enable_shared_from_this<Logger> {
public:
    using ptr = std::shared_ptr<Logger>;
    Logger(const std::string& name = "root", LogLevel::Level level = LogLevel::Level::DEBUG);
    void log(LogLevel::Level level, LogEvent::ptr event);
    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);
    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);
    void clearAppenders();
    std::string getName() const { return m_name; }
    LogLevel::Level getLevel() const { return m_level; }
    LogFormatter::ptr getFormatter() const { return m_formatter; }
    void setName(const std::string& name) { m_name = name; }
    void setLevel(LogLevel::Level level) { m_level = level; }
    void setFormatter(LogFormatter::ptr formatter) { m_formatter = formatter; }
private:
    std::string m_name;
    LogLevel::Level m_level;
    LogFormatter::ptr m_formatter;
    std::list<LogAppender::ptr> m_appenders;
};

class LogEventWrap {
public:
    LogEventWrap(LogLevel::Level level, Logger::ptr logger, LogEvent::ptr event);
    ~LogEventWrap();
    Logger::ptr getLogger() const { return m_logger; }
    LogEvent::ptr getEvent() const { return m_event; }
    std::stringstream& getSS() { return getEvent()->getSS(); }
private:
    LogLevel::Level m_level;
    Logger::ptr m_logger;
    LogEvent::ptr m_event;
};

class LoggerManager {
public:
    LoggerManager();
    void addLogger(Logger::ptr logger);
    void delLogger(Logger::ptr logger);
    void clearLoggers();
    Logger::ptr getLogger(const std::string& name) const;
    Logger::ptr getRoot() const;
private:
    LoggerManager(const LoggerManager&) = delete;
    LoggerManager(const LoggerManager&&) = delete;
    LoggerManager operator=(const LoggerManager&) = delete;
    LoggerManager operator=(const LoggerManager&&) = delete;
private:
    std::map<std::string, Logger::ptr> m_loggers;
    Logger::ptr m_root;
};

using LoggerMgr = sylar::utils::Singleton<LoggerManager>;

}

#endif