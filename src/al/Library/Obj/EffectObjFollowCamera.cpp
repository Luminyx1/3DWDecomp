#include "Library/Obj/EffectObjFollowCamera.hpp"

#include "Library/Camera/CameraUtil.hpp"
#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorFlagUtil.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorSceneUtil.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Library/Obj/EffectObjFunction.hpp"
#include "Library/Se/Function/SeFunction.hpp"
#include "Library/StageSwitch/StageSwitchFunc.hpp"
#include "Library/Thread/Functor.hpp"
#include "Project/Effect/Core/EffectUtil.hpp"

namespace {
using namespace al;

NERVE_DECL(EffectObjFollowCamera, Wait)
NERVE_DECL(EffectObjFollowCamera, Disappear)

NERVES_MAKE_NOSTRUCT(EffectObjFollowCamera, Wait, Disappear)
}  // namespace

namespace al {
/**
 * Constructs an effect object following the camera.
 * @param pName actor name
 */
EffectObjFollowCamera::EffectObjFollowCamera(const char* pName) : LiveActor(pName) {}

/**
 * Initializes the effect object and listens to its switches.
 * @param rInfo actor init info
 */
void EffectObjFollowCamera::init(const ActorInitInfo& rInfo) {
    using EffectObjFollowCameraFunctor =
        FunctorV0M<EffectObjFollowCamera*, void (EffectObjFollowCamera::*)()>;

    EffectObjFunction::initActorEffectObj(this, rInfo);
    invalidateClipping(this);
    setEffectFollowMtxPtr(this, "Wait", &mBaseMtx);
    initNerve(this, &NrvEffectObjFollowCameraWait, 0);
    if (listenStageSwitchOnOffAppear(
            this, EffectObjFollowCameraFunctor(this, &EffectObjFollowCamera::startAppear),
            EffectObjFollowCameraFunctor(this, &EffectObjFollowCamera::startDisappear))) {
        makeActorDead();
    } else {
        makeActorAppeared();
    }
    listenStageSwitchOnKill(
        this, EffectObjFollowCameraFunctor(this, &EffectObjFollowCamera::startDisappear));
}

/**
 * Appears if needed and starts the effect.
 */
void EffectObjFollowCamera::startAppear() {
    if (isDead(this)) {
        appear();
    }
    setNerve(this, &NrvEffectObjFollowCameraWait);
}

/**
 * Starts disappearing.
 */
void EffectObjFollowCamera::startDisappear() {
    setNerve(this, &NrvEffectObjFollowCameraDisappear);
}

/**
 * Follows the camera.
 */
void EffectObjFollowCamera::control() {
    mBaseMtx.setInverse(*getCameraViewMtxPtr(this));
}

/**
 * Follows the camera rotation while paused, keeping the position in snapshot mode.
 * @param isPaused whether the actor is paused
 */
void EffectObjFollowCamera::movementPaused(bool isPaused) {
    if (isSingleMode(this) && isEffectSnapshotCameraMode(this, "Wait")) {
        f32 x = mBaseMtx.m[0][3];
        f32 y = mBaseMtx.m[1][3];
        f32 z = mBaseMtx.m[2][3];
        mBaseMtx.setInverse(*getCameraViewMtxPtr(this));
        mBaseMtx.m[0][3] = x;
        mBaseMtx.m[1][3] = y;
        mBaseMtx.m[2][3] = z;
    } else {
        mBaseMtx.setInverse(*getCameraViewMtxPtr(this));
    }
    LiveActor::movementPaused(isPaused);
    updateEffects(this);
}

/**
 * Starts the effect and sound.
 */
void EffectObjFollowCamera::exeWait() {
    if (isFirstStep(this)) {
        tryEmitEffect(this, "Wait", nullptr);
        tryStartSe(this, "Wait");
    }
}

/**
 * Deletes the effect and dies after a while.
 */
void EffectObjFollowCamera::exeDisappear() {
    if (isFirstStep(this)) {
        tryDeleteEffect(this, "Wait");
    }
    if (isStep(this, 180)) {
        kill();
    }
}

/**
 * Checks if the effect is disappearing.
 * @return whether the effect is disappearing
 */
bool EffectObjFollowCamera::isDisappearing() {
    return isNerve(this, &NrvEffectObjFollowCameraDisappear);
}

/**
 * Restores a nerve saved with isDisappearing.
 * @param isWait whether to restore the wait nerve
 */
void EffectObjFollowCamera::restoreNerve(bool isWait) {
    if (isWait) {
        setNerve(this, &NrvEffectObjFollowCameraWait);
    } else {
        setNerve(this, &NrvEffectObjFollowCameraDisappear);
    }
}
}  // namespace al
