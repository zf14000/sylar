#include "log.h"
#include "fs_util.h"
#include "time_util.h"
#include <iostream>
#include <cstring>
#include <stdarg.h>

namespace sylar {

std::string LogLevel::ToString(LogLevel::Level level) {
#define XX(name) \
    if (level == LogLevel::Level::name) { \
        return #name; \
    }
    XX(DEBUG)
    XX(INFO)
    XX(WARN)
    XX(ERROR)
    XX(FATAL)
#undef XX
    return "UNKNOWN";
}

LogLevel::Level LogLevel::FromString(const std::string& str) {
#define XX(level, name) \
    if (str == #name) { \
        return LogLevel::Level::level; \
    }
    XX(DEBUG, DEBUG)
    XX(INFO, INFO)
    XX(WARN, WARN)
    XX(ERROR, ERROR)
    XX(FATAL, FATAL)
    XX(DEBUG, debug)
    XX(INFO, info)
    XX(WARN, warn)
    XX(ERROR, error)
    XX(FATAL, fatal)
#undef XX
    return LogLevel::Level::UNKNOWN;
}

LogEvent::LogEvent(uint32_t threadId,
    uint32_t fiberId,
    uint32_t fileLine,
    time_t date,
    std::string threadName,
    std::string fileName)
    :m_threadId(threadId)
    ,m_fiberId(fiberId)
    ,m_fileLine(fileLine)
    ,m_date(date)
    ,m_threadName(threadName)
    ,m_fileName(fileName) {
}

void LogEvent::format(const std::string& fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vformat(fmt, ap);
    va_end(ap);
}

void LogEvent::vformat(const std::string& fmt, va_list ap) {
    va_list aq;
    va_copy(aq, ap);
    int n = vsnprintf(nullptr, 0, fmt.c_str(), aq);
    va_end(aq);

    if (n < 0) {
        std::cerr << "vsnprintf calculate len errno=" << errno
                  << ", strerr=" << strerror(errno) << "\n";
        return;
    }

    std::string buf(n + 1, '\0');
    n = vsnprintf(buf.data(), buf.size(), fmt.c_str(), ap);
    if (n < 0) {
        std::cerr << "vsnprintf format errno=" << errno
                  << ", strerr=" << strerror(errno) << "\n";
        return;
    }
    buf.resize(n);
    getSS() << buf;
}

class LevelFormatItem : public LogFormatter::FormatItem {
public:
    LevelFormatItem(const std::string& param = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << LogLevel::ToString(event->getLevel());
    }
};

class ThreadIdFormatItem : public LogFormatter::FormatItem {
public:
    ThreadIdFormatItem(const std::string& param = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getThreadId();
    }
};

class FiberIdFormatItem : public LogFormatter::FormatItem {
public:
    FiberIdFormatItem(const std::string& param = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getFiberId();
    }
};

class FileLineFormatItem : public LogFormatter::FormatItem {
public:
    FileLineFormatItem(const std::string& param = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getFileLine();
    }
};

class DateFormatItem : public LogFormatter::FormatItem {
public:
    DateFormatItem(const std::string& fmt = "%Y/%m/%d %H:%M:%S") : m_fmt(fmt) {
        if (m_fmt.empty()) {
            m_fmt = "%Y/%m/%d %H:%M:%S";
        }
    }
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << sylar::utils::Time::ToString(event->getDate(), m_fmt);
    }
private:
    std::string m_fmt;
};

class ThreadNameFormatItem : public LogFormatter::FormatItem {
public:
    ThreadNameFormatItem(const std::string& param = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getThreadName();
    }
};

class LogNameFormatItem : public LogFormatter::FormatItem {
public:
    LogNameFormatItem(const std::string& param = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getLogName();
    }
};

class FileNameFormatItem : public LogFormatter::FormatItem {
public:
    FileNameFormatItem(const std::string& param = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getFileName();
    }
};

class MessageFormatItem : public LogFormatter::FormatItem {
public:
    MessageFormatItem(const std::string& msg = "") : m_msg(msg) {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getMsg();
    }
private:
    std::string m_msg;
};

class StringFormatItem : public LogFormatter::FormatItem {
public:
    StringFormatItem(const std::string& str = "") : m_str(str) {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << m_str;
    }
private:
    std::string m_str;
};

class TabFormatItem : public LogFormatter::FormatItem {
public:
    TabFormatItem(const std::string& param = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << "\t";
    }
};

class NewLineFormatItem : public LogFormatter::FormatItem {
public:
    NewLineFormatItem(const std::string& param = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << std::endl;
    }
};

LogFormatter::LogFormatter(const std::string& pattern) : m_pattern(pattern) {
    if (m_pattern.empty()) {
        m_pattern = "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n";
    }
    initItems();
}

void LogFormatter::initItems() {
    // %d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n
    // %d{%Y-%m-%d %H:%M:%S}|%t|%N|%f|%p|%c|%F:%l|%m%n
    // str%type{fmt}
    size_t i = 0;
    std::string str;
    std::list<std::pair<std::string, std::string> > item_list;
    while (i < m_pattern.size()) {
        if (m_pattern[i] == '%') {
            if (i + 1 == m_pattern.size()) {
                std::cerr << "invalid pattern=" << m_pattern
                    << "invalid index=" << i + 1 << std::endl;;
                return;
            }

            if (m_pattern[i + 1] == '%' ||
                m_pattern[i + 1] == '{' ||
                m_pattern[i + 1] == '}') {
                str.append(1, m_pattern[i + 1]);
                i += 2;
                continue;
            }

            if (!isalpha(m_pattern[i + 1])) {
                std::cerr << "invalid pattern=" << m_pattern
                    << "invalid index=" << i + 1 << std::endl;;
                return;
            }

            // str
            if (!str.empty()) {
                item_list.push_back(std::make_pair("s", str));
                str.clear();
            }

            // type
            std::string type = std::string(1, m_pattern[i + 1]);
            if (i + 2 == m_pattern.size() || m_pattern[i + 2] != '{') {
                item_list.push_back(std::make_pair(type, ""));
                i += 2;
                continue;
            }

            // fmt
            size_t fmt_end = m_pattern.find_first_of('}', i + 2);
            if (fmt_end == std::string::npos) {
                std::cerr << "invalid pattern=" << m_pattern
                    << "invalid index=" << i + 2 << std::endl;;
                return;
            }
            size_t fmt_begin = i + 3;
            std::string fmt = m_pattern.substr(fmt_begin, fmt_end - fmt_begin);
            item_list.push_back(std::make_pair(type, fmt));
            i = fmt_end;
        } else {
            if (m_pattern[i] == '{' || m_pattern[i] == '}') {
                std::cerr << "invalid pattern=" << m_pattern
                    << "invalid index=" << i << std::endl;;
                return;
            }
            str.append(1, m_pattern[i]);
        }
        ++i;
    }

    if (!str.empty()) {
        item_list.push_back(std::make_pair("s", str));
    }

    for (auto& i : item_list) {
#define XX(c, C) \
        if (#c == i.first) { \
            m_items.push_back(FormatItem::ptr(new C(i.second))); \
        }
        XX(m, MessageFormatItem)           //m:消息
        XX(s, StringFormatItem)            //m:字符串
        XX(p, LevelFormatItem)             //p:日志级别
        XX(c, LogNameFormatItem)           //c:日志名称
        XX(t, ThreadIdFormatItem)          //t:线程id
        XX(n, NewLineFormatItem)           //n:换行
        XX(d, DateFormatItem)              //d:时间
        XX(f, FileNameFormatItem)          //f:文件名
        XX(l, FileLineFormatItem)          //l:行号
        XX(T, TabFormatItem)               //T:Tab
        XX(F, FiberIdFormatItem)           //F:协程id
        XX(N, ThreadNameFormatItem)        //N:线程名称
#undef XX
    }
}

void LogFormatter::format(std::ostream& os, LogEvent::ptr event) {
    for (auto& item : m_items) {
        item->format(os, event);
    }
}

LogAppender::LogAppender(LogLevel::Level level) : m_level(level) {
}

void StdoutLogAppender::log(LogEvent::ptr event, std::shared_ptr<Logger> logger) {
    if (event->getLevel() < getLevel()) {
        return;
    }
    if (!getFormatter()) {
        setFormatter(logger->getFormatter());
    }
    getFormatter()->format(std::cout, event);
}

FileLogAppender::FileLogAppender(const std::string& file, LogLevel::Level level)
    :LogAppender(level)
    ,m_file(file) {
    
    if (!sylar::utils::FS::Exists(m_file)) {
        try {
            sylar::utils::FS::CreateFile(m_file);
        } catch(sylar::utils::FSException& e) {
            std::cerr << e.what() << std::endl;
        }
    }
}

FileLogAppender::~FileLogAppender() {
    if (m_ofs.is_open()) {
        m_ofs.close();
    }
}

bool FileLogAppender::reopen() {
    if (m_ofs.is_open()) {
        m_ofs.close();
    }
    m_ofs.open(m_file);
    return m_ofs.is_open();
}

void FileLogAppender::log(LogEvent::ptr event, std::shared_ptr<Logger> logger) {
    if (event->getLevel() < getLevel()) {
        return;
    }
    reopen();
    getFormatter()->format(m_ofs, event);
}

Logger::Logger(const std::string& name, LogLevel::Level level)
    :m_name(name)
    ,m_level(level) {
    m_formatter.reset(new LogFormatter);
}

void Logger::log(LogLevel::Level level, LogEvent::ptr event) {
    if (level < m_level) {
        return;
    }
    event->setLevel(level);
    event->setLogName(m_name);
    for (auto& appender : m_appenders) {
        appender->log(event, shared_from_this());
    }
}

void Logger::debug(LogEvent::ptr event) {
    log(LogLevel::Level::DEBUG, event);
}

void Logger::info(LogEvent::ptr event) {
    log(LogLevel::Level::INFO, event);
}

void Logger::warn(LogEvent::ptr event) {
    log(LogLevel::Level::WARN, event);
}

void Logger::error(LogEvent::ptr event) {
    log(LogLevel::Level::ERROR, event);
}

void Logger::fatal(LogEvent::ptr event) {
    log(LogLevel::Level::FATAL, event);
}

void Logger::addAppender(LogAppender::ptr appender) {
    m_appenders.push_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender) {
    m_appenders.remove(appender);
}

void Logger::clearAppenders() {
    m_appenders.clear();
}

LogEventWrap::LogEventWrap(LogLevel::Level level, Logger::ptr logger, LogEvent::ptr event)
    :m_level(level)
    ,m_logger(logger)
    ,m_event(event) {
}

LogEventWrap::~LogEventWrap() {
    m_logger->log(m_level, m_event);
}

LoggerManager::LoggerManager() {
    m_root.reset(new Logger);
    m_root->addAppender(LogAppender::ptr(new StdoutLogAppender));
    addLogger(m_root);
}

void LoggerManager::addLogger(Logger::ptr logger) {
    m_loggers[logger->getName()] = logger;
}

void LoggerManager::delLogger(Logger::ptr logger) {
    m_loggers.erase(logger->getName());
}

void LoggerManager::clearLoggers() {
    m_loggers.clear();
}

Logger::ptr LoggerManager::getLogger(const std::string& name) const {
    auto it = m_loggers.find(name);
    if (it == m_loggers.end()) {
        return nullptr;
    }
    return it->second;
}

Logger::ptr LoggerManager::getRoot() const {
    return m_root;
}


}