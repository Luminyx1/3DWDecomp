#pragma once

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class MessageSyncParts : public LiveActor {
public:
    MessageSyncParts(const char*, LiveActor*);

    void attackSensor(HitSensor* pSelf, HitSensor* pOther) override;
    bool receiveMsg(const SensorMsg* pMsg, HitSensor* pSelf, HitSensor* pOther) override;
    bool receiveMsgScreenPoint(const SensorMsg* pMsg, ScreenPointer* pPointer,
                               ScreenPointTarget* pTarget) override;

    LiveActor* mHostActor;                  // _148
    bool mIsSyncAttackSensor = true;        // _150
    bool mIsSyncReceiveMsg = true;          // _151
    bool mIsSyncReceiveScreenPoint = true;  // _152
};
}  // namespace al
