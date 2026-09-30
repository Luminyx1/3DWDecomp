#include "Project/Clipping/ClippingDirector.hpp"

#include "Library/Actor/ActorInitInfo.hpp"
#include "Library/Clipping/ClippingActorInfo.hpp"
#include "Library/Clipping/ClippingGroupHolder.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Project/Clipping/ViewInfoCtrl.hpp"

namespace al {
/**
 * Creates the clipping director.
 * @param pExecuteDirector execute director
 * @param maxActors maximum number of clipped actors
 * @param pAreaObjDirector area director
 * @param pPlayerHolder player holder
 * @param pSceneCameraInfo scene camera info
 * @param pCameraDirector camera director
 */
ClippingDirector::ClippingDirector(ExecuteDirector* pExecuteDirector, s32 maxActors,
                                   const AreaObjDirector* pAreaObjDirector,
                                   const PlayerHolder* pPlayerHolder,
                                   SceneCameraInfo* pSceneCameraInfo,
                                   CameraDirector_RS* pCameraDirector)
    : ClippingDirectorBase(pExecuteDirector, pAreaObjDirector, pPlayerHolder, pSceneCameraInfo,
                           pCameraDirector) {
    mClippingActorHolder = new ClippingActorHolder(maxActors);
    mClippingGroupHolder = new ClippingGroupHolder();
    mViewInfoCtrl = new ViewInfoCtrl(pAreaObjDirector, pPlayerHolder);
}

/**
 * Finishes initialization.
 */
void ClippingDirector::endInit() {
    ClippingDirectorBase::endInit();
    mViewInfoCtrl->endInit();
}

/**
 * Registers an actor for clipping.
 * @param pActor actor to register
 * @param rInfo actor init info
 */
void ClippingDirector::registerActor(LiveActor* pActor, const ActorInitInfo& rInfo) {
    ClippingActorInfo* info = mClippingActorHolder->registerActor(pActor);
    PlacementId* placementId = rInfo.mPlacementId;
    info->mPlacementId = placementId;
    mViewInfoCtrl->initActorInfo(info, placementId);
}

/**
 * Registers an actor to a host actor. Unused.
 * @param pActor actor to register
 * @param pHost host actor
 */
void ClippingDirector::registerActorToHost(LiveActor* pActor, const LiveActor* pHost) {}

/**
 * Adds an actor to group clipping if its placement enables it.
 * @param pActor actor to add
 * @param rInfo actor init info
 * @param num maximum number of actors of the group
 */
void ClippingDirector::addToGroupClipping(LiveActor* pActor, const ActorInitInfo& rInfo,
                                          s32 num) {
    if (!alPlacementFunction::isEnableGroupClipping(rInfo)) {
        return;
    }

    ClippingActorInfo* info = mClippingActorHolder->initGroupClipping(pActor, rInfo);
    mClippingGroupHolder->createAndAdd(info, rInfo, num);
}

/**
 * Adds an actor to the clipping targets.
 * @param pActor actor to add
 */
void ClippingDirector::addToClipping(LiveActor* pActor) {
    mClippingActorHolder->addToClippingTarget(pActor);
}

/**
 * Removes an actor from the clipping targets.
 * @param pActor actor to remove
 */
void ClippingDirector::removeFromClipping(LiveActor* pActor) {
    mClippingActorHolder->removeFromClippingTarget(pActor);
}

/**
 * Sets the far clip level of an actor.
 * @param pActor actor
 * @param level far clip level
 */
void ClippingDirector::setActorFarClipLevel(LiveActor* pActor, s32 level) {
    mClippingActorHolder->setFarClipLevel(pActor, level);
}

/**
 * Gets the clipping radius of an actor.
 * @param pActor actor
 * @return the clipping radius
 */
f32 ClippingDirector::getActorClippingRadius(const LiveActor* pActor) {
    return mClippingActorHolder->getClippingRadius(pActor);
}

/**
 * Invalidates the clipping of an actor.
 * @param pActor actor
 */
void ClippingDirector::invalidateActorClipping(LiveActor* pActor) {
    mClippingActorHolder->invalidateClipping(pActor);
}

/**
 * Validates the clipping of an actor.
 * @param pActor actor
 */
void ClippingDirector::validateActorClipping(LiveActor* pActor) {
    mClippingActorHolder->validateClipping(pActor);
}

/**
 * Sets the clipping of an actor to a sphere.
 * @param pActor actor
 * @param radius sphere radius
 * @param pOffset sphere center, or nullptr
 */
void ClippingDirector::setActorClippingInfo(LiveActor* pActor, f32 radius,
                                            const sead::Vector3f* pOffset) {
    mClippingActorHolder->setTypeToSphere(pActor, radius, pOffset);
}

/**
 * Gets the clipping center of an actor.
 * @param pActor actor
 * @return the clipping center
 */
const sead::Vector3f& ClippingDirector::getActorClippingCenterPos(const LiveActor* pActor) {
    return mClippingActorHolder->getClippingCenterPos(pActor);
}

/**
 * Sets the near clip distance of an actor.
 * @param pActor actor
 * @param distance near clip distance
 */
void ClippingDirector::setActorNearClipDistance(LiveActor* pActor, f32 distance) {
    mClippingActorHolder->setNearClipDistance(pActor, distance);
}

/**
 * Sets the near and far clip distances of an actor. Unused.
 * @param pActor actor
 * @param near near clip distance
 * @param far far clip distance
 */
void ClippingDirector::setActorNearFarClipDistance(LiveActor* pActor, f32 near, f32 far) {}

/**
 * Finds the clipping info of an actor.
 * @param pActor actor
 * @return the clipping info, or nullptr
 */
void* ClippingDirector::findActorInfo(const LiveActor* pActor) const {
    return mClippingActorHolder->find(pActor);
}

/**
 * Updates the view infos, actor clipping and group clipping.
 */
void ClippingDirector::execute() {
    ClippingDirectorBase::execute();
    mViewInfoCtrl->update();
    mClippingActorHolder->update(mClippingJudge);
    mClippingGroupHolder->update(mClippingJudge);
}
}  // namespace al
