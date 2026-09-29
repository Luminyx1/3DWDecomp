#include "thread/seadThreadUtil.h"
#include "basis/seadRawPrint.h"

namespace sead
{
/**
 * sead priorities are the platform's (0 to 31).
 * @param priority sead thread priority
 * @return the platform priority
 */
s32 ThreadUtil::ConvertPrioritySeadToPlatform(s32 priority)
{
    SEAD_ASSERT(priority >= 0);
    SEAD_ASSERT(priority < 32);
    return priority;
}

/**
 * sead priorities are the platform's (0 to 31).
 * @param priority platform thread priority
 * @return the sead priority
 */
s32 ThreadUtil::ConvertPriorityPlatformToSead(s32 priority)
{
    SEAD_ASSERT(priority >= 0);
    SEAD_ASSERT(priority < 32);
    return priority;
}

/**
 * @return the current stack pointer
 */
uintptr_t ThreadUtil::GetCurrentStackPointer()
{
    volatile uintptr_t stackPointer = 0;
    uintptr_t sp;
    asm volatile("mov %0, sp" : "=r"(sp));
    stackPointer = sp;
    return stackPointer;
}
}  // namespace sead
