#include "Library/Execute/ExecutorActorExecuteBase.hpp"

#include "Library/Execute/ActorExecuteInfo.hpp"
#include "Library/LiveActor/LiveActor.hpp"

namespace al {
/**
 * Constructs an empty actor executor.
 * @param pName Executor name.
 */
ExecutorActorExecuteBase::ExecutorActorExecuteBase(const char* pName) : mName(pName) {}

/**
 * Reserves a slot for an actor and adds this executor to the actor's updaters.
 * @param pActor The actor.
 */
void ExecutorActorExecuteBase::registerActor(LiveActor* pActor) {
    mActorNumMax++;
    pActor->mActorExecuteInfo->addUpdater(this);
}

/**
 * Allocates the actor table.
 */
void ExecutorActorExecuteBase::createExecutorTable() {
    if (mActorNumMax <= 0) {
        return;
    }

    mActors = new LiveActor*[mActorNumMax + 1];
    for (s32 i = 0; i <= mActorNumMax; i++) {
        mActors[i] = nullptr;
    }
}

/**
 * Adds an actor if it isn't executed yet.
 * @param pActor The actor.
 */
void ExecutorActorExecuteBase::addActor(LiveActor* pActor) {
    for (s32 i = 0; i < mActorNum; i++) {
        if (mActors[i] == pActor) {
            return;
        }
    }

    mActors[mActorNum] = pActor;
    mActorNum++;
}

/**
 * Removes an actor, moving the last actor into its slot.
 * @param pActor The actor.
 */
void ExecutorActorExecuteBase::removeActor(LiveActor* pActor) {
    for (s32 i = 0; i < mActorNum; i++) {
        if (mActors[i] == pActor) {
            if (mActorNum >= 2) {
                mActors[i] = mActors[mActorNum - 1];
                mActors[mActorNum - 1] = nullptr;
            }

            mActorNum--;
            return;
        }
    }
}

/**
 * Does nothing while paused.
 */
void ExecutorActorExecuteBase::executePaused() const {}
}  // namespace al
