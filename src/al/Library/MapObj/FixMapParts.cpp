#include "Library/MapObj/FixMapParts.hpp"

#include "Library/Connector/MtxConnector.hpp"
#include "Library/LiveActor/LiveActorFunc.hpp"
#include "Library/LiveActor/Util/ActorActionUtil.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorMapUtil.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/LiveActor/Util/ActorSensorUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"

namespace al {
/**
 * Constructs a fixed map part.
 * @param pName actor name
 */
FixMapParts::FixMapParts(const char* pName) : LiveActor(pName) {}

/**
 * Initializes the map part with a model suffix.
 * @param rInfo actor init info
 * @param pSuffix model suffix
 */
void FixMapParts::initWithSuffix(const ActorInitInfo& rInfo, const char* pSuffix) {
    initMapPartsActor(this, rInfo, pSuffix, 0);
    if (!mActorPoseKeeper) {
        initActorPoseTQSV(this);
    }

    if (!trySyncStageSwitchAppear(this)) {
        trySyncStageSwitchKill(this);
    }

    bool isConnectCollision = false;
    if (tryGetArg(&isConnectCollision, rInfo, "IsConnectCollision") && isConnectCollision) {
        mConnector = createMtxConnector(this);
    }
}

/**
 * Initializes the map part.
 * @param rInfo actor init info
 */
void FixMapParts::init(const ActorInitInfo& rInfo) {
    initWithSuffix(rInfo, nullptr);
}

/**
 * Attaches the connector to the collision below after placement.
 */
void FixMapParts::initAfterPlacement() {
    LiveActor::initAfterPlacement();
    if (mConnector) {
        attachMtxConnectorToCollision(mConnector, this, false);
    }
}

/**
 * Makes the map part appear.
 */
void FixMapParts::appear() {
    LiveActor::appear();
    if (mModelKeeper) {
        tryStartAction(this, "Appear");
    }
}

/**
 * Follows the connected collision.
 */
void FixMapParts::control() {
    if (mConnector) {
        connectPoseTrans(this, mConnector, getConnectBaseTrans(mConnector));
    }
}

/**
 * Moves the map part with a linked actor.
 * @param rTrans new translation
 */
void FixMapParts::updateLinkedTrans(const sead::Vector3f& rTrans) {
    alLiveActorFunction::forceUpdateTrans(this, rTrans, true);
}

/**
 * Handles safety point, model visibility and sink messages.
 * @param pMsg message
 * @param pOther sender sensor
 * @param pSelf receiver sensor
 * @return whether the message was handled
 */
bool FixMapParts::receiveMsg(const SensorMsg* pMsg, HitSensor* pOther, HitSensor* pSelf) {
    if (isMsgAskSafetyPoint(pMsg)) {
        return true;
    }

    if (isMsgShowModel(pMsg)) {
        showModelIfHide(this);
        return true;
    }

    if (isMsgHideModel(pMsg)) {
        hideModelIfShow(this);
        return true;
    }

    return isMsgSink(pMsg);
}
}  // namespace al
