#pragma once

#include <math/seadVector.h>

#include "Library/Camera/CameraPoser_RS.hpp"

namespace al {

class CameraPoserEntrance_RS : public CameraPoser_RS {
public:
    struct Param {
        f32 angleH = 0.0f;
        f32 angleV = 30.0f;
        f32 distance = 1800.0f;
        sead::Vector3f lookAtOffset = {0.0f, 0.0f, 0.0f};
        bool isKeepInAir = false;
        s32 keepInAirCancelStep = -1;
        bool isEndIfOnGround = false;
        s32 endDelayStepIfOnGround = 0;
        s32 forceEndStep = -1;
        bool isSetLookAt = false;
        bool isFollowTarget = false;
        bool isDisableEndIfNoVelocity = false;
        sead::Vector3f lookAtPos = {0.0f, 0.0f, 0.0f};
    };

    static_assert(sizeof(Param) == 0x3c);

    // Unknown object notified when the camera ends.
    struct EndNotifier {
        u8 _0[0x68];
        bool isEnableNotify;
        bool isEnd;
    };

    CameraPoserEntrance_RS(const char* pName);

    void initParam(f32 distance, f32 angleH, f32 angleV, const sead::Vector3f& rLookAtOffset);
    void initParam(f32 distance, const sead::Vector3f& rDir, const sead::Vector3f& rLookAtOffset);
    void initLookAtPosDirect(const sead::Vector3f& rLookAtPos);
    void loadParam(const ByamlIter& rIter) override;
    void start(const CameraStartInfo& rInfo) override;
    void movement() override;
    void update() override;
    void end() override;
    void setStartPos(sead::Vector3f& rPos);

    void exeKeepByFlag();
    void exeKeepInAir();
    void exeWait();

    bool isEnableRotateByPad() const override;

public:
    Param* mParam;
    sead::Vector3f mStartTargetTrans = {0.0f, 0.0f, 0.0f};
    sead::Vector3f mTargetTrans = {0.0f, 0.0f, 0.0f};
    sead::Vector3f mWaitStartTargetTrans = {0.0f, 0.0f, 0.0f};
    s32 mStep = 0;
    s32 mValidInputStep = 0;
    EndNotifier* mEndNotifier = nullptr;
    bool mIsEndNotified = false;
    bool mIsCheckMoveDistanceH = false;
    bool mIsSetStartPos = false;
    sead::Vector3f mStartPos = sead::Vector3f::zero;
};

static_assert(sizeof(CameraPoserEntrance_RS) == 0x198);

}  // namespace al
