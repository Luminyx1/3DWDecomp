#include "Library/Obj/SilhouetteModel.hpp"

#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/Model/ModelKeeper.hpp"
#include "Library/Model/alModelCafe.hpp"

namespace al {
/**
 * Constructs a silhouette model sharing the pose and model of a parent.
 * @param pParent parent actor
 * @param rInfo actor init info
 * @param pExecutorDrawName draw executor list name
 */
SilhouetteModel::SilhouetteModel(LiveActor* pParent, const ActorInitInfo& rInfo,
                                 const char* pExecutorDrawName)
    : LiveActor("シルエットモデル"), mParent(pParent) {
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

/**
 * Does nothing, the parent moves the model.
 */
void SilhouetteModel::movement() {}
}  // namespace al
