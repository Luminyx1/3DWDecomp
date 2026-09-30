#pragma once

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class MessageSyncParts : public LiveActor {
public:
    MessageSyncParts(const char* pName, LiveActor* pHost);

    void attackSensor(HitSensor* pSelf, HitSensor* pOther) override;
    bool receiveMsg(const SensorMsg* pMsg, HitSensor* pOther, HitSensor* pSelf) override;
    bool receiveMsgScreenPoint(const SensorMsg* pMsg, ScreenPointer* pPointer,
                               ScreenPointTarget* pTarget) override;

    LiveActor* mHost;
    bool mIsSyncAttackSensor = true;
    bool mIsSyncReceiveMsg = true;
    bool mIsSyncReceiveMsgScreenPoint = true;
};
}  // namespace al
