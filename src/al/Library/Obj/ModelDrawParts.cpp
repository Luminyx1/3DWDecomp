#include "Library/Obj/ModelDrawParts.hpp"

#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/Model/ModelKeeper.hpp"
#include "Library/Model/alModelCafe.hpp"

namespace al {
/**
 * Constructs parts drawing the model of a parent with another executor.
 * @param pName actor name
 * @param pParent parent actor
 * @param rInfo actor init info
 * @param pExecutorDrawName draw executor list name
 */
ModelDrawParts::ModelDrawParts(const char* pName, const LiveActor* pParent,
                               const ActorInitInfo& rInfo, const char* pExecutorDrawName)
    : LiveActor(pName), mParent(pParent) {
    initActorSceneInfo(this, rInfo);
    initPoseKeeper(mParent->getPoseKeeper());
    ModelKeeper* modelKeeper = new ModelKeeper();
    alModelCafe* model = alModelCafe::createFromOtherModel(pParent->getModelKeeper()->getModelCafe());
    modelKeeper->setModel(pParent->getModelKeeper()->getModelName(), model);
    initModelKeeper(modelKeeper);
    initActorClipping(this, rInfo);
    invalidateClipping(this);
    initExecutorDraw(this, rInfo, pExecutorDrawName);
    makeActorAppeared();
}
}  // namespace al
