#pragma once

#include "Library/HitSensor/HitSensorType.hpp"

namespace al {
    class LiveActor;
};

namespace alSensorFunction {
    void updateHitSensorsAll(al::LiveActor* pActor);
    void clearHitSensors(al::LiveActor* pActor);
    al::HitSensorType findSensorTypeByName(const char* pName);
};
