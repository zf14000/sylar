#ifndef __SYLAR_UTILS_THREAD_H__
#define __SYLAR_UTILS_THREAD_H__

#include <lock_util.h>
#include <string>
#include <memory>
#include <functional>

namespace sylar {
namespace utils {

class Thread : public std::enable_shared_from_this<Thread> {
public:
    using ptr = std::shared_ptr<Thread>;

    static Thread::ptr Create(const std::string& name, std::function<void(void* arg)> cb, void* arg);
    static pid_t GetTid();
    static const std::string& GetName();

    ~Thread();
    bool join();
    pid_t getTid() const { return m_tid; }
    const std::string& getName() const { return m_name; }
private:
    Thread(const std::string& name, std::function<void(void* arg)> cb, void* arg);
    static void* thread_start(void* arg);
    virtual void run();

    enum class State {
        INIT,
        CREATED,
        JOINING,
        JOINED,
        DETACHING,
        DETACHED
    };
private:
    // 代码不规范导致Thread::ptr引用计数提前减到0，join和析构同时发生时，加锁防止线程同时被join和detach
    Mutex m_lock;
    pid_t m_tid = -1;
    pthread_t m_pthread = 0;
    std::string m_name;
    std::function<void(void* arg)> m_cb;
    void* m_arg;
    State m_state = State::INIT;
};

}
}

#endif