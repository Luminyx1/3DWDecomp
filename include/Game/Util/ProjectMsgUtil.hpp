#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
    class HitSensor;
    class SensorMsg;
    class ComboCounter;
};  // namespace al

namespace rc {
    al::ComboCounter* tryGetMsgComboCount(const al::SensorMsg*);

    bool isMsgAskControlUserId(const al::SensorMsg*, int);
    bool sendMsgAskControlUserId(al::HitSensor*, int);

    bool trySendMsgBlockToUpperObj(al::HitSensor*, al::HitSensor*, int, al::ComboCounter*);
    bool trySendMsgBlockToLowerObj(al::HitSensor*, al::HitSensor*, al::ComboCounter*);

    bool isMsgJumpPanelAction(const al::SensorMsg*);
    bool isMsgPackunEat(const al::SensorMsg*);
    bool isMsgBullAttack(const al::SensorMsg*);
    bool isMsgStartGoalDemoPole(const al::SensorMsg*);
    bool isMsgStartGoalDemoHouse(const al::SensorMsg*);
    bool sendMsgDebugMovePosition(al::HitSensor* pReceiver, al::HitSensor* pSender,
                                  const sead::Vector3f& rPos);
};  // namespace rc