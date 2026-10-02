#pragma once

#include <math/seadVector.h>

#include "Library/Camera/CameraPoser_RS.hpp"

namespace al {
class IntervalTrigger;
class LiveActor;

class CameraPoserSubjective_RS : public CameraPoser_RS {
public:
    CameraPoserSubjective_RS(const char* pName);

    void init() override;
    void loadParam(const ByamlIter& rIter) override;
    void start(const CameraStartInfo& rInfo) override;
    void movement() override;
    void update() override;
    void startSnapShotMode() override;
    void endSnapShotMode() override;

    void exeWait();
    void exeReset();

    static f32 getCameraOffsetFront();

    bool isZooming() const override;
    bool isEnableRotateByPad() const override;

public:
    f32 mAngleH = 0.0f;
    f32 mInputAngleH = 0.0f;
    f32 mAngleV = 0.0f;
    f32 mTargetAngleV = 0.0f;
    f32 mInputSpeedH = 0.0f;
    f32 mGyroAngleH = 0.0f;
    f32 mGyroAngleV = 0.0f;
    f32 _160 = 0.0f;
    f32 _164 = 0.0f;
    bool mIsZooming = false;
    bool mIsRequestZoomIn = false;
    bool mIsValidResetAngleH = false;
    f32 mResetStartAngleH = 0.0f;
    f32 mResetStartAngleV = 0.0f;
    f32 mMinAngleV = -30.0f;
    f32 mMaxAngleV = 75.0f;
    f32 mCameraOffsetUp = 180.0f;
    f32 mStartAngleV = 0.0f;
    bool mIsSetStartAngleH = false;
    f32 mStartAngleH = 0.0f;
    f32 mPrevAngleH = 0.0f;
    f32 mPrevAngleV = 0.0f;
    IntervalTrigger* mMoveSeTrigger = nullptr;
    LiveActor* mSeActor = nullptr;
    bool mIsSnapShotMode = false;
};

static_assert(sizeof(CameraPoserSubjective_RS) == 0x1b0);

}  // namespace al
