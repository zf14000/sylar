#ifndef __SYLAR_UTILS_FIBER_H__
#define __SYLAR_UTILS_FIBER_H__

#include <memory>
#include <functional>
#include <ucontext.h>

namespace sylar {
namespace utils {

class Fiber : public std::enable_shared_from_this<Fiber> {
public:
    using ptr = std::shared_ptr<Fiber>;
    enum State {
        INIT,
        EXEC,
        READY,
        HOLD,
        TERM,
        EXCEPT
    };

    static void SetMainFiber();
    static Fiber::ptr GetCurFiber();
    static uint32_t GetFiberId();
    static uint32_t GetFiberCount();
    Fiber(std::function<void()> cb, size_t stacksize = 0);
    ~Fiber();
    void swapIn();
    static void YieldReady();
    static void YieldHold();
    void reset(std::function<void()> cb);
    uint32_t getId() const { return m_id; }
    State getState() const { return m_state; }
    void setState(State state) { m_state = state; }
private:
    Fiber();
    void swapOut();
    static void SetCurFiber(Fiber* f);
    static void MainFunc();
private:
    uint32_t m_id = 0;
    State m_state = INIT;
    ucontext_t m_ctx;
    void* m_stack = nullptr;
    size_t m_stacksize = 0;
    std::function<void()> m_cb;
};

}
}

#endif