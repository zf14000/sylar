#ifndef __SYLAR_SCHEDULER_H__
#define __SYLAR_SCHEDULER_H__

#include "thread_util.h"
#include "fiber_util.h"

namespace sylar {

class Scheduler {
public:
    using ptr = std::shared_ptr<Scheduler>;

    Scheduler(uint32_t threadCount = 1, const std::string& name = "scheduler");
    ~Scheduler();

    void start();
    void stop();
    bool isStop();

    template <typename FiberOrCb>
    void addTask(FiberOrCb f, pid_t t = -1) {
        bool need_tickle = false;
        LockType::Lock lock(m_lock);
        need_tickle |= addTaskNoLock(f, t);
        if (need_tickle) {
            tickle();
        }
    }

    template <typename InputIterator>
    void addTask(InputIterator begin, InputIterator end, pid_t t = -1) {
        bool need_tickle = false;
        LockType::Lock lock(m_lock);
        while (begin != end) {
            need_tickle |= addTaskNoLock(&*begin, t);
            ++begin;
        }
        if (need_tickle) {
            tickle();
        }
    }
private:
    using LockType = sylar::utils::Mutex;

    struct Task {
        Task() {}

        Task(sylar::utils::Fiber::ptr f, pid_t t = -1) : fiber(f), thread(t) {}

        Task(sylar::utils::Fiber::ptr* f, pid_t t = -1) : thread(t) {
            fiber.swap(*f);
        }

        Task(std::function<void(void)> f, pid_t t = -1) : cb(f), thread(t) {}

        Task(std::function<void(void)>* f, pid_t t = -1) : thread(t) {
            cb.swap(*f);
        }

        void reset() {
            fiber.reset();
            cb = nullptr;
            thread = -1;
        }

        sylar::utils::Fiber::ptr fiber = nullptr;
        std::function<void(void)> cb = nullptr;
        pid_t thread = -1;
    };

    virtual void run();
    virtual void idle();
    virtual void tickle();

    template<typename FiberOrCb>
    bool addTaskNoLock(FiberOrCb f, pid_t t) {
        // ET 触发方式，避免惊群
        bool need_tickle = m_tasks.empty();
        Task task(f, t);
        m_tasks.emplace_back(std::move(task));
        return need_tickle;
    }
private:
    LockType m_lock;
    std::string m_name;
    bool m_stop;
    uint32_t m_threadCount;
    std::atomic<uint32_t> m_activeThreads;
    std::atomic<uint32_t> m_idleThreads;
    std::vector<sylar::utils::Thread::ptr> m_threads;
    std::vector<Task> m_tasks;
};

}

#endif