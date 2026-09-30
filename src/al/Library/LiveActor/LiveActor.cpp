#include "Library/LiveActor/LiveActor.hpp"

#include "Library/Actor/ActorInitInfo.hpp"
#include "Library/Actor/ActorPoseKeeper.hpp"
#include "Library/Audio/System/AudioKeeper.hpp"
#include "Library/Execute/ActorExecuteInfo.hpp"
#include "Library/Execute/ActorSystemFunction.hpp"
#include "Library/Execute/ExecuteUtil.hpp"
#include "Library/HitSensor/HitSensorKeeper.hpp"
#include "Library/HitSensor/SensorFunction.hpp"
#include "Library/Item/ActorItemKeeper.hpp"
#include "Library/Item/ActorScoreKeeper.hpp"
#include "Library/LiveActor/ActorCollisionFunction.hpp"
#include "Library/LiveActor/LiveActorFlag.hpp"
#include "Library/LiveActor/LiveActorFunc.hpp"
#include "Library/LiveActor/SubActorKeeper.hpp"
#include "Library/LiveActor/SubActorUtil.hpp"
#include "Library/LiveActor/Util/ActorAnimUtil.hpp"
#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorFlagUtil.hpp"
#include "Library/LiveActor/Util/ActorMapUtil.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/LiveActor/Util/ActorMovementUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/LiveActor/Util/ActorSceneInfo.hpp"
#include "Library/LiveActor/Util/ActorSceneUtil.hpp"
#include "Library/LiveActor/Util/ActorSensorUtil.hpp"
#include "Library/Math/MatrixUtil.hpp"
#include "Library/Model/ModelKeeper.hpp"
#include "Library/Model/alModelCafe.hpp"
#include "Library/Nerve/NerveKeeper.hpp"
#include "Library/Rail/RailKeeper.hpp"
#include "Library/Screen/ScreenPointKeeper.hpp"
#include "Library/Sequence/DemoDirector.hpp"
#include "Library/Shadow/Common/ShadowUtil.hpp"
#include "Library/Shadow/ShadowKeeper.hpp"
#include "Library/StageSwitch/Core/StageSwitchKeeper.hpp"
#include "Project/Action/Common/ActionSeCtrl.hpp"
#include "Project/Action/Common/ActorActionKeeper.hpp"
#include "Project/Clipping/ClippingFunction.hpp"
#include "Project/Collision/Collider.hpp"
#include "Project/Collision/CollisionParts.hpp"
#include "Project/Effect/Core/EffectKeeper.hpp"
#include "Project/Light/ActorPrePassLightKeeper.hpp"
#include "Project/Play/Actor/ActorAlphaCtrl.hpp"
#include "Project/Play/Placement/PlacementHolder.hpp"

namespace al {
namespace {
inline void syncCollisionMtxByPose(LiveActor* pActor) {
    if (pActor->mCollisionParts &&
        (!pActor->mModelKeeper || !pActor->mModelKeeper->_18)) {
        sead::Matrix34f baseMtx;
        pActor->mActorPoseKeeper->calcBaseMtx(&baseMtx);
        preScaleMtx(&baseMtx, pActor->mActorPoseKeeper->getScale());
        syncCollisionMtx(pActor, &baseMtx);
    }
}

inline bool isActiveDemo(const LiveActor* pActor) {
    DemoDirector* demoDirector = pActor->mActorSceneInfo->demoDirector;
    return demoDirector && demoDirector->isActiveDemo();
}
}  // namespace

/**
 * Updates the level of detail of the far LOD actor.
 */
inline void LiveActor::updateLOD() {
    if (mFarLodActor && mFarLodActor->mModelKeeper) {
        bool isDemo = isActiveDemo(this);

        if (isSingleMode(this)) {
            mFarLodActor->mModelKeeper->updateLod(mFarLodActor->mActorPoseKeeper->mTranslation,
                                                  isDemo);
        }
    }
}

/**
 * Constructs an actor.
 * @param pName The actor name.
 */
LiveActor::LiveActor(const char* pName) : mActorName(pName) {
    mActorFlags = new LiveActorFlag();
    mPlacementHolder = new PlacementHolder();
}

/**
 * Destroys the actor's shadow and light keepers.
 */
LiveActor::~LiveActor() {
    if (mShadowKeeper) {
        delete mShadowKeeper;
        mShadowKeeper = nullptr;
    }

    if (mLightKeeper) {
        delete mLightKeeper;
        mLightKeeper = nullptr;
    }
}

/**
 * Finishes initialization after all actors are placed.
 */
void LiveActor::initAfterPlacement() {
    tryInitFixedModelGpuBuffer(this);

    if (mLightKeeper) {
        mLightKeeper->initAfterPlacement();
    }
}

/**
 * Makes the actor appear.
 */
void LiveActor::appear() {
    makeActorAppeared();

    if (mActionKeeper && mActionKeeper->getSeCtrl()) {
        mActionKeeper->getSeCtrl()->resetAction();
    }
}

/**
 * Makes the actor appear again.
 */
void LiveActor::reappear() {}

/**
 * Makes the actor alive and enables all of its components.
 */
void LiveActor::makeActorAppeared() {
    if (mHitSensorKeeper) {
        mHitSensorKeeper->validateBySystem();
    }

    if (mScreenPointKeeper) {
        mScreenPointKeeper->validateBySystem();
    }

    mActorFlags->isDead = false;

    if (isClipped(this)) {
        endClipped();
    }

    if (!isHideModel(this) && mModelKeeper) {
        mModelKeeper->show();
    }

    resetPosition(this, false);

    if (mCollisionParts) {
        validateCollisionPartsBySystem(this);
    }

    if (mHitSensorKeeper) {
        mHitSensorKeeper->update();
    }

    alClippingFunction::addToClippingTarget(this);

    if (mActorExecuteInfo) {
        alActorSystemFunction::addToExecutorMovement(this);

        if (!isHideModel(this) && mActorExecuteInfo->mDrawerCount >= 1) {
            alActorSystemFunction::addToExecutorDraw(this);
        }
    }

    if (getAudioKeeper()) {
        getAudioKeeper()->appear();
    }

    if (mLightKeeper) {
        mLightKeeper->appear(isHideModel(this));
    }

    if (mShadowKeeper && !isHideModel(this)) {
        mShadowKeeper->show();
    }

    if (mSubActorKeeper) {
        alSubActorFunction::trySyncAlive(mSubActorKeeper);
    }
}

/**
 * Kills the actor.
 */
void LiveActor::kill() {
    makeActorDead();
}

/**
 * Kills the actor.
 * @param isComplete Unused.
 */
void LiveActor::killComplete(bool isComplete) {
    kill();
}

/**
 * Makes the actor dead and disables all of its components.
 */
void LiveActor::makeActorDead() {
    if (mFarLodActor) {
        endFarLod();
    }

    if (mActorPoseKeeper) {
        setVelocityZero(this);
    }

    mActorFlags->isDead = true;

    if (mHitSensorKeeper) {
        mHitSensorKeeper->invalidateBySystem();
    }

    if (mScreenPointKeeper) {
        mScreenPointKeeper->invalidateBySystem();
    }

    alClippingFunction::removeFromClippingTarget(this);

    if (mCollider) {
        mCollider->onInvalidate();
    }

    if (mCollisionParts) {
        invalidateCollisionPartsBySystem(this);
    }

    if (mModelKeeper) {
        mModelKeeper->hide();
    }

    if (getEffectKeeper()) {
        getEffectKeeper()->deleteAndClearEffectAll();
    }

    if (getAudioKeeper()) {
        getAudioKeeper()->kill();
    }

    if (mLightKeeper) {
        mLightKeeper->requestKill();
    }

    if (mShadowKeeper) {
        mShadowKeeper->hide();
    }

    if (mActorExecuteInfo) {
        alActorSystemFunction::removeFromExecutorMovement(this);

        if (mActorExecuteInfo->mDrawerCount >= 1) {
            alActorSystemFunction::removeFromExecutorDraw(this);
        }
    }

    if (mSubActorKeeper) {
        alSubActorFunction::trySyncDead(mSubActorKeeper);
    }
}

/**
 * Shows the actor.
 * @return Always true.
 */
bool LiveActor::showActor() {
    if (!mActorFlags->_1c) {
        validateClipping(this);
    }

    if (isDead(this)) {
        return true;
    }

    if (mActorFlags->_1c && isClipped(this)) {
        endClipped();
    }

    if (mCollisionParts) {
        mCollisionParts->validateBySystem();
    }

    return true;
}

/**
 * Hides the actor.
 * @return Always true.
 */
bool LiveActor::hideActor() {
    if (!mActorFlags->_1c && !isDead(this)) {
        invalidateClipping(this);
    }

    if (isDead(this)) {
        return true;
    }

    if (!isClipped(this)) {
        startClipped();
    }

    if (mCollisionParts) {
        mCollisionParts->invalidateBySystem();
    }

    return true;
}

/**
 * Starts a demo.
 * @param demoId The demo id.
 */
void LiveActor::startDemoActor(s32 demoId) {}

/**
 * Ends a demo.
 * @param demoId The demo id.
 */
void LiveActor::endDemoActor(s32 demoId) {}

/**
 * Changes the scenario.
 * @param scenarioId The scenario id.
 * @param isInit Whether this happens during initialization.
 */
void LiveActor::changeScenarioID(s32 scenarioId, bool isInit) {}

/**
 * Moves the actor with the actor it is linked to.
 * @param rTrans The new translation.
 */
void LiveActor::updateLinkedTrans(const sead::Vector3f& rTrans) {
    setTrans(this, rTrans);
}

/**
 * Updates the actor while the scene is paused.
 * @param isCalcAnim Whether to calculate the animation.
 */
void LiveActor::movementPaused(bool isCalcAnim) {
    if (mActorFlags->isDead) {
        return;
    }

    if (mActorFlags->isClipped && !mActorFlags->isDrawClipping) {
        return;
    }

    mGlobalAlphaLastFrame = mGlobalAlpha;

    if (_140) {
        if (_142) {
            if (mNerveKeeper && mActorFlags->isDead) {
                return;
            }

            syncCollisionMtxByPose(this);
        }

        updateLOD();
        return;
    }

    if (mModelKeeper) {
        if (isSingleMode(this)) {
            bool isDemo = isActiveDemo(this);
            mModelKeeper->updateLod(mActorPoseKeeper->mTranslation, isDemo);
        }

        mModelKeeper->updatePaused();
    }

    if (isUpdateMovementEffectAudioCollision(this)) {
        syncCollisionMtxByPose(this);
    }

    if (isCalcAnim && mActorPoseKeeper) {
        sead::Matrix34f baseMtx;
        alActorPoseFunction::calcBaseMtx(&baseMtx, this);

        if (mModelKeeper && mModelKeeper->_1a) {
            setBaseMtxAndCalcAnim(this, baseMtx, getScale(this));
        }

        if (mCollisionParts) {
            preScaleMtx(&baseMtx, getScale(this));
            syncCollisionMtx(this, &baseMtx);
        }
    }
}

/**
 * Gets the scene info.
 * @return The scene info.
 */
ActorSceneInfo* LiveActor::getSceneInfo() const {
    return mActorSceneInfo;
}

/**
 * Updates the actor.
 */
void LiveActor::movement() {
    if (mActorFlags->isDead) {
        return;
    }

    if (mActorFlags->isClipped && !mActorFlags->isDrawClipping) {
        return;
    }

    mGlobalAlphaLastFrame = mGlobalAlpha;

    if (_140) {
        if (_142) {
            if (mNerveKeeper) {
                mNerveKeeper->update();

                if (mActorFlags->isDead) {
                    return;
                }
            }

            if (mAudioKeeper) {
                mAudioKeeper->update();
            }

            if (_143 && mActionKeeper) {
                mActionKeeper->updateSeActionCtrl();
            }

            control();
            syncCollisionMtxByPose(this);
        }

        updateLOD();
        return;
    }

    if (mActionKeeper) {
        mActionKeeper->updatePrev();
    }

    if (mModelKeeper) {
        bool isDemo = isActiveDemo(this);

        if (isSingleMode(this)) {
            mModelKeeper->updateLod(mActorPoseKeeper->mTranslation, isDemo);
        }

        mModelKeeper->update();
    }

    if (mActorFlags->isValidCeilWallFloorMatCode) {
        if (isCollidedGround(this)) {
            setMaterialCode(this, getCollidedFloorMaterialCodeName(this));
        } else if (isCollidedWall(this)) {
            setMaterialCode(this, getCollidedWallMaterialCodeName(this));
        } else if (isCollidedCeiling(this)) {
            setMaterialCode(this, getCollidedCeilingMaterialCodeName(this));
        }
    }

    if (mActorFlags->isValidMatCode && isCollidedGround(this)) {
        setMaterialCode(this, getCollidedFloorMaterialCodeName(this));
    }

    if (mHitSensorKeeper) {
        mHitSensorKeeper->attackSensor();

        if (mActorFlags->isDead) {
            return;
        }
    }

    if (mNerveKeeper) {
        mNerveKeeper->update();

        if (mActorFlags->isDead) {
            return;
        }
    }

    control();

    if (mActorFlags->isDead) {
        return;
    }

    updateCollider();

    if (isUpdateMovementEffectAudioCollision(this)) {
        if (mEffectKeeper) {
            mEffectKeeper->update();
        }

        if (mAudioKeeper) {
            mAudioKeeper->update();
        }

        syncCollisionMtxByPose(this);
    }

    if (mActionKeeper) {
        mActionKeeper->updatePost();
    }

    if (mHitSensorKeeper) {
        mHitSensorKeeper->update();
    }

    if (mScreenPointKeeper) {
        mScreenPointKeeper->update();
    }

    if (mModelKeeper) {
        mModelKeeper->mModelCafe->updateLast();
    }
}

/**
 * Calculates the animation of the actor.
 */
void LiveActor::calcAnim() {
    if (mActorFlags->isDead) {
        return;
    }

    if (mActorFlags->isClipped && !mActorFlags->isDrawClipping) {
        return;
    }

    if (_140) {
        return;
    }

    if (mActorPoseKeeper) {
        alLiveActorFunction::calcAnimDirect(this);
    }

    if (mAudioKeeper) {
        mAudioKeeper->update();
    }
}

/**
 * Updates the alpha of the model.
 * @return Whether the model should be updated.
 */
bool LiveActor::modelUpdate() {
    if (mActorFlags->isDead) {
        return false;
    }

    if (mActorFlags->isClipped && !mActorFlags->isDrawClipping) {
        return false;
    }

    if (mAlphaCtrl) {
        f32 alpha = mAlphaCtrl->update(getClippingJudge(this));

        if (!isInvalidClipping(this)) {
            alpha *= mGlobalAlpha;
        }

        mGlobalAlpha = alpha;
    }

    return true;
}

/**
 * Updates the model while paused.
 */
void LiveActor::pausedModelUpdate() {
    if (modelUpdate()) {
        mGlobalAlphaLastFrame = mGlobalAlpha;
    }
}

/**
 * Draws the actor.
 */
void LiveActor::draw() const {}

/**
 * Pauses the actor.
 */
void LiveActor::pause() {}

/**
 * Resumes the actor.
 */
void LiveActor::resume() {}

/**
 * Stops updating the actor because it is clipped.
 */
void LiveActor::startClipped() {
    mActorFlags->isClipped = true;

    if (!mActorFlags->isClippedByLOD) {
        if (mModelKeeper) {
            mModelKeeper->hide();
        }

        if (!mActorFlags->isDrawClipping) {
            if (mHitSensorKeeper) {
                mHitSensorKeeper->invalidateBySystem();
            }

            if (getEffectKeeper()) {
                getEffectKeeper()->offCalcAndDraw();
            }

            if (getAudioKeeper()) {
                getAudioKeeper()->startClipped();
            }
        }

        if (mShadowKeeper) {
            hideShadow(this);
        }

        if (mScreenPointKeeper) {
            mScreenPointKeeper->invalidateBySystem();
        }

        if (mLightKeeper) {
            mLightKeeper->hideModel();
        }

        if (mActorExecuteInfo) {
            if (!mActorFlags->isDrawClipping) {
                alActorSystemFunction::removeFromExecutorMovement(this);
            }

            if (mActorExecuteInfo->mDrawerCount >= 1) {
                alActorSystemFunction::removeFromExecutorDraw(this);
            }

            if (mActorFlags->isDrawClipping && mModelKeeper && mModelKeeper->_19) {
                setNeedSetBaseMtxAndCalcAnimFlag(this, false);
            }
        }

        if (mSubActorKeeper) {
            alSubActorFunction::trySyncClippingStart(mSubActorKeeper);
        }
    }

    if (mFarLodActor && !isClipped(mFarLodActor)) {
        mFarLodActor->startClipped();
    }
}

/**
 * Resumes updating the actor because it is no longer clipped.
 */
void LiveActor::endClipped() {
    mActorFlags->isClipped = false;

    if (!mActorFlags->isClippedByLOD) {
        if (!mActorFlags->isDrawClipping) {
            if (mHitSensorKeeper) {
                mHitSensorKeeper->validateBySystem();
                updateHitSensorsAll(this);
            }

            if (getEffectKeeper()) {
                getEffectKeeper()->onCalcAndDraw();
            }

            if (getAudioKeeper()) {
                getAudioKeeper()->endClipped();
            }
        }

        if (mScreenPointKeeper) {
            mScreenPointKeeper->validateBySystem();
        }

        if (mActorExecuteInfo) {
            if (!mActorFlags->isDrawClipping) {
                alActorSystemFunction::addToExecutorMovement(this);
            }

            if (mActorExecuteInfo->mDrawerCount >= 1 && !isHideModel(this)) {
                alActorSystemFunction::addToExecutorDraw(this);
            }

            if (mModelKeeper && mModelKeeper->_19) {
                setNeedSetBaseMtxAndCalcAnimFlag(this, true);
            }
        }

        if (!isHideModel(this) && mModelKeeper) {
            mModelKeeper->show();
        }

        if (mLightKeeper) {
            mLightKeeper->appear(isHideModel(this));
        }

        if (mShadowKeeper && !isHideModel(this)) {
            showShadow(this);
        }

        if (mSubActorKeeper) {
            alSubActorFunction::trySyncClippingEnd(mSubActorKeeper);
        }

        if (mActorFlags->isDrawClipping && isSingleMode(this)) {
            alLiveActorFunction::forceUpdateTrans(this, getTrans(this), false);
        }
    }

    if (mFarLodActor && _140 && isClipped(mFarLodActor)) {
        mFarLodActor->endClipped();
    }
}

/**
 * Stops updating the actor because of its level of detail.
 */
void LiveActor::startClippedByLod() {
    mActorFlags->isClippedByLOD = true;

    if (mActorFlags->isClipped) {
        return;
    }

    if (mModelKeeper) {
        mModelKeeper->hide();
    }

    if (!mActorFlags->isDrawClipping) {
        if (mHitSensorKeeper) {
            mHitSensorKeeper->invalidateBySystem();
        }

        if (getEffectKeeper()) {
            getEffectKeeper()->offCalcAndDraw();
        }

        if (getAudioKeeper()) {
            getAudioKeeper()->startClipped();
        }
    }

    if (mShadowKeeper) {
        hideShadow(this);
    }

    if (mScreenPointKeeper) {
        mScreenPointKeeper->invalidateBySystem();
    }

    if (mLightKeeper) {
        mLightKeeper->hideModel();
    }

    if (mActorExecuteInfo) {
        if (!mActorFlags->isDrawClipping) {
            alActorSystemFunction::removeFromExecutorMovement(this);
        }

        if (mActorExecuteInfo->mDrawerCount >= 1) {
            alActorSystemFunction::removeFromExecutorDraw(this);
        }

        if (mActorFlags->isDrawClipping && mModelKeeper && mModelKeeper->_19) {
            setNeedSetBaseMtxAndCalcAnimFlag(this, false);
        }
    }

    if (mSubActorKeeper) {
        alSubActorFunction::trySyncClippingStart(mSubActorKeeper);
    }
}

/**
 * Resumes updating the actor after its level of detail changed.
 */
void LiveActor::endClippedByLod() {
    mActorFlags->isClippedByLOD = false;

    if (mActorFlags->isClipped) {
        return;
    }

    if (!mActorFlags->isDrawClipping) {
        if (mHitSensorKeeper) {
            mHitSensorKeeper->validateBySystem();
            updateHitSensorsAll(this);
        }

        if (getEffectKeeper()) {
            getEffectKeeper()->onCalcAndDraw();
        }

        if (getAudioKeeper()) {
            getAudioKeeper()->endClipped();
        }
    }

    if (mScreenPointKeeper) {
        mScreenPointKeeper->validateBySystem();
    }

    if (mActorExecuteInfo) {
        if (!mActorFlags->isDrawClipping) {
            alActorSystemFunction::addToExecutorMovement(this);
        }

        if (mActorExecuteInfo->mDrawerCount >= 1 && !isHideModel(this)) {
            alActorSystemFunction::addToExecutorDrawImmediate(this);
        }

        if (mModelKeeper && mModelKeeper->_19) {
            setNeedSetBaseMtxAndCalcAnimFlag(this, true);
        }
    }

    if (!isHideModel(this) && mModelKeeper) {
        mModelKeeper->show();
    }

    if (mLightKeeper) {
        mLightKeeper->appear(isHideModel(this));
    }

    if (mShadowKeeper && !isHideModel(this)) {
        showShadow(this);
    }

    if (mSubActorKeeper) {
        alSubActorFunction::trySyncClippingEnd(mSubActorKeeper);
    }
}

/**
 * Sets the global Y offset of the actor, its sub actors and its far LOD actors.
 * @param pYOffset The Y offset.
 */
void LiveActor::setGlobalYOffsetRef(f32* pYOffset) {
    LiveActor* actor = this;

    do {
        actor->mGlobalYOffsetRef = pYOffset;

        if (actor->mModelKeeper) {
            actor->mModelKeeper->setGlobalYOffset(pYOffset);
        }

        if (actor->mSubActorKeeper) {
            alSubActorFunction::setGlobalYOffset(actor->mSubActorKeeper, pYOffset);
        }

        actor = actor->mFarLodActor;
    } while (actor);
}

/**
 * Gets the global Y offset.
 * @return The Y offset.
 */
f32 LiveActor::getGlobalYOffset() const {
    return mGlobalYOffsetRef ? *mGlobalYOffsetRef : mGlobalYOffset;
}

/**
 * Sets the actor shown at far level of detail.
 * @param pActor The far LOD actor.
 */
void LiveActor::setFarLodActor(LiveActor* pActor) {
    mFarLodActor = pActor;
}

/**
 * Switches to the far level of detail.
 */
void LiveActor::startFarLod() {
    if (_140) {
        return;
    }

    _140 = true;

    if (mFarLodActor && isClipped(mFarLodActor) && !isClipped(this)) {
        mFarLodActor->endClipped();
        alActorSystemFunction::addToExecutorDrawImmediate(mFarLodActor);
    }

    if (!_142) {
        startClippedByLod();
        return;
    }

    if (mFarLodActor && mActorPoseKeeper) {
        alLiveActorFunction::calcAnimDirect(mFarLodActor);
    }

    if (mActorExecuteInfo && mActorExecuteInfo->mDrawerCount >= 1) {
        hideModelIfShow(this);
    }
}

/**
 * Switches back from the far level of detail.
 */
void LiveActor::endFarLod() {
    if (!_140) {
        return;
    }

    _140 = false;

    if (mFarLodActor && !isClipped(mFarLodActor)) {
        mFarLodActor->startClipped();
    }

    if (!_142) {
        endClippedByLod();
        return;
    }

    if (mActorPoseKeeper) {
        alLiveActorFunction::calcAnimDirect(this);
    }

    if (mActorExecuteInfo && mActorExecuteInfo->mDrawerCount >= 1) {
        showModelIfHide(this);

        if (!isDead(this) && !isClipped(this)) {
            alActorSystemFunction::addToExecutorDrawImmediate(this);
        }
    }
}

/**
 * Gets the base matrix of the model or the pose.
 * @return The base matrix, or nullptr.
 */
const sead::Matrix34f* LiveActor::getBaseMtx() const {
    if (mModelKeeper) {
        return mModelKeeper->mModelCafe->mBaseMtx;
    }

    if (mActorPoseKeeper) {
        return mActorPoseKeeper->getMtxPtr();
    }

    return nullptr;
}

/**
 * Sets whether the actor is a far LOD model.
 * @param isFarLodModel Whether the actor is a far LOD model.
 */
void LiveActor::setIsFarLodModel(bool isFarLodModel) {
    mIsFarLodModel = isFarLodModel;
}

/**
 * Gets the scene object holder.
 * @return The scene object holder.
 */
SceneObjHolder* LiveActor::getSceneObjHolder() const {
    return mActorSceneInfo->sceneObjHolder;
}

/**
 * Gets the collision director.
 * @return The collision director.
 */
CollisionDirector* LiveActor::getCollisionDirector() const {
    return mActorSceneInfo->collisionDirector;
}

/**
 * Gets the area object director.
 * @return The area object director.
 */
AreaObjDirector* LiveActor::getAreaObjDirector() const {
    return mActorSceneInfo->areaObjDirector;
}

/**
 * Gets the scene camera info.
 * @return The scene camera info.
 */
SceneCameraInfo* LiveActor::getSceneCameraInfo() const {
    return mActorSceneInfo->sceneCameraInfo;
}

/**
 * Gets the camera director.
 * @return The camera director.
 */
CameraDirector_RS* LiveActor::getCameraDirector_RS() const {
    return mActorSceneInfo->cameraDirector;
}

/**
 * Sets the pose keeper.
 * @param pPoseKeeper The pose keeper.
 */
void LiveActor::initPoseKeeper(ActorPoseKeeperBase* pPoseKeeper) {
    mActorPoseKeeper = pPoseKeeper;
}

/**
 * Sets the execute info.
 * @param pExecuteInfo The execute info.
 */
void LiveActor::initExecuteInfo(ActorExecuteInfo* pExecuteInfo) {
    mActorExecuteInfo = pExecuteInfo;
}

/**
 * Sets the model keeper and links the global alpha and Y offset to it.
 * @param pModelKeeper The model keeper.
 */
void LiveActor::initModelKeeper(ModelKeeper* pModelKeeper) {
    mModelKeeper = pModelKeeper;
    mModelKeeper->setGlobalAlpha(&mGlobalAlphaLastFrame);
    mModelKeeper->setGlobalYOffset(&mGlobalYOffset);
    offUpdateMovementEffectAudioCollision(this);
}

/**
 * Creates the action keeper.
 * @param pArchiveName The archive name.
 * @param pSuffix The file suffix.
 */
void LiveActor::initActionKeeper(const char* pArchiveName, const char* pSuffix) {
    mActionKeeper = ActorActionKeeper::tryCreate(this, pArchiveName, pSuffix);

    if (mActionKeeper) {
        mActionKeeper->init();
    }
}

/**
 * Sets the nerve keeper.
 * @param pNerveKeeper The nerve keeper.
 */
void LiveActor::initNerveKeeper(NerveKeeper* pNerveKeeper) {
    mNerveKeeper = pNerveKeeper;
}

/**
 * Creates the hit sensor keeper.
 * @param num The maximum number of sensors.
 */
void LiveActor::initHitSensor(s32 num) {
    mHitSensorKeeper = new HitSensorKeeper(num);
}

/**
 * Creates the screen point keeper.
 * @param num The maximum number of targets.
 */
void LiveActor::initScreenPointKeeper(s32 num) {
    mScreenPointKeeper = new ScreenPointKeeper(num);
}

/**
 * Sets the effect keeper and connects it to the model and camera.
 * @param pEffectKeeper The effect keeper.
 */
void LiveActor::initEffectKeeper(EffectKeeper* pEffectKeeper) {
    mEffectKeeper = pEffectKeeper;

    if (mModelKeeper) {
        alEffectKeeperInitFunction::setupModelToEffectKeeper(mEffectKeeper, mModelKeeper);
    }

    alEffectKeeperInitFunction::setupCameraToEffectKeeper(mEffectKeeper, this);
}

/**
 * Sets the audio keeper.
 * @param pAudioKeeper The audio keeper.
 */
void LiveActor::initAudioKeeper(AudioKeeper* pAudioKeeper) {
    mAudioKeeper = pAudioKeeper;
}

/**
 * Sets the ocean wave keeper.
 * @param pOceanWaveKeeper The ocean wave keeper.
 */
void LiveActor::initOceanWaveKeeper(OceanWaveKeeper* pOceanWaveKeeper) {
    mOceanWaveKeeper = pOceanWaveKeeper;
}

/**
 * Creates the stage switch keeper.
 */
void LiveActor::initStageSwitchKeeper() {
    mStageSwitchKeeper = new StageSwitchKeeper();
}

/**
 * Creates the rail keeper from the placement.
 * @param rInfo The actor init info.
 */
void LiveActor::initRailKeeper(const ActorInitInfo& rInfo) {
    mRailKeeper = tryCreateRailKeeper(*rInfo.mPlacementInfo, "Rail");
}

/**
 * Creates the collider.
 * @param radius The radius.
 * @param offsetY The Y offset.
 * @param hitInfoNum The number of stored hit infos.
 */
void LiveActor::initCollider(f32 radius, f32 offsetY, u32 hitInfoNum) {
    mCollider = new Collider(getCollisionDirector(), getBaseMtx(), &getTrans(this),
                             &getGravity(this), radius, offsetY, hitInfoNum);
    mActorFlags->isNoCollide = false;
}

/**
 * Sets the shadow keeper.
 * @param pShadowKeeper The shadow keeper.
 */
void LiveActor::initShadowKeeper(ShadowKeeper* pShadowKeeper) {
    mShadowKeeper = pShadowKeeper;
}

/**
 * Creates the item keeper.
 * @param num The maximum number of items.
 */
void LiveActor::initItemKeeper(s32 num) {
    mItemKeeper = new ActorItemKeeper(this, num);
}

/**
 * Creates the score keeper.
 */
void LiveActor::initScoreKeeper() {
    mScoreKeeper = new ActorScoreKeeper();
}

/**
 * Sets the pre-pass light keeper.
 * @param pLightKeeper The light keeper.
 */
void LiveActor::initActorPrePassLightKeeper(ActorPrePassLightKeeper* pLightKeeper) {
    mLightKeeper = pLightKeeper;
}

/**
 * Sets the sub actor keeper and links the sub actors to the global alpha.
 * @param pSubActorKeeper The sub actor keeper.
 */
void LiveActor::initSubActorKeeper(SubActorKeeper* pSubActorKeeper) {
    mSubActorKeeper = pSubActorKeeper;
    setSubActorAlphaPtr(this, &mGlobalAlphaLastFrame);
}

/**
 * Sets the scene info.
 * @param pSceneInfo The scene info.
 */
void LiveActor::initSceneInfo(ActorSceneInfo* pSceneInfo) {
    mActorSceneInfo = pSceneInfo;
}

/**
 * Sets the alpha control and registers the actor for alpha updates.
 * @param pAlphaCtrl The alpha control.
 * @param rInfo The actor init info.
 */
void LiveActor::initActorAlphaCtrl(ActorAlphaCtrl* pAlphaCtrl, const ActorInitInfo& rInfo) {
    if (pAlphaCtrl) {
        mAlphaCtrl = pAlphaCtrl;
        registerExecutorActorUpdate(this, rInfo.mExecuteDirector, "アルファ制御");
    }
}

/**
 * Enables or disables collision.
 * @param isEnable Whether collision is enabled.
 */
void LiveActor::setCollision(bool isEnable) {
    if (isEnable) {
        if (mCollisionParts) {
            validateCollisionPartsBySystem(this);
        }

        return;
    }

    if (mCollider) {
        mCollider->onInvalidate();
    }

    if (mCollisionParts) {
        invalidateCollisionPartsBySystem(this);
    }
}

/**
 * Runs the actor's logic.
 */
void LiveActor::control() {}

/**
 * Moves the actor by its velocity, colliding if it has a collider.
 */
void LiveActor::updateCollider() {
    if (!mActorPoseKeeper) {
        return;
    }

    const sead::Vector3f& velocity = getVelocity(this);

    if (!mCollider) {
        *getTransPtr(this) += velocity;
        return;
    }

    if (mActorFlags->isNoCollide) {
        *getTransPtr(this) += velocity;
        mCollider->onInvalidate();
        return;
    }

    sead::Vector3f move = mCollider->collide(velocity);
    *getTransPtr(this) += move;
}

/**
 * Initializes the placement holder.
 * @param rInfo The actor init info.
 */
void LiveActor::setPlacementHolder(const ActorInitInfo& rInfo) {
    mPlacementHolder->init(*rInfo.mPlacementInfo);
}

/**
 * Sets the global alpha pointer of the model and sub actors.
 * @param pAlpha The alpha pointer.
 */
void LiveActor::setGlobalAlphaPtr(f32* pAlpha) {
    if (mModelKeeper) {
        mModelKeeper->setGlobalAlpha(pAlpha);
    }

    if (mSubActorKeeper) {
        setSubActorAlphaPtr(this, pAlpha);
    }
}

}  // namespace al

namespace alLiveActorFunction {
/**
 * Calculates the model animation, collision matrix and effects of an actor.
 * @param pActor The actor.
 */
void calcAnimDirect(al::LiveActor* pActor) {
    sead::Matrix34f baseMtx;
    alActorPoseFunction::calcBaseMtx(&baseMtx, pActor);

    if (pActor->mModelKeeper && pActor->mModelKeeper->_1a) {
        al::setBaseMtxAndCalcAnim(pActor, baseMtx, al::getScale(pActor));
    }

    if (pActor->mCollisionParts) {
        al::preScaleMtx(&baseMtx, al::getScale(pActor));
        al::syncCollisionMtx(pActor, &baseMtx);
    }

    if (pActor->getEffectKeeper()) {
        pActor->getEffectKeeper()->update();
    }
}

/**
 * Moves an actor and immediately updates its model.
 * @param pActor The actor.
 * @param rTrans The new translation.
 * @param isUpdateSubActor Whether to move the sub actors too.
 */
void forceUpdateTrans(al::LiveActor* pActor, const sead::Vector3f& rTrans, bool isUpdateSubActor) {
    al::setTrans(pActor, rTrans);
    calcAnimDirect(pActor);

    if (pActor->mModelKeeper) {
        pActor->mModelKeeper->update();
    }

    calcAnimDirect(pActor);

    if (!isUpdateSubActor) {
        return;
    }

    al::SubActorKeeper* keeper = pActor->mSubActorKeeper;

    if (!keeper) {
        return;
    }

    for (s32 i = 0; i < keeper->mCount; i++) {
        al::LiveActor* subActor = keeper->mInfos[i]->mSubActor;

        if (subActor) {
            forceUpdateTrans(subActor, rTrans, true);
        }
    }
}

/**
 * Enables or disables the alpha control of an actor and its sub actors.
 * @param pActor The actor.
 * @param isOn Whether the alpha control is enabled.
 */
void setAlphaCtrlOn(al::LiveActor* pActor, bool isOn) {
    if (pActor->mAlphaCtrl) {
        pActor->mAlphaCtrl->mIsOn = isOn;
    }

    al::SubActorKeeper* keeper = pActor->mSubActorKeeper;

    if (!keeper) {
        return;
    }

    for (s32 i = 0; i < keeper->mCount; i++) {
        al::LiveActor* subActor = keeper->mInfos[i]->mSubActor;

        if (subActor && subActor->mAlphaCtrl) {
            subActor->mAlphaCtrl->mIsOn = isOn;
        }
    }
}
}  // namespace alLiveActorFunction
