#include "log.h"

void test_class() {
    sylar::Logger::ptr logger(new sylar::Logger);
    sylar::LogAppender::ptr appender(new sylar::StdoutLogAppender);
    logger->addAppender(appender);
    sylar::LogEvent::ptr event(new sylar::LogEvent(
        1,
        2,
        __LINE__,
        time(nullptr),
        "thread-1",
        __FILE__));
    event->getSS() << "test class log";
    logger->setLevel(sylar::LogLevel::Level::WARN);
    logger->debug(event);
    logger->info(event);
    logger->warn(event);
    logger->error(event);
    logger->fatal(event);
}

void test_macro() {
    SYLAR_STREAM_LOG_INFO(SYLAR_LOG_NAME("root")) << "test macro SYLAR_STREAM_LOG_INFO";
    SYLAR_MSG_LOG_DEBUG(SYLAR_LOG_ROOT(), "test macro SYLAR_MSG_LOG_DEBUG");
    SYLAR_FMT_LOG_ERROR(SYLAR_LOG_ROOT(), "test macro %s", "SYLAR_FMT_LOG_ERROR");
}

int main() {
    test_class();
    test_macro();
}