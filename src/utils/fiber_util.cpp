#include "fiber_util.h"
#include "assert_util.h"
#include "config.h"
#include <atomic>

namespace sylar {
namespace utils {

static thread_local Fiber::ptr t_main_fiber;
static thread_local Fiber* t_cur_fiber;
static std::atomic<uint32_t> s_fiber_id = 0;
static std::atomic<uint32_t> s_fiber_count = 0;
static Logger::ptr g_logger = SYLAR_LOG_NAME("root");
static ConfigVar<size_t>::ptr g_fiber_stack_size =
    Config::Lookup<size_t>("fiber.stack_size", 1024 * 128, "fiber stack size");

void Fiber::SetMainFiber() {
    if (t_main_fiber) {
        return;
    }
    t_main_fiber = Fiber::ptr(new Fiber);
    SetCurFiber(t_main_fiber.get());
}

Fiber* Fiber::GetCurFiber() {
    return t_cur_fiber;
}

void Fiber::SetCurFiber(Fiber* f) {
    t_cur_fiber = f;
}

uint32_t Fiber::GetFiberId() {
    if (t_cur_fiber) {
        return t_cur_fiber->getId();
    }
    return 0;
}

uint32_t Fiber::GetFiberCount() {
    return s_fiber_count;
}

Fiber::Fiber() {
    ++s_fiber_count;
    m_state = EXEC;
    SYLAR_STREAM_LOG_DEBUG(g_logger) << "Fiber::Fiber id=" << m_id;
}

Fiber::Fiber(std::function<void()> cb, size_t stacksize)
        : m_id(++s_fiber_id), m_cb(cb) {
    ++s_fiber_count;
    m_stacksize = stacksize ? stacksize : g_fiber_stack_size->getValue();
    m_stack = malloc(m_stacksize);
    SYLAR_ASSERT(m_stack != nullptr);
    SYLAR_ASSERT(getcontext(&m_ctx) != -1);
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;
    m_ctx.uc_link = nullptr;
    makecontext(&m_ctx, &Fiber::MainFunc, 0);
    SYLAR_STREAM_LOG_DEBUG(g_logger) << "Fiber::Fiber id=" << m_id;
}

Fiber::~Fiber() {
    --s_fiber_count;
    if (m_id != 0) {
        SYLAR_ASSERT(m_state == INIT || m_state == TERM || m_state == EXCEPT);
        if (m_stack) {
            free(m_stack);
        }
    } else {
        SYLAR_ASSERT(m_state == EXEC);
        if (GetCurFiber() == this) {
            SetCurFiber(nullptr);
        }
    }
    SYLAR_STREAM_LOG_DEBUG(g_logger) << "Fiber::~Fiber id=" << m_id;
}

void Fiber::swapIn() {
    SYLAR_ASSERT(m_state != EXEC && m_state != TERM && m_state != EXCEPT);
    SetCurFiber(this);
    m_state = EXEC;
    t_main_fiber->m_state = HOLD;
    SYLAR_ASSERT(swapcontext(&t_main_fiber->m_ctx, &t_cur_fiber->m_ctx) != -1);
}

void Fiber::swapOut() {
    SetCurFiber(t_main_fiber.get());
    t_main_fiber->m_state = EXEC;
    SYLAR_ASSERT(swapcontext(&m_ctx, &t_main_fiber->m_ctx) != -1);
}

void Fiber::YieldReady() {
    auto cur = GetCurFiber();
    cur->m_state = READY;
    SetCurFiber(t_main_fiber.get());
    t_main_fiber->m_state = EXEC;
    SYLAR_ASSERT(swapcontext(&cur->m_ctx, &t_main_fiber->m_ctx) != -1);
}

void Fiber::YieldHold() {
    auto cur = GetCurFiber();
    cur->m_state = HOLD;
    SetCurFiber(t_main_fiber.get());
    t_main_fiber->m_state = EXEC;
    SYLAR_ASSERT(swapcontext(&cur->m_ctx, &t_main_fiber->m_ctx) != -1);
}

void Fiber::MainFunc() {
    auto cur = GetCurFiber();
    try {
        if (cur->m_cb) {
            cur->m_cb();
            cur->m_cb = nullptr;
            cur->m_state = TERM;
        }
    } catch (const std::exception& e) {
        SYLAR_STREAM_LOG_ERROR(g_logger) << "Fiber Except: " << e.what()
            << ", fiber_id=" << cur->getId() << std::endl
            << sylar::utils::BacktraceToString();
        cur->m_state = EXCEPT;
    } catch (...) {
        SYLAR_STREAM_LOG_ERROR(g_logger) << "Fiber Except"
            << ", fiber_id=" << cur->getId() << std::endl
            << sylar::utils::BacktraceToString();
        cur->m_state = EXCEPT;
    }
    cur->swapOut();

    SYLAR_STREAM_LOG_FATAL(g_logger) << "Code cannot reach here, fiber_id=" << cur->getId();
    SYLAR_ASSERT(false);
}

}
}