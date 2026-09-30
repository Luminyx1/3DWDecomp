#include "Library/Obj/EffectObj.hpp"

#include "Library/Collision/PartsConnectorUtil.hpp"
#include "Library/Connector/MtxConnector.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Obj/EffectObjFunction.hpp"
#include "Library/Se/Function/SeFunction.hpp"
#include "Library/StageSwitch/StageSwitchFunc.hpp"
#include "Library/LiveActor/Util/ActorMapUtil.hpp"
#include "Library/Thread/Functor.hpp"
#include "Project/Effect/Core/EffectUtil.hpp"

namespace al {
/**
 * Constructs an effect object.
 * @param pName actor name
 */
EffectObj::EffectObj(const char* pName) : LiveActor(pName) {}

/**
 * Initializes the effect object and listens to its switches.
 * @param rInfo actor init info
 */
void EffectObj::init(const ActorInitInfo& rInfo) {
    using EffectObjFunctor = FunctorV0M<EffectObj*, void (EffectObj::*)()>;

    EffectObjFunction::initActorEffectObj(this, rInfo);
    makeMtxRT(&mBaseMtx, this);
    trySyncStageSwitchAppear(this);
    tryListenStageSwitchKill(this);
    listenStageSwitchOnOff(this, "OnKillOffAppearSwitch", EffectObjFunctor(this, &EffectObj::kill),
                           EffectObjFunctor(this, &EffectObj::appear));
    listenStageSwitchOnOff(this, "OnAppearOnAppearSwitch",
                           EffectObjFunctor(this, &EffectObj::appearBySwitch),
                           EffectObjFunctor(this, &EffectObj::killBySwitch));
    mMtxConnector = tryCreateMtxConnector(this, rInfo);
}

/**
 * Appears when the appear switch turns on.
 */
void EffectObj::appearBySwitch() {
    makeActorAppeared();
}

/**
 * Dies when the appear switch turns off, unless the object appeared normally.
 */
void EffectObj::killBySwitch() {
    if (mIsAppeared) {
        return;
    }
    makeActorDead();
}

/**
 * Connects to the collision below the object.
 */
void EffectObj::initAfterPlacement() {
    if (mMtxConnector) {
        attachMtxConnectorToCollision(mMtxConnector, this, false);
    }
}

/**
 * Appears and starts the effect and sound.
 */
void EffectObj::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    makeMtxRT(&mBaseMtx, this);
    emitEffect(this, "Wait", nullptr);
    tryStartSe(this, "Wait");
}

/**
 * Follows the connected collision and updates the base matrix.
 */
void EffectObj::control() {
    if (mMtxConnector) {
        connectPoseQT(this, mMtxConnector);
    }
    makeMtxRT(&mBaseMtx, this);
    mBaseMtx.m[1][3] += getGlobalYOffset();
}

/**
 * Keeps updating the effects while paused.
 * @param isPaused whether the actor is paused
 */
void EffectObj::movementPaused(bool isPaused) {
    LiveActor::movementPaused(isPaused);
    updateEffects(this);
}

/**
 * Appears.
 */
void EffectObj::appear() {
    mIsAppeared = true;
    LiveActor::appear();
}

/**
 * Kills the object.
 */
void EffectObj::kill() {
    mIsAppeared = false;
    LiveActor::kill();
}
}  // namespace al
