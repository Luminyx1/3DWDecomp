#include "Library/Obj/MessageSyncParts.hpp"

namespace al {
    /**
     * @brief Constructs a part that forwards sensor and screen point messages to its host.
     * @param pName The actor name.
     * @param pHost The host actor that receives the messages.
     */
    MessageSyncParts::MessageSyncParts(const char* pName, LiveActor* pHost)
        : LiveActor(pName), mHostActor(pHost) {}

    /**
     * @brief Forwards sensor attacks to the host.
     * @param pSelf The attacking sensor.
     * @param pOther The attacked sensor.
     */
    void MessageSyncParts::attackSensor(HitSensor* pSelf, HitSensor* pOther) {
        if (mIsSyncAttackSensor) {
            mHostActor->attackSensor(pSelf, pOther);
        }
    }

    /**
     * @brief Forwards received messages to the host.
     * @param pMsg The received message.
     * @param pSelf The receiving sensor.
     * @param pOther The sending sensor.
     * @return Whether the host handled the message.
     */
    bool MessageSyncParts::receiveMsg(const SensorMsg* pMsg, HitSensor* pSelf, HitSensor* pOther) {
        if (mIsSyncReceiveMsg) {
            return mHostActor->receiveMsg(pMsg, pSelf, pOther);
        }
        return false;
    }

    /**
     * @brief Forwards received screen point messages to the host.
     * @param pMsg The received message.
     * @param pPointer The screen pointer.
     * @param pTarget The screen point target.
     * @return Whether the host handled the message.
     */
    bool MessageSyncParts::receiveMsgScreenPoint(const SensorMsg* pMsg, ScreenPointer* pPointer,
                                                 ScreenPointTarget* pTarget) {
        if (mIsSyncReceiveScreenPoint) {
            return mHostActor->receiveMsgScreenPoint(pMsg, pPointer, pTarget);
        }
        return false;
    }
}  // namespace al
