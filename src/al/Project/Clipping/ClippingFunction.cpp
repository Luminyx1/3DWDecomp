#include "Project/Clipping/ClippingFunction.hpp"

#include "Library/Clipping/ClippingDirectorBase.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorSceneInfo.hpp"

namespace alClippingFunction {
/**
 * Makes an actor a clipping target.
 * @param pActor actor to add
 */
void addToClippingTarget(al::LiveActor* pActor) {
    pActor->getSceneInfo()->clippingDirectorBase->addToClipping(pActor);
}

/**
 * Stops an actor from being a clipping target.
 * @param pActor actor to remove
 */
void removeFromClippingTarget(al::LiveActor* pActor) {
    pActor->getSceneInfo()->clippingDirectorBase->removeFromClipping(pActor);
}
}  // namespace alClippingFunction
