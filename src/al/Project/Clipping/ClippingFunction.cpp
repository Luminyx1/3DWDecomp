#include "Project/Clipping/ClippingFunction.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorSceneInfo.hpp"
#include "Project/Clipping/ClippingDirectorBase.hpp"

namespace alClippingFunction {
    /**
     * @brief Adds an actor to the scene's clipping targets.
     * @param pActor The actor to start clipping.
     */
    void addToClippingTarget(al::LiveActor* pActor) {
        pActor->getSceneInfo()->clippingDirectorBase->addToClipping(pActor);
    }

    /**
     * @brief Removes an actor from the scene's clipping targets.
     * @param pActor The actor to stop clipping.
     */
    void removeFromClippingTarget(al::LiveActor* pActor) {
        pActor->getSceneInfo()->clippingDirectorBase->removeFromClipping(pActor);
    }
};
