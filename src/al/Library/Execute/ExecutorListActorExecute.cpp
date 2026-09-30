#include "Library/Execute/ExecutorListActorExecute.hpp"

#include "Library/Execute/ExecutorActorExecuteBase.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
/**
 * Constructs an actor executor list.
 * @param pListName List name.
 * @param capacity Maximum number of executors.
 * @param pGroupName Group name.
 */
ExecutorListActorExecuteBase::ExecutorListActorExecuteBase(const char* pListName, s32 capacity,
                                                           const char* pGroupName)
    : ExecutorListBase(pListName, pGroupName), mExecutorNumMax(capacity) {
    mExecutors = new ExecutorActorExecuteBase*[capacity];

    for (s32 i = 0; i < mExecutorNumMax; i++) {
        mExecutors[i] = nullptr;
    }
}

/**
 * Registers an actor to the executor matching its name, creating it if needed.
 * @param pActor The actor.
 */
void ExecutorListActorExecuteBase::registerActor(LiveActor* pActor) {
    for (s32 i = 0; i < mExecutorNum; i++) {
        ExecutorActorExecuteBase* executor = mExecutors[i];

        if (isEqualString(executor->mName, pActor->getName())) {
            executor->registerActor(pActor);
            return;
        }
    }

    ExecutorActorExecuteBase* executor = createExecutor(pActor->getName());
    executor->registerActor(pActor);
    mExecutors[mExecutorNum] = executor;
    mExecutorNum++;
}

/**
 * Allocates the actor tables of all executors.
 */
void ExecutorListActorExecuteBase::createList() {
    for (s32 i = 0; i < mExecutorNum; i++) {
        mExecutors[i]->createExecutorTable();
    }
}

/**
 * Executes all executors.
 */
void ExecutorListActorExecuteBase::executeList() const {
    for (s32 i = 0; i < mExecutorNum; i++) {
        mExecutors[i]->execute();
    }
}

/**
 * Executes all executors while paused.
 */
void ExecutorListActorExecuteBase::executeListPaused() const {
    for (s32 i = 0; i < mExecutorNum; i++) {
        mExecutors[i]->executePaused();
    }
}
}  // namespace al
