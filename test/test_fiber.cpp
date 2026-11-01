#include "fiber_util.h"
#include "log.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("root");

void fiber_func() {
    SYLAR_STREAM_LOG_INFO(g_logger) << "fiber_func begin";
    SYLAR_STREAM_LOG_INFO(g_logger) << "fiber_func YieldHold 1";
    sylar::utils::Fiber::YieldHold();
    SYLAR_STREAM_LOG_INFO(g_logger) << "fiber_func YieldHold 2";
    sylar::utils::Fiber::YieldHold();
    SYLAR_STREAM_LOG_INFO(g_logger) << "fiber_func end";
}

void test() {
    SYLAR_STREAM_LOG_INFO(g_logger) << "test begin";
    sylar::utils::Thread::SetThreadName("t-main");
    sylar::utils::Fiber::SetMainFiber();
    sylar::utils::Fiber::ptr fiber(new sylar::utils::Fiber(fiber_func));
    SYLAR_STREAM_LOG_INFO(g_logger) << "test swapIn 1";
    fiber->swapIn();
    SYLAR_STREAM_LOG_INFO(g_logger) << "test swapIn 2";
    fiber->swapIn();
    SYLAR_STREAM_LOG_INFO(g_logger) << "test swapIn 3";
    fiber->swapIn();
    SYLAR_STREAM_LOG_INFO(g_logger) << "test end";
}

int main() {
    test();
    return 0;
}