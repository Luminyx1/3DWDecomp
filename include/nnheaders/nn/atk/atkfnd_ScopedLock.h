#pragma once
#include <nn/os/os_Mutex.h>

namespace nn::atk::detail::fnd {
class ScopedMutexLock {
  public:
    /** @brief Acquires a mutex for this scope. @param rMutex Mutex that must outlive the guard. */
    explicit ScopedMutexLock(os::Mutex& rMutex) : mMutex(rMutex) { mMutex.Lock(); }
    /** @brief Releases the mutex acquired at construction. */
    ~ScopedMutexLock() { mMutex.Unlock(); }
    ScopedMutexLock(const ScopedMutexLock&) = delete;
    ScopedMutexLock& operator=(const ScopedMutexLock&) = delete;

  private:
    os::Mutex& mMutex;
};
} // namespace nn::atk::detail::fnd
