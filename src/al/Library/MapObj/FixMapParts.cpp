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
     * @brief Constructs a fixed map part.
     * @param pName The actor name.
     */
    FixMapParts::FixMapParts(const char* pName) : LiveActor(pName) {}

    /**
     * @brief Initializes the map part with a model suffix and optionally connects it to collision.
     * @param rInfo The actor init info.
     * @param pSuffix The model suffix, or nullptr for none.
     */
    void FixMapParts::initWithSuffix(const ActorInitInfo& rInfo, const char* pSuffix) {
        initMapPartsActor(this, rInfo, pSuffix, 0);

        if (mActorPoseKeeper == nullptr) {
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
     * @brief Initializes the map part without a model suffix.
     * @param rInfo The actor init info.
     */
    void FixMapParts::init(const ActorInitInfo& rInfo) {
        initWithSuffix(rInfo, nullptr);
    }

    /**
     * @brief Attaches the connector to the collision below after placement.
     */
    void FixMapParts::initAfterPlacement() {
        LiveActor::initAfterPlacement();
        if (mConnector != nullptr) {
            attachMtxConnectorToCollision(mConnector, this, false);
        }
    }

    /**
     * @brief Appears and plays the appear action if the part has a model.
     */
    void FixMapParts::appear() {
        LiveActor::appear();
        if (mModelKeeper != nullptr) {
            tryStartAction(this, "Appear");
        }
    }

    /**
     * @brief Follows the connected collision if there is one.
     */
    void FixMapParts::control() {
        if (mConnector != nullptr) {
            connectPoseTrans(this, mConnector, getConnectBaseTrans(mConnector));
        }
    }

    /**
     * @brief Moves the part to a linked position.
     * @param rTrans The new position.
     */
    void FixMapParts::updateLinkedTrans(const sead::Vector3f& rTrans) {
        alLiveActorFunction::forceUpdateTrans(this, rTrans, true);
    }

    /**
     * @brief Handles safety point, show/hide model and sink messages.
     * @param pMsg The received message.
     * @param pSelf The receiving sensor.
     * @param pOther The sending sensor.
     * @return Whether the message was handled.
     */
    bool FixMapParts::receiveMsg(const SensorMsg* pMsg, HitSensor* pSelf, HitSensor* pOther) {
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
