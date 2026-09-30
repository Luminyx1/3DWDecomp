#include "Library/Obj/DepthShadowModel.hpp"

#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/Model/ModelKeeper.hpp"
#include "Library/Model/alModelCafe.hpp"

namespace al {
/**
 * Constructs a depth shadow model sharing the pose and model of a parent.
 * @param pParent parent actor
 * @param rInfo actor init info
 * @param pExecutorDrawName draw executor list name
 */
DepthShadowModel::DepthShadowModel(LiveActor* pParent, const ActorInitInfo& rInfo,
                                   const char* pExecutorDrawName)
    : LiveActor("デプスシャドウモデル"), mParent(pParent) {
    initActorSceneInfo(this, rInfo);
    initPoseKeeper(mParent->mActorPoseKeeper);
    ModelKeeper* modelKeeper = new ModelKeeper();
    alModelCafe* model = alModelCafe::createFromOtherModel(pParent->mModelKeeper->getModelCafe());
    modelKeeper->_18 = pParent->mModelKeeper->_18;
    modelKeeper->setModel(pParent->mModelKeeper->mModelName, model);
    initModelKeeper(modelKeeper);
    initExecutorDraw(this, rInfo, pExecutorDrawName);
    makeActorAppeared();
}
}  // namespace al
