#pragma once

#include "Library/Nerve/NerveExecutor.hpp"

namespace al {
class ActorInitInfo;

class SwingMovement : public NerveExecutor {
public:
    SwingMovement();
    SwingMovement(const ActorInitInfo&);

    bool updateRotate();
    void exeMove();
    void exeStop();
    bool isLeft() const;
    bool isStop() const;

    f32 getCurrentAngle() const { return mCurrentAngle; }

    s32 mFrameInCycle = 0;     // _10
    s32 mDelayRate = 0;        // _14
    f32 mSwingAngle = 45.0f;   // _18
    s32 mSwingCycle = 240;     // _1c
    s32 mStopTime = 6;         // _20
    f32 mOffsetRotate = 0.0f;  // _24
    f32 mCurrentAngle = 0.0f;  // _28
};
}  // namespace al
