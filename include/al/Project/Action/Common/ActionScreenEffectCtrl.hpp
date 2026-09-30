#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
class LiveActor;

struct ActionRadialBlurData {
    template <typename T>
    inline void serialize(T& rBridge) {
        rBridge.exec(&mIsEnable, "RadialBlurEnable");
        rBridge.exec(&mStartFrame, "RadialBlurStartFrame");
        rBridge.exec(&mBlurFrame, "RadialBlurBlurFrame");
        rBridge.exec(&mJointName, "RadialBlurJointName");
        rBridge.exec(&mPosOffset, "RadialBlurPosOffset");
        rBridge.exec(&mRadiusBegin, "RadialBlurRadiusBegin");
        rBridge.exec(&mRadiusEnd, "RadialBlurRadiusEnd");
    }

    bool mIsEnable = false;
    s32 mStartFrame = 0;
    s32 mBlurFrame = 1;
    const char* mJointName = nullptr;
    sead::Vector3f mPosOffset = sead::Vector3f::zero;
    f32 mRadiusBegin = 0.0f;
    f32 mRadiusEnd = 2000.0f;
};

struct ActionScreenEffectCtrlInfo {
    ActionScreenEffectCtrlInfo();

    template <typename T>
    inline void serialize(T& rBridge) {
        rBridge.exec(&mActionName, "ActionName");
        rBridge.exec(&mRadialBlur, "RadialBlur");
    }

    const char* mActionName = nullptr;
    bool mIsActive = false;
    ActionRadialBlurData mRadialBlur;
};

static_assert(sizeof(ActionScreenEffectCtrlInfo) == 0x40);

class ActionScreenEffectCtrl {
public:
    static ActionScreenEffectCtrl* tryCreate(const LiveActor* pActor);

    ActionScreenEffectCtrl(const LiveActor* pActor);
    void startAction(const char* pActionName);
    void update(f32 frame, f32 frameRate);

private:
    const LiveActor* mParentActor;
    const char* mActionName = nullptr;
    s32 mInfoCount = 0;
    ActionScreenEffectCtrlInfo* mInfos = nullptr;
};

static_assert(sizeof(ActionScreenEffectCtrl) == 0x20);
}  // namespace al
