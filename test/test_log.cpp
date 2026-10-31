#include "log.h"
#include "thread_util.h"

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

void printf_plus(void*) {
    while (true) {
        SYLAR_STREAM_LOG_INFO(SYLAR_LOG_NAME("system")) << "+++++++++++++++++++++++++";
    }
}

void printf_minus(void*) {
    while (true) {
        SYLAR_STREAM_LOG_INFO(SYLAR_LOG_NAME("system")) << "-------------------------";
    }
}

void test_thread() {
    const uint8_t thread_num = 2;
    std::vector<sylar::utils::Thread::ptr> thrs;
    for (uint8_t i = 0; i < thread_num; ++i) {
        if (i % 2 == 0) {
            thrs.emplace_back(
                sylar::utils::Thread::Create("thread-" + std::to_string(i + 1), &printf_plus, nullptr));
        } else {
            thrs.emplace_back(
                sylar::utils::Thread::Create("thread-" + std::to_string(i + 1), &printf_minus, nullptr));
        }
    }
    for (uint8_t i = 0; i < thread_num; ++i) {
        if (thrs[i]) {
            thrs[i]->join();
        }
    }
}

int main() {
    test_class();
    test_macro();
    // test_thread();
}