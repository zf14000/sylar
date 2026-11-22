#ifndef __SYLAR_SCHEDULER_H__
#define __SYLAR_SCHEDULER_H__

#include "thread_util.h"
#include "fiber_util.h"

namespace sylar {

class Scheduler {
public:
    using ptr = std::shared_ptr<Scheduler>;

    static Scheduler* GetThis();
    Scheduler(uint32_t threadCount = 1, const std::string& name = "scheduler");
    virtual ~Scheduler();

    void start();
    void stop();

    template <typename T>
    void schedule(T f, pid_t t = -1) {
        bool need_tickle = false;
        LockType::Lock lock(m_lock);
        need_tickle |= scheduleNoLock(f, t);
        if (need_tickle) {
            tickle();
        }
    }

    template <typename InputIterator>
    void schedule(InputIterator begin, InputIterator end, pid_t t = -1) {
        bool need_tickle = false;
        LockType::Lock lock(m_lock);
        while (begin != end) {
            need_tickle |= scheduleNoLock(&*begin, t);
            ++begin;
        }
        if (need_tickle) {
            tickle();
        }
    }
private:
    using LockType = sylar::utils::Mutex;

    struct FiberOrCb {
        FiberOrCb() {}

        FiberOrCb(sylar::utils::Fiber::ptr f, pid_t t = -1) : fiber(f), thread(t) {}

        FiberOrCb(sylar::utils::Fiber::ptr* f, pid_t t = -1) : thread(t) {
            fiber.swap(*f);
        }

        FiberOrCb(std::function<void(void)> f, pid_t t = -1) : cb(f), thread(t) {}

        FiberOrCb(std::function<void(void)>* f, pid_t t = -1) : thread(t) {
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

    void run();
    virtual void idle();
    virtual void tickle();

    template<typename T>
    bool scheduleNoLock(T f, pid_t t) {
        // ET 触发方式，避免惊群
        bool need_tickle = m_tasks.empty();
        FiberOrCb task(f, t);
        m_tasks.emplace_back(std::move(task));
        return need_tickle;
    }
protected:
    virtual bool isStop();
    bool hasIdleThread();
private:
    LockType m_lock;
    std::string m_name;
    bool m_stop;
    uint32_t m_threadCount;
    std::atomic<uint32_t> m_activeThreads;
    std::atomic<uint32_t> m_idleThreads;
    std::vector<sylar::utils::Thread::ptr> m_threads;
    std::vector<FiberOrCb> m_tasks;
};

}

#endif