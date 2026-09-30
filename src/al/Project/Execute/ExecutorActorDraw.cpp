#include "Library/Execute/ExecutorActorExecuteBase.hpp"
#include "Library/LiveActor/LiveActor.hpp"

namespace al {
/**
 * Constructs an actor draw executor.
 * @param pName Executor name.
 */
ExecutorActorDraw::ExecutorActorDraw(const char* pName) : ExecutorActorExecuteBase(pName) {}

/**
 * Draws all actors.
 */
void ExecutorActorDraw::execute() const {
    for (s32 i = 0; i < mActorNum; i++) {
        mActors[i]->draw();
    }
}
}  // namespace al
