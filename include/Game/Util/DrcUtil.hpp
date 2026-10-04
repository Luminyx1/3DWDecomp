#pragma once

namespace al {
    class HitSensor;
    class LiveActor;
    class ScreenPointer;
};  // namespace al

namespace DrcFunction {
    al::HitSensor* tryFindDrcPlayerSensor(const al::LiveActor*, const al::ScreenPointer*);
    al::HitSensor* tryFindDrcPlayerSensor(const al::LiveActor*, const al::HitSensor*);
    al::LiveActor* tryFindDrcTouchActor(const al::LiveActor*, const al::ScreenPointer*);
};  // namespace DrcFunction
