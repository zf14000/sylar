#ifndef __SYLAR_UTILS_FIBER_H__
#define __SYLAR_UTILS_FIBER_H__

#include <memory>
#include <functional>
#include <ucontext.h>

namespace sylar {
namespace utils {

class Fiber : std::enable_shared_from_this<Fiber> {
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
    static uint32_t GetFiberId();
    static uint32_t GetFiberCount();
    Fiber(std::function<void()> cb, size_t stacksize = 0);
    ~Fiber();
    void swapIn();
    static void YieldReady();
    static void YieldHold();
    uint32_t getId() const { return m_id; }
private:
    Fiber();
    void swapOut();
    static Fiber* GetCurFiber();
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