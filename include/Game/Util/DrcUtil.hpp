#pragma once

namespace al {
    class HitSensor;
    class LiveActor;
    class ScreenPointer;
};  // namespace al

namespace rc {
    bool tryFindDrcTouchActor(const al::LiveActor*, const al::ScreenPointer*);
};

namespace DrcFunction {
    al::HitSensor* tryFindDrcPlayerSensor(const al::LiveActor*, const al::ScreenPointer*);
};  // namespace DrcFunction
