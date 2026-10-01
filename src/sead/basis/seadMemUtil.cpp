#include <prim/seadMemUtil.h>
#include <thread/seadThread.h>

namespace sead {
/**
 * Checks whether an address belongs to a registered thread's stack.
 * @param pAddress Address to test; the stack's upper boundary is excluded.
 * @return Whether the main thread or another managed thread owns the stack address.
 */
bool MemUtil::isStack(const void* pAddress)
{
    if (ThreadMgr::instance() == nullptr)
        return false;
    const auto contains = [](const void* address, const void* bottom, s32 size) {
        const uintptr_t start = reinterpret_cast<uintptr_t>(bottom);
        const uintptr_t end = start + size;
        return (start <= reinterpret_cast<uintptr_t>(address)) & (reinterpret_cast<uintptr_t>(address) < end);
    };

    Thread* main = ThreadMgr::instance()->getMainThread();

    if (main != nullptr && contains(pAddress, main->mThreadInner->_stack, main->mStackSize))
        return true;
    {
        ScopedLock<CriticalSection> lock(ThreadMgr::instance()->getListCS());

        for (Thread* thread : ThreadMgr::instance()->mList) {
            if (contains(pAddress, thread->mThreadInner->_stack, thread->mStackSize))
                return true;
        }
    }

    return false;
}
}  // namespace sead
