#include "Library/LiveActor/Util/ActorClippingUtil.hpp"

#include <math/seadMathCalcCommon.h>

#include "Library/Actor/ActorInitInfo.hpp"
#include "Library/Audio/System/AudioKeeper.hpp"
#include "Library/Clipping/ClippingAreaActorInfo.hpp"
#include "Library/Clipping/ClippingDirectorBase.hpp"
#include "Library/Execute/ActorSystemFunction.hpp"
#include "Library/HitSensor/HitSensorKeeper.hpp"
#include "Library/HitSensor/SensorFunction.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/LiveActorFlag.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/LiveActor/Util/ActorSceneInfo.hpp"
#include "Library/Obj/PlacementClippingExpander.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementInfo.hpp"
#include "Library/Shadow/Common/ShadowUtil.hpp"
#include "Project/Collision/CollisionUtil.hpp"
#include "Project/Effect/Core/EffectKeeper.hpp"

namespace al {
/**
 * Registers an actor to the clipping system.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 */
void initActorClipping(LiveActor* pActor, const ActorInitInfo& rInfo) {
    pActor->getSceneInfo()->clippingDirectorBase->registerActor(pActor, rInfo);
}

/**
 * Recreates the clipping of an actor.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 */
void recreateClipping(LiveActor* pActor, const ActorInitInfo& rInfo) {
    pActor->getSceneInfo()->clippingDirectorBase->recreateClipping(pActor, rInfo);
}

/**
 * Makes an actor use the clipping of a host actor.
 * @param pActor The actor.
 * @param pHost The host actor.
 */
void addToHostActorClipping(LiveActor* pActor, const LiveActor* pHost) {
    pHost->getSceneInfo()->clippingDirectorBase->registerActorToHost(pActor, pHost);
}

/**
 * Adds an actor to a clipping group.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param num The group size.
 */
void initGroupClipping(LiveActor* pActor, const ActorInitInfo& rInfo, s32 num) {
    pActor->getSceneInfo()->clippingDirectorBase->addToGroupClipping(pActor, rInfo, num);
}

/**
 * Resets the clipping distance states of the scene.
 * @param pActor The actor.
 */
void resetClippingDistanceStates(LiveActor* pActor) {
    pActor->getSceneInfo()->clippingDirectorBase->resetClippingDistanceStates();
}

/**
 * Sets whether the scene uses the expanded clipping mode.
 * @param pActor The actor.
 * @param isExpanded Whether to expand the clipping.
 */
void setExpandedClippingMode(LiveActor* pActor, bool isExpanded) {
    pActor->getSceneInfo()->clippingDirectorBase->setExpandedClippingMode(isExpanded);
}

/**
 * Checks whether an actor is in the expanded clipping mode.
 * @param pActor The actor.
 * @return Whether the clipping is expanded.
 */
bool isExpandedClippingMode(const LiveActor* pActor) {
    ClippingAreaActorInfoNode* node = pActor->mClippingInfoNode;

    if (!node) {
        return false;
    }

    ClippingAreaActorInfo* info = node->mInfo;

    if (!info) {
        return false;
    }

    return info->isExpandedClippingMode();
}

/**
 * Moves an actor into the clipping group of another actor.
 * @param pActor The actor.
 * @param pGroupActor The group actor.
 */
void moveActorToClippingGroup(LiveActor* pActor, LiveActor* pGroupActor) {
    pActor->getSceneInfo()->clippingDirectorBase->moveToClippingGroup(pActor, pGroupActor);
}

/**
 * Sets whether collision clipping is disabled.
 * @param pActor The actor.
 * @param isDisabled Whether to disable collision clipping.
 */
void setCollisionClippingDisabled(LiveActor* pActor, bool isDisabled) {
    pActor->getSceneInfo()->clippingDirectorBase->setCollisionClippingDisabled(isDisabled);
}

/**
 * Sets the far clip level of an actor to the maximum.
 * @param pActor The actor.
 */
void setClippingFarMax(LiveActor* pActor) {
    pActor->getSceneInfo()->clippingDirectorBase->setActorFarClipLevel(pActor, 0);
}

/**
 * Gets the clipping radius of an actor.
 * @param pActor The actor.
 * @return The clipping radius.
 */
f32 getClippingRadius(const LiveActor* pActor) {
    return pActor->getSceneInfo()->clippingDirectorBase->getActorClippingRadius(pActor);
}

/**
 * Sets whether an actor ignores collision clipping.
 * @param pActor The actor.
 * @param isNoClip Whether to ignore collision clipping.
 */
void setNoCollisionClip(LiveActor* pActor, bool isNoClip) {
    pActor->getSceneInfo()->clippingDirectorBase->setNoCollisionClip(pActor, isNoClip);
}

/**
 * Sets the clipping radius and offset of an actor.
 * @param pActor The actor.
 * @param radius The clipping radius.
 * @param pOffset The clipping center, or nullptr.
 */
void setClippingInfo(LiveActor* pActor, f32 radius, const sead::Vector3f* pOffset) {
    pActor->getSceneInfo()->clippingDirectorBase->setActorClippingInfo(pActor, radius, pOffset);
}

/**
 * Sets the clipping offset of an actor.
 * @param pActor The actor.
 * @param rOffset The clipping offset.
 */
void setClippingOffset(LiveActor* pActor, const sead::Vector3f& rOffset) {
    pActor->getSceneInfo()->clippingDirectorBase->setActorClippingOffset(pActor, rOffset);
}

/**
 * Sets the shadow clipping distance of an actor.
 * @param pActor The actor.
 * @param distance The shadow clipping distance.
 */
void setShadowClippingDistance(LiveActor* pActor, f32 distance) {
    pActor->getSceneInfo()->clippingDirectorBase->setShadowClippingDistance(pActor, distance);
}

/**
 * Sets the draw clipping radius of an actor.
 * @param pActor The actor.
 * @param radius The draw clipping radius.
 */
void setDrawClippingRadius(LiveActor* pActor, f32 radius) {
    pActor->getSceneInfo()->clippingDirectorBase->setDrawClippingRadius(pActor, radius);
}

/**
 * Gets the clipping center of an actor.
 * @param pActor The actor.
 * @return The clipping center.
 */
const sead::Vector3f& getClippingCenterPos(const LiveActor* pActor) {
    return pActor->getSceneInfo()->clippingDirectorBase->getActorClippingCenterPos(pActor);
}

/**
 * Sets the near clip distance of an actor.
 * @param pActor The actor.
 * @param distance The near clip distance.
 */
void setClippingNearDistance(LiveActor* pActor, f32 distance) {
    pActor->getSceneInfo()->clippingDirectorBase->setActorNearClipDistance(pActor, distance);
}

/**
 * Sets the near and far clip distances of an actor.
 * @param pActor The actor.
 * @param near The near clip distance.
 * @param far The far clip distance.
 */
void setClippingNearFarDistance(LiveActor* pActor, f32 near, f32 far) {
    pActor->getSceneInfo()->clippingDirectorBase->setActorNearFarClipDistance(pActor, near, far);
}

/**
 * Gets the clipping judge of the scene.
 * @param pActor The actor.
 * @return The clipping judge.
 */
ClippingJudge* getClippingJudge(LiveActor* pActor) {
    return pActor->getSceneInfo()->clippingDirectorBase->mClippingJudge;
}

/**
 * Disables the LOD of all actors.
 * @param pActor The actor.
 */
void disableAllLOD(LiveActor* pActor) {
    pActor->getSceneInfo();
    ClippingDirectorBase::sLODDisabled = true;
}

/**
 * Enables the LOD of all actors.
 * @param pActor The actor.
 */
void enableAllLOD(LiveActor* pActor) {
    pActor->getSceneInfo();
    ClippingDirectorBase::sLODDisabled = false;
}

/**
 * Sets whether the LOD of an actor is disabled.
 * @param pActor The actor.
 * @param isDisabled Whether to disable the LOD.
 */
void forceLodDisabled(LiveActor* pActor, bool isDisabled) {
    pActor->getSceneInfo()->clippingDirectorBase->setLODDisabled(pActor, isDisabled);
}

/**
 * Expands the clipping radius of an actor to cover its shadow.
 * @param pActor The actor.
 * @param pOffset The clipping center to update, or nullptr.
 * @param shadowLength The shadow length.
 */
void expandClippingRadiusByShadowLength(LiveActor* pActor, sead::Vector3f* pOffset, f32 shadowLength) {
    sead::Vector3f trans = getTrans(pActor);
    f32 radius = getClippingRadius(pActor);

    if (radius >= shadowLength) {
        return;
    }

    if (pOffset) {
        f32 newRadius = (radius + shadowLength) * 0.5f;
        pOffset->set(trans + getGravity(pActor) * (newRadius - radius));
        setClippingInfo(pActor, newRadius, pOffset);
    } else {
        setClippingInfo(pActor, sead::Mathf::max(radius, shadowLength), nullptr);
    }
}

/**
 * Expands the clipping of an actor down to the ground below it.
 * @param pActor The actor.
 * @param pOffset The clipping center to update.
 * @param length The search length.
 * @return Whether the clipping was expanded.
 */
bool tryExpandClippingToGround(LiveActor* pActor, sead::Vector3f* pOffset, f32 length) {
    f32 radius = getClippingRadius(pActor);
    sead::Vector3f hitPos = sead::Vector3f::zero;
    sead::Vector3f trans = getTrans(pActor);
    sead::Vector3f dir = getGravity(pActor) * length;

    if (!alCollisionUtil::getFirstPolyOnArrow(pActor, &hitPos, nullptr, trans, dir, nullptr, nullptr)) {
        return false;
    }

    f32 distance = (hitPos - trans).length();

    if (distance < radius) {
        return false;
    }

    if (distance > length) {
        return false;
    }

    f32 newRadius = (radius + distance) * 0.5f;
    sead::Vector3f offset = trans + (hitPos - trans) * ((distance - newRadius) / distance);
    pOffset->x = offset.x;
    pOffset->y = offset.y;
    pOffset->z = offset.z;
    setClippingInfo(pActor, newRadius, pOffset);
    return true;
}

/**
 * Expands the clipping radius of an actor to cover its shadow, if it has one.
 * @param pActor The actor.
 * @param pOffset The clipping center to update, or nullptr.
 * @return Whether the actor has a shadow.
 */
bool tryExpandClippingByShadowLength(LiveActor* pActor, sead::Vector3f* pOffset) {
    if (!isExistShadow(pActor)) {
        return false;
    }

    expandClippingRadiusByShadowLength(pActor, pOffset, getShadowDropLengthMax(pActor));
    return true;
}

/**
 * Expands the clipping of an actor with a linked expand object.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @return Whether an expand object is linked.
 */
bool tryExpandClippingByExpandObject(LiveActor* pActor, const ActorInitInfo& rInfo) {
    PlacementInfo linkInfo;

    if (!tryGetLinksInfo(&linkInfo, rInfo, "ClippingExpandObject")) {
        return false;
    }

    PlacementClippingExpander* expander = new PlacementClippingExpander();
    expander->init(pActor, linkInfo);
    return true;
}

/**
 * Checks whether an actor is clipped.
 * @param pActor The actor.
 * @return Whether the actor is clipped.
 */
bool isClipped(const LiveActor* pActor) {
    return pActor->mActorFlags->isClipped;
}

/**
 * Checks whether the clipping of an actor is invalid.
 * @param pActor The actor.
 * @return Whether the clipping is invalid.
 */
bool isInvalidClipping(const LiveActor* pActor) {
    return pActor->mActorFlags->isInvalidClipping;
}

/**
 * Invalidates the clipping of an actor, ending its clipped state.
 * @param pActor The actor.
 */
void invalidateClipping(LiveActor* pActor) {
    if (pActor->mActorFlags->isClipped) {
        pActor->endClipped();
    }

    if (pActor->mActorFlags->isInvalidClipping) {
        return;
    }

    pActor->mGlobalAlpha = 1.0f;
    pActor->getSceneInfo()->clippingDirectorBase->invalidateActorClipping(pActor);
}

/**
 * Validates the clipping of an actor.
 * @param pActor The actor.
 */
void validateClipping(LiveActor* pActor) {
    if (pActor->mActorFlags->isInvalidClipping) {
        pActor->getSceneInfo()->clippingDirectorBase->validateActorClipping(pActor);
    }
}

/**
 * Keeps an actor drawn while it is clipped.
 * @param pActor The actor.
 */
void onDrawClipping(LiveActor* pActor) {
    pActor->mActorFlags->isDrawClipping = true;

    if (!pActor->mActorFlags->isClipped) {
        return;
    }

    alActorSystemFunction::addToExecutorMovement(pActor);

    if (pActor->mHitSensorKeeper) {
        pActor->mHitSensorKeeper->validateBySystem();
        alSensorFunction::updateHitSensorsAll(pActor);
    }

    if (pActor->getEffectKeeper()) {
        pActor->getEffectKeeper()->onCalcAndDraw();
    }

    if (pActor->getAudioKeeper()) {
        pActor->getAudioKeeper()->startClipped();
    }
}

/**
 * Stops drawing an actor while it is clipped.
 * @param pActor The actor.
 */
void offDrawClipping(LiveActor* pActor) {
    pActor->mActorFlags->isDrawClipping = false;

    if (!pActor->mActorFlags->isClipped) {
        return;
    }

    alActorSystemFunction::removeFromExecutorMovement(pActor);

    if (pActor->mHitSensorKeeper) {
        pActor->mHitSensorKeeper->invalidateBySystem();
    }

    if (pActor->getEffectKeeper()) {
        pActor->getEffectKeeper()->offCalcAndDraw();
    }

    if (pActor->getAudioKeeper()) {
        pActor->getAudioKeeper()->endClipped();
    }
}

/**
 * Makes the clipping judge use the camera clipping position as the player position.
 * @param pActor The actor.
 */
void onUseCameraClippingPos(LiveActor* pActor) {
    pActor->getSceneInfo()->clippingDirectorBase->setClippingJudgeUsClippingPosAsPlayerPos(true);
}

/**
 * Stops the clipping judge from using the camera clipping position as the player position.
 * @param pActor The actor.
 */
void offUseCameraClippingPos(LiveActor* pActor) {
    pActor->getSceneInfo()->clippingDirectorBase->setClippingJudgeUsClippingPosAsPlayerPos(false);
}
}  // namespace al

namespace alActorFunction {
/**
 * Checks whether an actor is drawn while clipped.
 * @param pActor The actor.
 * @return Whether the actor is drawn while clipped.
 */
bool isDrawClipping(const al::LiveActor* pActor) {
    return pActor->mActorFlags->isDrawClipping;
}
}  // namespace alActorFunction
