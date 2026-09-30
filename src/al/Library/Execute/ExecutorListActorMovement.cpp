#include "Library/Execute/ExecutorActorExecuteBase.hpp"
#include "Library/Execute/ExecutorListActorExecute.hpp"

namespace al {
/**
 * Constructs an actor movement executor list.
 * @param pListName List name.
 * @param capacity Maximum number of executors.
 * @param pGroupName Group name.
 */
ExecutorListActorMovement::ExecutorListActorMovement(const char* pListName, s32 capacity,
                                                     const char* pGroupName)
    : ExecutorListActorExecuteBase(pListName, capacity, pGroupName) {}

/**
 * Creates an actor movement executor.
 * @param pName Executor name.
 * @return The executor.
 */
ExecutorActorExecuteBase* ExecutorListActorMovement::createExecutor(const char* pName) const {
    return new ExecutorActorMovement(pName);
}

/**
 * Constructs an actor anim calculation executor list.
 * @param pListName List name.
 * @param capacity Maximum number of executors.
 * @param pGroupName Group name.
 */
ExecutorListActorCalcAnim::ExecutorListActorCalcAnim(const char* pListName, s32 capacity,
                                                     const char* pGroupName)
    : ExecutorListActorExecuteBase(pListName, capacity, pGroupName) {}

/**
 * Creates an actor anim calculation executor.
 * @param pName Executor name.
 * @return The executor.
 */
ExecutorActorExecuteBase* ExecutorListActorCalcAnim::createExecutor(const char* pName) const {
    return new ExecutorActorCalcAnim(pName);
}

/**
 * Constructs a combined actor movement and anim calculation executor list.
 * @param pListName List name.
 * @param capacity Maximum number of executors.
 * @param pGroupName Group name.
 * @param pThread Thread used to calculate anims.
 */
ExecutorListActorMovementCalcAnim::ExecutorListActorMovementCalcAnim(
    const char* pListName, s32 capacity, const char* pGroupName,
    MultiCoreExecutorThreadBase* pThread)
    : ExecutorListActorExecuteBase(pListName, capacity, pGroupName), mThread(pThread) {}

/**
 * Creates a combined actor movement and anim calculation executor.
 * @param pName Executor name.
 * @return The executor.
 */
ExecutorActorExecuteBase*
ExecutorListActorMovementCalcAnim::createExecutor(const char* pName) const {
    return new ExecutorActorMovementCalcAnim(pName, mThread);
}

/**
 * Constructs an actor model update executor list.
 * @param pListName List name.
 * @param capacity Maximum number of executors.
 * @param pGroupName Group name.
 */
ExecutorListActorModelUpdate::ExecutorListActorModelUpdate(const char* pListName, s32 capacity,
                                                           const char* pGroupName)
    : ExecutorListActorExecuteBase(pListName, capacity, pGroupName) {}

/**
 * Creates an actor model update executor.
 * @param pName Executor name.
 * @return The executor.
 */
ExecutorActorExecuteBase* ExecutorListActorModelUpdate::createExecutor(const char* pName) const {
    return new ExecutorActorModelUpdate(pName);
}
}  // namespace al
