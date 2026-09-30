#include "Library/Execute/ExecutorActorExecuteBase.hpp"
#include "Library/Execute/ExecutorListActorExecute.hpp"

namespace al {
/**
 * Constructs an actor draw executor list.
 * @param pListName List name.
 * @param capacity Maximum number of executors.
 * @param pGroupName Group name.
 */
ExecutorListActorDraw::ExecutorListActorDraw(const char* pListName, s32 capacity,
                                             const char* pGroupName)
    : ExecutorListActorExecuteBase(pListName, capacity, pGroupName) {}

/**
 * Creates an actor draw executor.
 * @param pName Executor name.
 * @return The executor.
 */
ExecutorActorExecuteBase* ExecutorListActorDraw::createExecutor(const char* pName) const {
    return new ExecutorActorDraw(pName);
}
}  // namespace al
