#include "thread_util.h"
#include "log.h"
#include <cstring>
#include <unistd.h>
#include <sys/syscall.h>

namespace sylar {
namespace utils {

static thread_local pid_t t_tid = -1;
static thread_local std::string t_name = "UNKNOWN";

static Logger::ptr g_logger = SYLAR_LOG_NAME("root");

Thread::ptr Thread::Create(const std::string& name, std::function<void(void* arg)> cb, void* arg) {
    Thread::ptr thr(new Thread(name, cb, arg));
    pthread_attr_t attr;
    int rt = pthread_attr_init(&attr);
    if (rt != 0) {
        SYLAR_STREAM_LOG_ERROR(g_logger) << "pthread_attr_init failed, rt=" << rt
            << ", errno=" << errno << ", strerr=" << strerror(errno);
        return nullptr;
    }

    rt = pthread_create(&thr->m_pthread, &attr, &thread_start, thr.get());
    pthread_attr_destroy(&attr);
    if (rt != 0) {
        SYLAR_STREAM_LOG_ERROR(g_logger) << "pthread_create failed, rt=" << rt
            << ", errno=" << errno << ", strerr=" << strerror(errno);
        return nullptr;
    }
    thr->m_state = State::CREATED;
    return thr;
}

pid_t Thread::GetTid() {
    return t_tid;
}

pid_t Thread::GetThreadId() {
    return syscall(SYS_gettid);
}

const std::string& Thread::GetThreadName() {
    return t_name;
}

void Thread::SetThreadName(const std::string& name) {
    t_name = name;
}

Thread::Thread(const std::string& name, std::function<void(void* arg)> cb, void* arg)
    : m_name(name), m_cb(cb), m_arg(arg) {
}

Thread::~Thread() {
    Mutex::Lock lock(m_lock);
    if (m_state != State::CREATED) {
        return;
    }
    m_state = State::DETACHING;
    lock.unlock();

    int rt = pthread_detach(m_pthread);
    if (rt != 0) {
        SYLAR_STREAM_LOG_ERROR(g_logger) << "pthread_detach failed, rt=" << rt
            << ", errno=" << errno << ", strerr=" << strerror(errno);
    }

    lock.lock();
    m_state = State::DETACHED;
}

bool Thread::join() {
    Mutex::Lock lock(m_lock);
    if (m_state != State::CREATED) {
        return false;
    }
    m_state = State::JOINING;
    lock.unlock();

    int rt = pthread_join(m_pthread, nullptr);
    if (rt != 0) {
        SYLAR_STREAM_LOG_ERROR(g_logger) << "pthread_join failed, rt=" << rt
            << ", errno=" << errno << ", strerr=" << strerror(errno);
        lock.lock();
        m_state = State::CREATED;
        return false;
    }

    lock.lock();
    m_state = State::JOINED;
    return true;
}

void* Thread::thread_start(void* arg) {
    Thread* thr = static_cast<Thread*>(arg);
    Thread::ptr self = thr->shared_from_this();
    /*
        shared_from_this 防止 run 里面有异步调用直接使用 this，析构后造成野指针

        void Thread::run() {
            Timer::GetInstance()->addTimer(2000, [this]() {
                SYLAR_STREAM_LOG_INFO(g_logger) << this->m_name;
            });
            if (m_cb) m_cb(m_args);
        }
    */

    if (!self->m_name.empty()) {
        int rt = pthread_setname_np(pthread_self(), self->m_name.c_str());
        if (rt != 0) {
            SYLAR_STREAM_LOG_ERROR(g_logger) << "pthread_setname_np failed, rt=" << rt
                << ", errno=" << errno << ", strerr=" << strerror(errno);
        }
        t_name = self->m_name;
    }

    self->m_tid = syscall(SYS_gettid);
    if (self->m_tid == -1) {
        SYLAR_STREAM_LOG_ERROR(g_logger) << "syscall(SYS_gettid) failed, tid=" << self->m_tid;
    }
    t_tid = self->m_tid;

    self->run();
    return nullptr;
}

void Thread::run() {
    if (m_cb) {
        try {
            m_cb(m_arg);
        } catch (...) {
            SYLAR_STREAM_LOG_ERROR(g_logger) << "thread run throw exception"
                << ", thread_name=" << m_name
                << ", thread_id=" << m_tid;
        } 
    }
}

}
}