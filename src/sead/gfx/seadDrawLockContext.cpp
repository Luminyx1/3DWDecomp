#include "gfx/seadDrawLockContext.h"

namespace sead
{
DrawLockContext::DrawLockContext() = default;

/**
 * Initializes the lock; does nothing on this platform.
 */
void DrawLockContext::initialize(Heap*) {}

/**
 * Locks the critical section.
 */
void DrawLockContext::lock()
{
    mCriticalSection.lock();
}

/**
 * Unlocks the critical section.
 */
void DrawLockContext::unlock()
{
    mCriticalSection.unlock();
}

/**
 * Generates host IO messages; does nothing in release builds.
 */
void DrawLockContext::genMessage(hostio::Context*) {}

}  // namespace sead
