#include "thread_util.h"
#include "log.h"
#include <sched.h>

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("root");
static uint64_t count = 0;
static uint32_t times = 10000;
static uint8_t thr_num = 5;
static sylar::utils::Spinlock lock;

void thread_func(void* arg) {
    // int i = (int)(uintptr_t)arg;
    for (uint32_t j = 0; j < times; ++j) {
        sylar::utils::Spinlock::Lock spinlock(lock);
        ++count;
        spinlock.unlock();
        sched_yield();
    }
}

void test() {
    std::vector<sylar::utils::Thread::ptr> thrs;
    for (int i = 0; i < thr_num; ++i) {
        thrs.emplace_back(sylar::utils::Thread::Create(
            "thread-" + std::to_string(i + 1), &thread_func, (void*)(uintptr_t)i));
    }

    for (int i = 0; i < thr_num; ++i) {
        if (thrs[i]) {
            thrs[i]->join();
        }
    }

    SYLAR_STREAM_LOG_INFO(g_logger) << "count=" << count;
}

int main() {
    test();
    return 0;
}