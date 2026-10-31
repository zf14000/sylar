#include "lock_util.h"
#include "log.h"
#include <cstring>

namespace sylar {
namespace utils {

static Logger::ptr g_logger = SYLAR_LOG_NAME("root");

Semaphore::Semaphore(unsigned int count) {
    int rt = sem_init(&m_sem, 0, count);
    if (rt != 0) {
        SYLAR_STREAM_LOG_ERROR(g_logger) << "pthread_mutex_init failed"
            << ", rt=" << rt
            << ", errno=" << errno
            << ", strerr=" << strerror(errno);
    }
}

Semaphore::~Semaphore() {
    int rt = sem_destroy(&m_sem);
    if (rt != 0) {
        SYLAR_STREAM_LOG_ERROR(g_logger) << "pthread_mutex_destroy failed"
            << ", rt=" << rt
            << ", errno=" << errno
            << ", strerr=" << strerror(errno);
    }
}

bool Semaphore::wait() {
    int rt = sem_wait(&m_sem);
    if (rt != 0) {
        SYLAR_STREAM_LOG_ERROR(g_logger) << "sem_wait failed"
            << ", rt=" << rt
            << ", errno=" << errno
            << ", strerr=" << strerror(errno);
        return false;
    }
    return true;
}

bool Semaphore::notify() {
    int rt = sem_post(&m_sem);
    if (rt != 0) {
        SYLAR_STREAM_LOG_ERROR(g_logger) << "sem_post failed"
            << ", rt=" << rt
            << ", errno=" << errno
            << ", strerr=" << strerror(errno);
        return false;
    }
    return true;
}

Mutex::Mutex() {
    int rt = pthread_mutex_init(&m_mutex, nullptr);
    if (rt != 0) {
        SYLAR_STREAM_LOG_ERROR(g_logger) << "pthread_mutex_init failed"
            << ", rt=" << rt
            << ", errno=" << errno
            << ", strerr=" << strerror(errno);
    }
}

Mutex::~Mutex() {
    int rt = pthread_mutex_destroy(&m_mutex);
    if (rt != 0) {
        SYLAR_STREAM_LOG_ERROR(g_logger) << "pthread_mutex_destroy failed"
            << ", rt=" << rt
            << ", errno=" << errno
            << ", strerr=" << strerror(errno);
    }
}

bool Mutex::lock() {
    int rt = pthread_mutex_lock(&m_mutex);
    if (rt != 0) {
        SYLAR_STREAM_LOG_ERROR(g_logger) << "pthread_mutex_lock failed"
            << ", rt=" << rt
            << ", errno=" << errno
            << ", strerr=" << strerror(errno);
            return false;
    }
    return true;
}

bool Mutex::unlock() {
    int rt = pthread_mutex_unlock(&m_mutex);
    if (rt != 0) {
        SYLAR_STREAM_LOG_ERROR(g_logger) << "pthread_mutex_unlock failed"
            << ", rt=" << rt
            << ", errno=" << errno
            << ", strerr=" << strerror(errno);
        return false;
    }
    return true;
}

Spinlock::Spinlock() {
    int rt = pthread_spin_init(&m_lock, PTHREAD_PROCESS_PRIVATE);
    if (rt != 0) {
        SYLAR_STREAM_LOG_ERROR(g_logger) << "pthread_spin_init failed"
            << ", rt=" << rt
            << ", errno=" << errno
            << ", strerr=" << strerror(errno);
    }
}

Spinlock::~Spinlock() {
    int rt = pthread_spin_destroy(&m_lock);
    if (rt != 0) {
        SYLAR_STREAM_LOG_ERROR(g_logger) << "pthread_spin_destroy failed"
            << ", rt=" << rt
            << ", errno=" << errno
            << ", strerr=" << strerror(errno);
    }
}

bool Spinlock::lock() {
    int rt = pthread_spin_lock(&m_lock);
    if (rt != 0) {
        SYLAR_STREAM_LOG_ERROR(g_logger) << "pthread_spin_lock failed"
            << ", rt=" << rt
            << ", errno=" << errno
            << ", strerr=" << strerror(errno);
        return false;
    }
    return true;
}

bool Spinlock::unlock() {
    int rt = pthread_spin_unlock(&m_lock);
    if (rt != 0) {
        SYLAR_STREAM_LOG_ERROR(g_logger) << "pthread_spin_unlock failed"
            << ", rt=" << rt
            << ", errno=" << errno
            << ", strerr=" << strerror(errno);
        return false;
    }
    return true;
}

RWlock::RWlock() {
    int rt = pthread_rwlock_init(&m_lock, nullptr);
    if (rt != 0) {
        SYLAR_STREAM_LOG_ERROR(g_logger) << "pthread_rwlock_init failed"
            << ", rt=" << rt
            << ", errno=" << errno
            << ", strerr=" << strerror(errno);
    }
}

RWlock::~RWlock() {
    int rt = pthread_rwlock_destroy(&m_lock);
    if (rt != 0) {
        SYLAR_STREAM_LOG_ERROR(g_logger) << "pthread_rwlock_destroy failed"
            << ", rt=" << rt
            << ", errno=" << errno
            << ", strerr=" << strerror(errno);
    }
}

bool RWlock::rdlock() {
    int rt = pthread_rwlock_rdlock(&m_lock);
    if (rt != 0) {
        SYLAR_STREAM_LOG_ERROR(g_logger) << "pthread_rwlock_rdlock failed"
            << ", rt=" << rt
            << ", errno=" << errno
            << ", strerr=" << strerror(errno);
        return false;
    }
    return true;
}

bool RWlock::wrlock() {
    int rt = pthread_rwlock_wrlock(&m_lock);
    if (rt != 0) {
        SYLAR_STREAM_LOG_ERROR(g_logger) << "pthread_rwlock_wrlock failed"
            << ", rt=" << rt
            << ", errno=" << errno
            << ", strerr=" << strerror(errno);
        return false;
    }
    return true;
}

bool RWlock::unlock() {
    int rt = pthread_rwlock_unlock(&m_lock);
    if (rt != 0) {
        SYLAR_STREAM_LOG_ERROR(g_logger) << "pthread_rwlock_unlock failed"
            << ", rt=" << rt
            << ", errno=" << errno
            << ", strerr=" << strerror(errno);
        return false;
    }
    return true;
}

CASLock::CASLock() {
    m_lock.clear();
}

CASLock::~CASLock() {
}

bool CASLock::lock() {
    while(std::atomic_flag_test_and_set_explicit(&m_lock, std::memory_order_acquire));
    return true;
}

bool CASLock::unlock() {
    std::atomic_flag_clear_explicit(&m_lock, std::memory_order_release);
    return true;
}

}
}