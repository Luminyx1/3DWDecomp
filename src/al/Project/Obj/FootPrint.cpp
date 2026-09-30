#include "Project/Obj/FootPrint.hpp"

#include "Library/Collision/PartsConnector.hpp"
#include "Library/Collision/PartsConnectorUtil.hpp"
#include "Library/LiveActor/Util/ActorAnimUtil.hpp"
#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Model/ModelKeeper.hpp"
#include "Library/Model/alModelCafe.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Project/Collision/CollisionParts.hpp"
#include "Project/Model/SimpleModelG3D.hpp"

namespace {
using namespace al;

NERVE_DECL(FootPrint, Appear)
NERVE_DECL(FootPrint, Disappear)

NERVES_MAKE_NOSTRUCT(FootPrint, Appear, Disappear)
}  // namespace

namespace al {
/**
 * Constructs a dead foot print.
 * @param rInfo actor init info
 * @param pArchiveName foot print archive name
 */
FootPrint::FootPrint(const ActorInitInfo& rInfo, const char* pArchiveName)
    : LiveActor("足跡オブジェ") {
    initActorWithArchiveNameNoPlacementInfo(this, rInfo, pArchiveName, nullptr);
    initNerve(this, &NrvFootPrintAppear, 0);
    mConnector = new CollisionPartsConnector();
    makeActorDead();
}

/**
 * Appears, hiding the model from the depth shadow for the first steps.
 */
void FootPrint::appear() {
    mModelKeeper->getModelCafe()->getModelG3D()->_45 = true;
    LiveActor::appear();
    setNerve(this, &NrvFootPrintAppear);
    invalidateClipping(this);
}

/**
 * Starts disappearing.
 */
void FootPrint::startDisappear() {
    setNerve(this, &NrvFootPrintDisappear);
}

/**
 * Checks if the foot print is disappearing.
 * @return whether the foot print is disappearing
 */
bool FootPrint::isDisappear() const {
    return isNerve(this, &NrvFootPrintDisappear);
}

/**
 * Shows the foot print while it is connected to the collision.
 */
void FootPrint::exeAppear() {
    if (isFirstStep(this)) {
        startMclAnim(this, mMclAnimName);
        setMclAnimFrameAndStop(this, 0.0f);
    }

    if (isStep(this, 3)) {
        mModelKeeper->getModelCafe()->getModelG3D()->_45 = false;
    }

    if (mConnector->isConnecting()) {
        return;
    }

    kill();
}

/**
 * Plays the disappear animation.
 */
void FootPrint::exeDisappear() {
    if (isFirstStep(this)) {
        startMclAnim(this, mMclAnimName);
    }

    if (isMclAnimEnd(this)) {
        kill();
    }
}

/**
 * Sets the animation of the floor material.
 * @param pAnimName animation name
 */
void FootPrint::setAnimationByMaterial(const char* pAnimName) {
    startMtsAnim(this, pAnimName);
    mMclAnimName = pAnimName;
}

/**
 * Sets the animation of the character.
 * @param pAnimName animation name
 */
void FootPrint::setAnimationByCharacter(const char* pAnimName) {
    tryStartMtpAnimIfExist(this, pAnimName);
}

/**
 * Sets the animation of the metamorphosis.
 * @param pAnimName animation name
 */
void FootPrint::setAnimationByMetamorphosis(const char* pAnimName) {
    tryStartVisAnimIfExist(this, pAnimName);
}

/**
 * Connects the foot print to a collision part.
 * @param pParts collision part
 */
void FootPrint::setFollowCollisionParts(const CollisionParts* pParts) {
    mConnector->setBaseQuatTrans(getQuat(this), getTrans(this));
    mConnector->init(&pParts->mBaseMtx, pParts->mBaseInvMtx, pParts);
    mCollisionParts = pParts;
}

/**
 * Follows the connected collision part.
 */
void FootPrint::control() {
    if (mCollisionParts->_154) {
        return;
    }

    connectPoseQT(this, mConnector);
}
}  // namespace al
