#pragma once

#include <basis/seadTypes.h>

namespace al {
class HitSensor;

class SensorHitGroup {
public:
    SensorHitGroup(s32 maxSensors, const char* pName);

    void add(HitSensor* pSensor);
    void remove(HitSensor* pSensor);
    HitSensor* getSensor(s32 idx) const;
    void clear() const;
    void executeHitCheckGroup(SensorHitGroup* pOther);
    void executeHitCheck(HitSensor* pA, HitSensor* pB);
    void executeHitCheckInSameGroup();

    s32 getSensorCount() const { return mSensorCount; }

    s32 mMaxSensors;
    s32 mSensorCount = 0;
    HitSensor** mSensors;
};
}  // namespace al
