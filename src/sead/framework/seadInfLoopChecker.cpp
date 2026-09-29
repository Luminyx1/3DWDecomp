#include <framework/seadInfLoopChecker.h>

namespace sead
{
SEAD_TASK_SINGLETON_IMPL(InfLoopChecker)

/**
 * Constructs the infinite loop checker task.
 * @param rArg construction argument supplied by the task manager
 */
InfLoopChecker::InfLoopChecker(const TaskConstructArg& rArg)
    : CalculateTask(rArg, "sead::InfLoopChecker")
{
}

/**
 * Destroys the infinite loop checker task.
 */
InfLoopChecker::~InfLoopChecker() = default;

/**
 * Counts one more pass without a frame, reporting an infinite loop once the threshold is reached.
 */
void InfLoopChecker::countUp()
{
    if (!mEnabled || mSkipCounter != 0)
    {
        return;
    }

    if (mLoopCount < mLoopThreshold)
    {
        mLoopCount++;
    }
    else
    {
        onInfLoop_();
    }
}

/**
 * Notifies every listener that an infinite loop was detected.
 */
void InfLoopChecker::onInfLoop_()
{
    InfLoopParam param;
    mEvent.emit(param);
}

/**
 * Prepares the task by shrinking its heaps.
 */
void InfLoopChecker::prepare()
{
    adjustHeapAll();
}

/**
 * Resets the loop counter once per frame.
 */
void InfLoopChecker::calc()
{
    mLoopCount = 0;
}

}  // namespace sead
