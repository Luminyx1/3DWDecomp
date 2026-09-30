#include "Library/Obj/MessageSyncParts.hpp"

namespace al {
/**
 * Constructs parts that forward sensor messages to a host.
 * @param pName actor name
 * @param pHost host actor
 */
MessageSyncParts::MessageSyncParts(const char* pName, LiveActor* pHost)
    : LiveActor(pName), mHost(pHost) {}

/**
 * Forwards sensor attacks to the host.
 * @param pSelf own sensor
 * @param pOther other sensor
 */
void MessageSyncParts::attackSensor(HitSensor* pSelf, HitSensor* pOther) {
    if (mIsSyncAttackSensor) {
        mHost->attackSensor(pSelf, pOther);
    }
}

/**
 * Forwards messages to the host.
 * @param pMsg message
 * @param pOther sender sensor
 * @param pSelf receiver sensor
 * @return whether the host handled the message
 */
bool MessageSyncParts::receiveMsg(const SensorMsg* pMsg, HitSensor* pOther, HitSensor* pSelf) {
    if (mIsSyncReceiveMsg) {
        return mHost->receiveMsg(pMsg, pOther, pSelf);
    }

    return false;
}

/**
 * Forwards screen point messages to the host.
 * @param pMsg message
 * @param pPointer screen pointer
 * @param pTarget screen point target
 * @return whether the host handled the message
 */
bool MessageSyncParts::receiveMsgScreenPoint(const SensorMsg* pMsg, ScreenPointer* pPointer,
                                             ScreenPointTarget* pTarget) {
    if (mIsSyncReceiveMsgScreenPoint) {
        return mHost->receiveMsgScreenPoint(pMsg, pPointer, pTarget);
    }

    return false;
}
}  // namespace al
