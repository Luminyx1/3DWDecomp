#include "Library/Execute/ExecutorActorExecuteBase.hpp"
#include "Library/Execute/MultiCoreExecutorThread.hpp"
#include "Library/LiveActor/LiveActor.hpp"

#include <thread/seadDelegateThread.h>

namespace al {
/**
 * Constructs an actor movement executor.
 * @param pName Executor name.
 */
ExecutorActorMovement::ExecutorActorMovement(const char* pName)
    : ExecutorActorExecuteBase(pName) {}
/**
 * Runs the movement of all actors.
 */
void ExecutorActorMovement::execute() const {
    for (s32 i = 0; i < mActorNum; i++) {
        mActors[i]->movement();
    }
}

/**
 * Runs the paused movement of all actors.
 */
void ExecutorActorMovement::executePaused() const {
    for (s32 i = 0; i < mActorNum; i++) {
        mActors[i]->movementPaused(false);
    }
}

/**
 * Constructs an actor anim calculation executor.
 * @param pName Executor name.
 */
ExecutorActorCalcAnim::ExecutorActorCalcAnim(const char* pName)
    : ExecutorActorExecuteBase(pName) {}

/**
 * Calculates the anims of all actors.
 */
void ExecutorActorCalcAnim::execute() const {
    for (s32 i = 0; i < mActorNum; i++) {
        mActors[i]->calcAnim();
    }
}

/**
 * Constructs an actor model update executor.
 * @param pName Executor name.
 */
ExecutorActorModelUpdate::ExecutorActorModelUpdate(const char* pName)
    : ExecutorActorExecuteBase(pName) {}

/**
 * Updates the models of all actors.
 */
void ExecutorActorModelUpdate::execute() const {
    for (s32 i = 0; i < mActorNum; i++) {
        mActors[i]->modelUpdate();
    }
}

/**
 * Updates the models of all actors while paused.
 */
void ExecutorActorModelUpdate::executePaused() const {
    for (s32 i = 0; i < mActorNum; i++) {
        mActors[i]->pausedModelUpdate();
    }
}

/**
 * Constructs a combined actor movement and anim calculation executor.
 * @param pName Executor name.
 * @param pThread Thread used to calculate anims.
 */
ExecutorActorMovementCalcAnim::ExecutorActorMovementCalcAnim(const char* pName,
                                                             MultiCoreExecutorThreadBase* pThread)
    : ExecutorActorExecuteBase(pName), mThread(pThread) {}

/**
 * Runs the movement of all actors and calculates their anims, on another core for many actors.
 */
void ExecutorActorMovementCalcAnim::execute() const {
    if (mThread != nullptr && mActorNum >= 11) {
        for (s32 i = 0; i < mActorNum; i++) {
            mActors[i]->movement();
        }

        mThread->mThread->sendMessage(reinterpret_cast<s64>(mActors),
                                      sead::MessageQueue::BlockType::Blocking);
        return;
    }

    for (s32 i = 0; i < mActorNum; i++) {
        mActors[i]->movement();
        mActors[i]->calcAnim();
    }
}

/**
 * Runs the paused movement of all actors.
 */
void ExecutorActorMovementCalcAnim::executePaused() const {
    for (s32 i = 0; i < mActorNum; i++) {
        mActors[i]->movementPaused(true);
    }
}
}  // namespace al
