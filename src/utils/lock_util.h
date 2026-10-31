#ifndef __SYLAR_UTILS_LOCK_H__
#define __SYLAR_UTILS_LOCK_H__

#include <pthread.h>
#include <semaphore.h>
#include <atomic>
#include <memory>

namespace sylar {
namespace utils {

class NoCopyMove {
public:
    NoCopyMove() = default;
    ~NoCopyMove() = default;
private:
    NoCopyMove(const NoCopyMove&) = delete;
    NoCopyMove& operator=(const NoCopyMove&) const = delete;
    NoCopyMove(NoCopyMove&&) = delete;
    NoCopyMove& operator=(NoCopyMove&&) = delete;
};

template <typename T>
class LockGuard {
public:
    explicit LockGuard(T& lock) : m_lock(lock) {
        if (m_lock.lock()) {
            m_isLock = true;
        }
    }

    ~LockGuard() {
        unlock();
    }

    void lock() {
        if (!m_isLock) {
            m_lock.lock();
            m_isLock = true;
        }
    }

    void unlock() {
        if (m_isLock) {
            if (m_lock.unlock()) {
                m_isLock = false;
            }
        }
    }
private:
    T& m_lock;
    bool m_isLock = false;
};

template <typename T>
class ReadLockGuard {
public:
    explicit ReadLockGuard(T& lock) : m_lock(lock) {
        if (m_lock.rdlock()) {
            m_isLock = true;
        }
    }

    ~ReadLockGuard() {
        unlock();
    }

    void lock() {
        if (!m_isLock) {
            m_lock.rdlock();
            m_isLock = true;
        }
    }

    void unlock() {
        if (m_isLock) {
            if (m_lock.unlock()) {
                m_isLock = false;
            }
        }
    }
private:
    T& m_lock;
    bool m_isLock = false;
};

template <typename T>
class WriteLockGuard {
public:
    explicit WriteLockGuard(T& lock) : m_lock(lock) {
        if (m_lock.wrlock()) {
            m_isLock = true;
        }
    }
    
    ~WriteLockGuard() {
        unlock();
    }

    void lock() {
        if (!m_isLock) {
            m_lock.wrlock();
            m_isLock = true;
        }
    }

    void unlock() {
        if (m_isLock) {
            if (m_lock.unlock()) {
                m_isLock = false;
            }
        }
    }
private:
    T& m_lock;
    bool m_isLock = false;
};

class Semaphore : public NoCopyMove {
public:
    Semaphore(unsigned int count = 0);
    ~Semaphore();
    bool wait();
    bool notify();
private:
    sem_t m_sem;
};

class Mutex : public NoCopyMove {
public:
    using Lock = LockGuard<Mutex>;
    Mutex();
    ~Mutex();
    bool lock();
    bool unlock();
private:
    pthread_mutex_t m_mutex;
};

class Spinlock : public NoCopyMove {
public:
    using Lock = LockGuard<Spinlock>;
    Spinlock();
    ~Spinlock();
    bool lock();
    bool unlock();
private:
    pthread_spinlock_t m_lock;
};

class RWlock : public NoCopyMove {
public:
    using ReadLock = ReadLockGuard<RWlock>; // std::shared_lock<std::shared_mutex>
    using WriteLock = WriteLockGuard<RWlock>; // std::unique_lock<std::shared_mutex>
    
    RWlock();
    ~RWlock();
    bool rdlock();
    bool wrlock();
    bool unlock();
private:
    pthread_rwlock_t m_lock;
};

class CASLock : public NoCopyMove {
public:
    using Lock = LockGuard<CASLock>;
    CASLock();
    ~CASLock();
    bool lock();
    bool unlock();
private:
    std::atomic_flag m_lock;
};

class NullMutex : public NoCopyMove {
public:
    using Lock = LockGuard<NullMutex>;
    NullMutex() {}
    ~NullMutex() {}
    bool lock() { return true; }
    bool unlock() { return true; }
private:
    pthread_mutex_t m_mutex;
};

class NullSpinlock : public NoCopyMove {
public:
    using Lock = LockGuard<NullSpinlock>;
    NullSpinlock() {}
    ~NullSpinlock() {}
    bool lock() { return true; }
    bool unlock() { return true; }
private:
    pthread_spinlock_t m_lock;
};

class NullRWlock : public NoCopyMove {
public:
    using ReadLock = ReadLockGuard<NullRWlock>;
    using WriteLock = WriteLockGuard<NullRWlock>;
    
    NullRWlock() {}
    ~NullRWlock() {}
    bool rdlock() { return true; }
    bool wrlock() { return true; }
    bool unlock() { return true; }
private:
    pthread_rwlock_t m_lock;
};

class NullCASLock : public NoCopyMove {
public:
    using Lock = LockGuard<NullCASLock>;
    NullCASLock() {}
    ~NullCASLock() {}
    bool lock() { return true; }
    bool unlock() { return true; }
private:
    std::atomic_flag m_lock;
};

}
}

#endif