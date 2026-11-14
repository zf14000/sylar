#include "scheduler.h"
#include "fiber_util.h"
#include "log.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("root");

void test_cb() {
    SYLAR_STREAM_LOG_INFO(g_logger) << "test_cb begin";
    SYLAR_STREAM_LOG_INFO(g_logger) << "test_cb YieldReady";
    sylar::utils::Fiber::YieldReady();
    SYLAR_STREAM_LOG_INFO(g_logger) << "test_cb end";
}

void test_fiber() {
    SYLAR_STREAM_LOG_INFO(g_logger) << "test_fiber begin";
    SYLAR_STREAM_LOG_INFO(g_logger) << "test_fiber YieldReady";
    sylar::utils::Fiber::YieldReady();
    SYLAR_STREAM_LOG_INFO(g_logger) << "test_fiber end";
}

int main() {
    SYLAR_STREAM_LOG_INFO(g_logger) << "test scheduler begin";
    {
        sylar::Scheduler::ptr scheduler(new sylar::Scheduler(2));
        scheduler->addTask(test_cb);
        auto fiber = sylar::utils::Fiber::ptr(new sylar::utils::Fiber(test_fiber));
        scheduler->addTask(fiber);
        scheduler->start();
    }
    SYLAR_STREAM_LOG_INFO(g_logger) << "test scheduler end";
    return 0;
}