#include "Project/Clipping/ClippingAreaDirector.hpp"

#include "Library/Clipping/ClippingJudge.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Project/Clipping/ClippingAreaActorInfo.hpp"
#include "Project/Clipping/ClippingAreaActorViewHolder.hpp"
#include "Project/Clipping/ClippingFarAreaObserver.hpp"

namespace al {
/**
 * Creates the area based clipping director.
 * @param pExecuteDirector execute director
 * @param maxActors maximum number of clipped actors
 * @param pAreaObjDirector area director
 * @param pPlayerHolder player holder
 * @param pSceneCameraInfo scene camera info
 * @param pCameraDirector camera director
 * @param pThread thread used for asynchronous clipping
 */
ClippingAreaDirector::ClippingAreaDirector(ExecuteDirector* pExecuteDirector, s32 maxActors,
                                           const AreaObjDirector* pAreaObjDirector,
                                           const PlayerHolder* pPlayerHolder,
                                           SceneCameraInfo* pSceneCameraInfo,
                                           CameraDirector_RS* pCameraDirector,
                                           MultiCoreQueueThread* pThread)
    : ClippingDirectorBase(pExecuteDirector, pAreaObjDirector, pPlayerHolder, pSceneCameraInfo,
                           pCameraDirector),
      mPlayerHolder(pPlayerHolder) {
    mFarAreaObserver->setDefaultFarClipDistance(200000.0f);
    mViewHolder = new ClippingAreaActorViewHolder(maxActors, pThread, pAreaObjDirector);
}

/**
 * Destroys the clipping director.
 */
ClippingAreaDirector::~ClippingAreaDirector() {
    delete mViewHolder;
}

/**
 * Finishes initialization.
 */
void ClippingAreaDirector::endInit() {
    ClippingDirectorBase::endInit();
    mViewHolder->endInit();
}

/**
 * Resets the clipping distances of all actors.
 */
void ClippingAreaDirector::resetClippingDistanceStates() {
    mViewHolder->resetClippingDistanceStates();
}

/**
 * Sets whether the clipping uses expanded distances.
 * @param isExpanded whether to expand the clipping distances
 */
void ClippingAreaDirector::setExpandedClippingMode(bool isExpanded) {
    mViewHolder->setExpandedClippingMode(isExpanded);
}

/**
 * Registers an actor for clipping.
 * @param pActor actor to register
 * @param rInfo actor init info
 */
void ClippingAreaDirector::registerActor(LiveActor* pActor, const ActorInitInfo& rInfo) {
    mViewHolder->registerActor(pActor, rInfo);
}

/**
 * Recreates the clipping of an actor.
 * @param pActor actor
 * @param rInfo actor init info
 */
void ClippingAreaDirector::recreateClipping(LiveActor* pActor, const ActorInitInfo& rInfo) {
    mViewHolder->recreateClipping(pActor, rInfo);
}

/**
 * Registers an actor to a host actor.
 * @param pActor actor to register
 * @param pHost host actor
 */
void ClippingAreaDirector::registerActorToHost(LiveActor* pActor, const LiveActor* pHost) {
    mViewHolder->registerActorToHost(pActor, pHost);
}

/**
 * Adds an actor to group clipping. Unused.
 * @param pActor actor to add
 * @param rInfo actor init info
 * @param num maximum number of actors of the group
 */
void ClippingAreaDirector::addToGroupClipping(LiveActor* pActor, const ActorInitInfo& rInfo,
                                              s32 num) {}

/**
 * Adds an actor to the clipping system.
 * @param pActor actor to add
 */
void ClippingAreaDirector::addToClipping(LiveActor* pActor) {
    mViewHolder->moveToClippingSystem(pActor);
}

/**
 * Removes an actor from the clipping system.
 * @param pActor actor to remove
 */
void ClippingAreaDirector::removeFromClipping(LiveActor* pActor) {
    mViewHolder->removeFromClippingSystem(pActor);
}

/**
 * Moves an actor to the clipping group of another actor.
 * @param pActor actor to move
 * @param pGroupActor actor owning the group
 */
void ClippingAreaDirector::moveToClippingGroup(LiveActor* pActor, LiveActor* pGroupActor) {
    mViewHolder->moveToClippingGroup(pActor, pGroupActor);
}

/**
 * Sets the far clip level of an actor. Unused.
 * @param pActor actor
 * @param level far clip level
 */
void ClippingAreaDirector::setActorFarClipLevel(LiveActor* pActor, s32 level) {}

/**
 * Gets the clipping radius of an actor.
 * @param pActor actor
 * @return the clipping radius
 */
f32 ClippingAreaDirector::getActorClippingRadius(const LiveActor* pActor) {
    return mViewHolder->getClippingRadius(pActor);
}

/**
 * Invalidates the clipping of an actor.
 * @param pActor actor
 */
void ClippingAreaDirector::invalidateActorClipping(LiveActor* pActor) {
    mViewHolder->invalidateClipping(pActor);
}

/**
 * Validates the clipping of an actor.
 * @param pActor actor
 */
void ClippingAreaDirector::validateActorClipping(LiveActor* pActor) {
    mViewHolder->validateClipping(pActor);
}

/**
 * Sets the clipping radius of an actor.
 * @param pActor actor
 * @param radius clipping radius
 * @param pOffset clipping offset, unused
 */
void ClippingAreaDirector::setActorClippingInfo(LiveActor* pActor, f32 radius,
                                                const sead::Vector3f* pOffset) {
    mViewHolder->setClippingRadius(pActor, radius);
}

/**
 * Sets the clipping offset of an actor.
 * @param pActor actor
 * @param rOffset clipping offset
 */
void ClippingAreaDirector::setActorClippingOffset(LiveActor* pActor,
                                                  const sead::Vector3f& rOffset) {
    mViewHolder->setClippingOffset(pActor, rOffset);
}

/**
 * Gets the clipping center of an actor. Unsupported.
 * @param pActor actor
 * @return the zero vector
 */
const sead::Vector3f& ClippingAreaDirector::getActorClippingCenterPos(const LiveActor* pActor) {
    return sead::Vector3f::zero;
}

/**
 * Sets the near clip distance of an actor. Unused.
 * @param pActor actor
 * @param distance near clip distance
 */
void ClippingAreaDirector::setActorNearClipDistance(LiveActor* pActor, f32 distance) {}

/**
 * Sets the near and far clip distances of an actor.
 * @param pActor actor
 * @param near near clip distance
 * @param far far clip distance
 */
void ClippingAreaDirector::setActorNearFarClipDistance(LiveActor* pActor, f32 near, f32 far) {
    mViewHolder->updateNearFarClipping(pActor, near, far);
}

/**
 * Sets the shadow clipping distance of an actor.
 * @param pActor actor
 * @param distance shadow clipping distance
 */
void ClippingAreaDirector::setShadowClippingDistance(LiveActor* pActor, f32 distance) {
    mViewHolder->setShadowClippingDistance(pActor, distance);
}

/**
 * Sets the draw clipping radius of an actor.
 * @param pActor actor
 * @param radius draw clipping radius
 */
void ClippingAreaDirector::setDrawClippingRadius(LiveActor* pActor, f32 radius) {
    mViewHolder->setDrawClippingRadius(pActor, radius);
}

/**
 * Sets whether the collision of an actor is never clipped.
 * @param pActor actor
 * @param isNoClip whether the collision is never clipped
 */
void ClippingAreaDirector::setNoCollisionClip(LiveActor* pActor, bool isNoClip) {
    mViewHolder->setNoCollisionClip(pActor, isNoClip);
}

/**
 * Finds the clipping info of an actor. Unsupported.
 * @param pActor actor
 * @return nullptr
 */
void* ClippingAreaDirector::findActorInfo(const LiveActor* pActor) const {
    return nullptr;
}

/**
 * Sets whether the collision clipping is disabled.
 * @param isDisabled whether the collision clipping is disabled
 */
void ClippingAreaDirector::setCollisionClippingDisabled(bool isDisabled) {
    mViewHolder->setCollisionClippingDisabled(isDisabled);
}

/**
 * Disables the force clip areas.
 */
void ClippingAreaDirector::disableForceClipAreas() {
    mViewHolder->disableForceClipAreas();
}

/**
 * Updates the clipping.
 */
void ClippingAreaDirector::execute() {
    ClippingDirectorBase::execute();
    mClippingJudge->setPlayerPos(mPlayerHolder);
    mViewHolder->update(mClippingJudge);
}

/**
 * Requests an asynchronous clipping update.
 */
void ClippingAreaDirector::executeRequestAsyncUpdate() {
    mViewHolder->execute();
}

/**
 * Waits for the pending asynchronous clipping update.
 */
void ClippingAreaDirector::waitPendingClippingRequest() {
    mViewHolder->waitIfPendingAsyncClipping();
    if (_18 && *_18 == 1) {
        mViewHolder->resetClippingDistanceStates();
    }
}

/**
 * Sets whether the LOD of an actor is disabled.
 * @param pActor actor
 * @param isDisabled whether the LOD is disabled
 */
void ClippingAreaDirector::setLODDisabled(LiveActor* pActor, bool isDisabled) {
    ClippingAreaActorInfoNode* node = pActor->mClippingInfoNode;
    if (!node) {
        return;
    }

    ClippingAreaActorInfo* info = node->mInfo;
    if (!info) {
        info->mIsLODDisabled = isDisabled;
    }

    setLodDisabled(pActor, isDisabled);
}
}  // namespace al
