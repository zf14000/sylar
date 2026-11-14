#include "scheduler.h"
#include "log.h"
#include <unistd.h>

namespace sylar {

static Logger::ptr g_logger = SYLAR_LOG_NAME("root");

Scheduler::Scheduler(uint32_t threadCount, const std::string& name)
        :m_name(name)
        ,m_stop(true)
        ,m_threadCount(threadCount)
        ,m_activeThreads(0)
        ,m_idleThreads(threadCount) {
    SYLAR_STREAM_LOG_DEBUG(g_logger) << "Scheduler::Scheduler";
}

Scheduler::~Scheduler() {
    SYLAR_STREAM_LOG_DEBUG(g_logger) << "Scheduler::~Scheduler begin";
    stop();
    SYLAR_STREAM_LOG_DEBUG(g_logger) << "Scheduler::~Scheduler end";
}

void Scheduler::start() {
    SYLAR_STREAM_LOG_DEBUG(g_logger) << "Scheduler::start";
    if (!m_stop) {
        return;
    }
    m_stop = false;
    LockType::Lock lock(m_lock);
    m_threads.resize(m_threadCount);
    for (uint32_t i = 0; i < m_threadCount; ++i) {
        m_threads[i] = sylar::utils::Thread::Create(
            "t-" + m_name + "-" + std::to_string(i + 1),
            std::bind(&Scheduler::run, this));
    }
}

void Scheduler::stop() {
    SYLAR_STREAM_LOG_DEBUG(g_logger) << "Scheduler::stop";
    if (isStop()) {
        return;
    }
    m_stop = true;
    LockType::Lock lock(m_lock);
    auto threads = m_threads;
    lock.unlock();
    for (auto& t : threads) {
        t->join();
    }
}

bool Scheduler::isStop() {
    LockType::Lock lock(m_lock);
    return m_stop && m_activeThreads == 0 && m_tasks.empty();
}

void Scheduler::run() {
    sylar::utils::Fiber::SetMainFiber();
    sylar::utils::Fiber::ptr cb_fiber;
    sylar::utils::Fiber::ptr idle_fiber = std::make_shared<sylar::utils::Fiber>(std::bind(&Scheduler::idle, this));

    while (true) {
        Task task;
        bool found = false;
        bool need_tickle = false;

        LockType::Lock lock(m_lock);
        auto it = m_tasks.begin();
        for (; it != m_tasks.end(); ++it) {
            if (it->thread != -1 && it->thread != sylar::utils::Thread::GetThreadId()) {
                need_tickle = true;
                continue;
            }

            if (it->fiber && it->fiber->getState() == sylar::utils::Fiber::State::EXEC) {
                continue;
            }

            found = true;
            task = *it;
            m_tasks.erase(it);
            break;
        }
        need_tickle |= (it != m_tasks.end());
        lock.unlock();

        if (need_tickle) {
            tickle();
        }

        if (found) {
            ++m_activeThreads;
            if (task.fiber) {
                if (task.fiber->getState() == sylar::utils::Fiber::State::INIT ||
                        task.fiber->getState() == sylar::utils::Fiber::State::READY ||
                        task.fiber->getState() == sylar::utils::Fiber::State::HOLD) {
                    task.fiber->swapIn();
                    if (task.fiber->getState() == sylar::utils::Fiber::State::READY) {
                        lock.lock();
                        m_tasks.emplace_back(task);
                        lock.unlock();
                    }
                }
            } else if (task.cb) {
                if (cb_fiber) {
                    cb_fiber->reset(task.cb);
                } else {
                    cb_fiber = std::make_shared<sylar::utils::Fiber>(task.cb);
                }
                cb_fiber->swapIn();
                if (cb_fiber->getState() == sylar::utils::Fiber::State::READY) {
                    lock.lock();
                    m_tasks.emplace_back(Task(cb_fiber));
                    lock.unlock();
                }
                cb_fiber.reset();
            } else {
                SYLAR_STREAM_LOG_ERROR(g_logger) << "task is invalid, fiber and cb are null";
            }
            --m_activeThreads;
        } else {
            if (idle_fiber->getState() == sylar::utils::Fiber::State::TERM) {
                break;
            }
            ++m_idleThreads;
            idle_fiber->swapIn();
            --m_idleThreads;
        }
    }
}

void Scheduler::idle() {
    while (!isStop()) {
        SYLAR_STREAM_LOG_DEBUG(g_logger) << "Scheduler::idle";
        sleep(2);
        sylar::utils::Fiber::YieldHold();
    }
}

void Scheduler::tickle() {
    SYLAR_STREAM_LOG_DEBUG(g_logger) << "Scheduler::tickle";
}

}