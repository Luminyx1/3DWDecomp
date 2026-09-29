#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/Execute/ActorSystemFunction.hpp"
#include "Library/HitSensor/HitSensorKeeper.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/LiveActorFlag.hpp"
#include "Library/LiveActor/SensorFunction.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/LiveActor/Util/ActorSceneInfo.hpp"
#include "Library/Obj/PlacementClippingExpander.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementInfo.hpp"
#include "Library/Shadow/Common/ShadowUtil.hpp"
#include "Project/Audio/AudioKeeper.hpp"
#include "Project/Clipping/ClippingAreaActorInfo.hpp"
#include "Project/Clipping/ClippingDirectorBase.hpp"
#include "Project/Collision/CollisionUtil.hpp"
#include "Project/Effect/Core/EffectKeeper.hpp"

#include <math/seadMathCalcCommon.h>

namespace al {
    /**
     * @brief Registers an actor with the scene's clipping director.
     * @param pActor The actor to register.
     * @param rInfo The actor's init info (placement and clipping settings).
     */
    void initActorClipping(LiveActor* pActor, const ActorInitInfo& rInfo) {
        pActor->getSceneInfo()->clippingDirectorBase->registerActor(pActor, rInfo);
    }

    /**
     * @brief Recreates an actor's clipping from its init info.
     * @param pActor The actor to update.
     * @param rInfo The actor's init info.
     */
    void recreateClipping(LiveActor* pActor, const ActorInitInfo& rInfo) {
        pActor->getSceneInfo()->clippingDirectorBase->recreateClipping(pActor, rInfo);
    }

    /**
     * @brief Makes an actor share the clipping of a host actor.
     * @param pActor The actor to register.
     * @param pHost The actor whose clipping to follow.
     */
    void addToHostActorClipping(LiveActor* pActor, const LiveActor* pHost) {
        pHost->getSceneInfo()->clippingDirectorBase->registerActorToHost(pActor, pHost);
    }

    /**
     * @brief Adds an actor to a clipping group, so the group is clipped together.
     * @param pActor The actor to add.
     * @param rInfo The actor's init info.
     * @param groupIdx Passed on to ClippingDirectorBase::addToGroupClipping.
     */
    void initGroupClipping(LiveActor* pActor, const ActorInitInfo& rInfo, s32 groupIdx) {
        pActor->getSceneInfo()->clippingDirectorBase->addToGroupClipping(pActor, rInfo, groupIdx);
    }

    /**
     * @brief Resets the clipping distance states of the whole scene.
     * @param pActor Any actor in the scene.
     */
    void resetClippingDistanceStates(LiveActor* pActor) {
        pActor->getSceneInfo()->clippingDirectorBase->resetClippingDistanceStates();
    }

    /**
     * @brief Turns the scene's expanded clipping mode on or off.
     * @param pActor Any actor in the scene.
     * @param isExpanded Whether expanded clipping should be used.
     */
    void setExpandedClippingMode(LiveActor* pActor, bool isExpanded) {
        pActor->getSceneInfo()->clippingDirectorBase->setExpandedClippingMode(isExpanded);
    }

    /**
     * @brief Checks whether the clipping area an actor is in uses expanded clipping.
     * @param pActor The actor to check.
     * @return True if the actor has clipping area info in expanded mode.
     */
    bool isExpandedClippingMode(const LiveActor* pActor) {
        if (pActor->mClippingAreaActorInfoNode == nullptr) {
            return false;
        }

        ClippingAreaActorInfo* pInfo = pActor->mClippingAreaActorInfoNode->mAreaActorInfo;

        if (pInfo == nullptr) {
            return false;
        }

        return pInfo->mIsExpandedClippingMode;
    }

    /**
     * @brief Moves an actor into the clipping group of another actor.
     * @param pActor The actor to move.
     * @param pGroupActor An actor of the target group.
     */
    void moveActorToClippingGroup(LiveActor* pActor, LiveActor* pGroupActor) {
        pActor->getSceneInfo()->clippingDirectorBase->moveToClippingGroup(pActor, pGroupActor);
    }

    /**
     * @brief Turns clipping of collision on or off for the scene.
     * @param pActor Any actor in the scene.
     * @param isDisabled Whether collision clipping should be disabled.
     */
    void setCollisionClippingDisabled(LiveActor* pActor, bool isDisabled) {
        pActor->getSceneInfo()->clippingDirectorBase->setCollisionClippingDisabled(isDisabled);
    }

    /**
     * @brief Sets an actor's far clip level to the maximum distance.
     * @param pActor The actor to update.
     */
    void setClippingFarMax(LiveActor* pActor) {
        pActor->getSceneInfo()->clippingDirectorBase->setActorFarClipLevel(pActor, 0);
    }

    /**
     * @brief Gets an actor's clipping radius.
     * @param pActor The actor to check.
     * @return The radius of the actor's clipping sphere.
     */
    f32 getClippingRadius(const LiveActor* pActor) {
        return pActor->getSceneInfo()->clippingDirectorBase->getActorClippingRadius(pActor);
    }

    /**
     * @brief Sets whether an actor's collision stays active while it is clipped.
     * @param pActor The actor to update.
     * @param isNoClip Whether the collision should not be clipped.
     */
    void setNoCollisionClip(LiveActor* pActor, bool isNoClip) {
        pActor->getSceneInfo()->clippingDirectorBase->setNoCollisionClip(pActor, isNoClip);
    }

    /**
     * @brief Sets an actor's clipping sphere.
     * @param pActor The actor to update.
     * @param radius The radius of the sphere.
     * @param pCenter The center to follow, or nullptr to use the actor's position.
     */
    void setClippingInfo(LiveActor* pActor, f32 radius, const sead::Vector3f* pCenter) {
        pActor->getSceneInfo()->clippingDirectorBase->setActorClippingInfo(pActor, radius, pCenter);
    }

    /**
     * @brief Sets the offset of an actor's clipping sphere from its position.
     * @param pActor The actor to update.
     * @param rOffset The offset.
     */
    void setClippingOffset(LiveActor* pActor, const sead::Vector3f& rOffset) {
        pActor->getSceneInfo()->clippingDirectorBase->setActorClippingOffset(pActor, rOffset);
    }

    /**
     * @brief Sets the distance at which an actor's shadow is clipped.
     * @param pActor The actor to update.
     * @param distance The clipping distance of the shadow.
     */
    void setShadowClippingDistance(LiveActor* pActor, f32 distance) {
        pActor->getSceneInfo()->clippingDirectorBase->setShadowClippingDistance(pActor, distance);
    }

    /**
     * @brief Sets the radius used when clipping an actor's drawing.
     * @param pActor The actor to update.
     * @param radius The draw clipping radius.
     */
    void setDrawClippingRadius(LiveActor* pActor, f32 radius) {
        pActor->getSceneInfo()->clippingDirectorBase->setDrawClippingRadius(pActor, radius);
    }

    /**
     * @brief Gets the center of an actor's clipping sphere.
     * @param pActor The actor to check.
     * @return The center of the clipping sphere.
     */
    const sead::Vector3f& getClippingCenterPos(const LiveActor* pActor) {
        return pActor->getSceneInfo()->clippingDirectorBase->getActorClippingCenterPos(pActor);
    }

    /**
     * @brief Sets the distance under which an actor is clipped for being too close to the camera.
     * @param pActor The actor to update.
     * @param distance The near clipping distance.
     */
    void setClippingNearDistance(LiveActor* pActor, f32 distance) {
        pActor->getSceneInfo()->clippingDirectorBase->setActorNearClipDistance(pActor, distance);
    }

    /**
     * @brief Sets an actor's near and far clipping distances.
     * @param pActor The actor to update.
     * @param nearDistance The near clipping distance.
     * @param farDistance The far clipping distance.
     */
    void setClippingNearFarDistance(LiveActor* pActor, f32 nearDistance, f32 farDistance) {
        pActor->getSceneInfo()->clippingDirectorBase->setActorNearFarClipDistance(pActor, nearDistance, farDistance);
    }

    /**
     * @brief Gets the scene's clipping judge.
     * @param pActor Any actor in the scene.
     * @return The clipping judge of the scene's clipping director.
     */
    ClippingJudge* getClippingJudge(LiveActor* pActor) {
        return pActor->getSceneInfo()->clippingDirectorBase->mClippingJudge;
    }

    /**
     * @brief Disables level-of-detail switching for every actor.
     * @param pActor Any actor in the scene.
     */
    void disableAllLOD(LiveActor* pActor) {
        pActor->getSceneInfo()->clippingDirectorBase->sLODDisabled = true;
    }

    /**
     * @brief Enables level-of-detail switching for every actor.
     * @param pActor Any actor in the scene.
     */
    void enableAllLOD(LiveActor* pActor) {
        pActor->getSceneInfo()->clippingDirectorBase->sLODDisabled = false;
    }

    /**
     * @brief Turns level-of-detail switching off or on for one actor.
     * @param pActor The actor to update.
     * @param isDisabled Whether LOD switching should be disabled.
     */
    void forceLodDisabled(LiveActor* pActor, bool isDisabled) {
        pActor->getSceneInfo()->clippingDirectorBase->setLODDisabled(pActor, isDisabled);
    }

    /**
     * @brief Grows an actor's clipping sphere so it also covers the actor's shadow.
     * @param pActor The actor to update.
     * @param pCenter Receives the new clipping center (moved along gravity), or nullptr to keep the center.
     * @param shadowLength The length the shadow can drop.
     */
    void expandClippingRadiusByShadowLength(LiveActor* pActor, sead::Vector3f* pCenter, f32 shadowLength) {
        const sead::Vector3f trans = getTrans(pActor);
        f32 radius = getClippingRadius(pActor);

        if (radius >= shadowLength) {
            return;
        }

        if (pCenter != nullptr) {
            f32 newRadius = (radius + shadowLength) * 0.5f;
            *pCenter = trans + getGravity(pActor) * (newRadius - radius);
            setClippingInfo(pActor, newRadius, pCenter);
        }
        else {
            setClippingInfo(pActor, sead::Mathf::max(radius, shadowLength), nullptr);
        }
    }

    /**
     * @brief Grows an actor's clipping sphere down to the ground below it.
     * @param pActor The actor to update.
     * @param pCenter Receives the new clipping center.
     * @param distance How far down to look for ground.
     * @return True if ground was found within range and the clipping was expanded.
     */
    bool tryExpandClippingToGround(LiveActor* pActor, sead::Vector3f* pCenter, f32 distance) {
        f32 radius = getClippingRadius(pActor);
        sead::Vector3f hitPos = sead::Vector3f::zero;
        sead::Vector3f trans = getTrans(pActor);
        sead::Vector3f arrow = getGravity(pActor) * distance;

        if (!alCollisionUtil::getFirstPolyOnArrow(pActor, &hitPos, nullptr, trans, arrow, nullptr, nullptr)) {
            return false;
        }

        f32 groundDistance = (hitPos - trans).length();

        if (groundDistance < radius) {
            return false;
        }

        if (groundDistance <= distance) {
            f32 newRadius = (radius + groundDistance) * 0.5f;
            *pCenter = trans + (hitPos - trans) * ((groundDistance - newRadius) / groundDistance);
            setClippingInfo(pActor, newRadius, pCenter);
            return true;
        }

        return false;
    }

    /**
     * @brief Grows an actor's clipping sphere to cover its shadow, if it has one.
     * @param pActor The actor to update.
     * @param pCenter Receives the new clipping center, or nullptr to keep the center.
     * @return True if the actor has a shadow.
     */
    bool tryExpandClippingByShadowLength(LiveActor* pActor, sead::Vector3f* pCenter) {
        if (!isExistShadow(pActor)) {
            return false;
        }

        expandClippingRadiusByShadowLength(pActor, pCenter, getShadowDropLengthMax(pActor));
        return true;
    }

    /**
     * @brief Expands an actor's clipping to a linked ClippingExpander placement, if it has one.
     * @param pActor The actor to update.
     * @param rInfo The actor's init info.
     * @return True if the actor links to a clipping expander.
     */
    bool tryExpandClippingByExpandObject(LiveActor* pActor, const ActorInitInfo& rInfo) {
        PlacementInfo linksInfo;

        if (!tryGetLinksInfo(&linksInfo, rInfo, "ClippingExpander")) {
            return false;
        }

        PlacementClippingExpander* pExpander = new PlacementClippingExpander();
        pExpander->init(pActor, linksInfo);
        return true;
    }

    /**
     * @brief Checks whether an actor is clipped.
     * @param pActor The actor to check.
     * @return True if the actor is currently clipped.
     */
    bool isClipped(const LiveActor* pActor) {
        return pActor->mActorFlags->isClipped;
    }

    /**
     * @brief Checks whether an actor's clipping is turned off.
     * @param pActor The actor to check.
     * @return True if the actor is never clipped.
     */
    bool isInvalidClipping(const LiveActor* pActor) {
        return pActor->mActorFlags->isInvalidClipping;
    }

    /**
     * @brief Turns off clipping for an actor, unclipping it if needed.
     * @param pActor The actor to update.
     */
    void invalidateClipping(LiveActor* pActor) {
        if (pActor->mActorFlags->isClipped) {
            pActor->endClipped();
        }

        if (!pActor->mActorFlags->isInvalidClipping) {
            pActor->mGlobalAlpha = 1.0f;
            pActor->getSceneInfo()->clippingDirectorBase->invalidateActorClipping(pActor);
        }
    }

    /**
     * @brief Turns clipping back on for an actor.
     * @param pActor The actor to update.
     */
    void validateClipping(LiveActor* pActor) {
        if (pActor->mActorFlags->isInvalidClipping) {
            pActor->getSceneInfo()->clippingDirectorBase->validateActorClipping(pActor);
        }
    }

    /**
     * @brief Keeps a clipped actor moving and drawing, restoring its sensors, effects and sounds.
     * @param pActor The actor to update.
     */
    void onDrawClipping(LiveActor* pActor) {
        pActor->mActorFlags->isDrawClipping = true;

        if (pActor->mActorFlags->isClipped) {
            alActorSystemFunction::addToExecutorMovement(pActor);

            if (pActor->mHitSensorKeeper != nullptr) {
                pActor->mHitSensorKeeper->validateBySystem();
                alSensorFunction::updateHitSensorsAll(pActor);
            }

            if (pActor->getEffectKeeper() != nullptr) {
                pActor->getEffectKeeper()->onCalcAndDraw();
            }

            if (pActor->getAudioKeeper() != nullptr) {
                pActor->getAudioKeeper()->startClipped();
            }
        }
    }

    /**
     * @brief Stops a clipped actor from moving and drawing again.
     * @param pActor The actor to update.
     */
    void offDrawClipping(LiveActor* pActor) {
        pActor->mActorFlags->isDrawClipping = false;

        if (pActor->mActorFlags->isClipped) {
            alActorSystemFunction::removeFromExecutorMovement(pActor);

            if (pActor->mHitSensorKeeper != nullptr) {
                pActor->mHitSensorKeeper->invalidateBySystem();
            }

            if (pActor->getEffectKeeper() != nullptr) {
                pActor->getEffectKeeper()->offCalcAndDraw();
            }

            if (pActor->getAudioKeeper() != nullptr) {
                pActor->getAudioKeeper()->endClipped();
            }
        }
    }

    /**
     * @brief Makes the clipping judge use the camera's clipping position as the player position.
     * @param pActor Any actor in the scene.
     */
    void onUseCameraClippingPos(LiveActor* pActor) {
        pActor->getSceneInfo()->clippingDirectorBase->setClippingJudgeUsClippingPosAsPlayerPos(true);
    }

    /**
     * @brief Makes the clipping judge use the real player position again.
     * @param pActor Any actor in the scene.
     */
    void offUseCameraClippingPos(LiveActor* pActor) {
        pActor->getSceneInfo()->clippingDirectorBase->setClippingJudgeUsClippingPosAsPlayerPos(false);
    }
};

namespace alActorFunction {
    /**
     * @brief Checks whether an actor keeps drawing while clipped.
     * @param pActor The actor to check.
     * @return The actor's draw clipping flag.
     */
    bool isDrawClipping(const al::LiveActor* pActor) {
        return pActor->mActorFlags->isDrawClipping;
    }
};
