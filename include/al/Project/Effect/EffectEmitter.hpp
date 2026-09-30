#pragma once

#include <math/seadVector.h>

#include "Library/Model/JointMtxPtr.hpp"

namespace nn::vfx {
class Handle;
}

namespace al {
class EmitterSetResourceInfo;
struct EffectResourceInfo;
class EffectSystemInfo;

class EffectEmitter {
public:
    EffectEmitter(const EffectSystemInfo* pSystemInfo, EffectResourceInfo* pResourceInfo,
                  s32 handleNum);

    void initMtxPtr(JointMtxPtr mtxPtr);
    void updateMtxPtr(JointMtxPtr mtxPtr);
    void createEmitter(const JointMtxPtr* pMtxPtr, const sead::Vector3f* pPos, s32 groupId,
                       s32 priority, u64 userData);
    void tryDeleteEmitter(bool isKill);
    void setStopCalcAndDraw(bool isStop);
    void setEnableDraw(bool isEnable);
    bool isActive() const;
    bool isEnableEmit() const;
    bool isFirstFrame() const;
    void resetFirstFrame();

    nn::vfx::Handle* getHandle() const { return mHandle; }
    EffectResourceInfo* getResourceInfo() const { return mResourceInfo; }

    const EffectSystemInfo* mSystemInfo;
    nn::vfx::Handle* mHandle = nullptr;
    nn::vfx::Handle** mHandles = nullptr;
    s32 mHandleNum = 0;
    s32 mHandleIndex = 0;
    EffectResourceInfo* mResourceInfo;
    bool _28 = false;
    s32 _2c = -1;
    s32 _30 = -1;
    JointMtxPtr mJointMtxPtr;
};

struct EffectResourceInfo {
    EffectResourceInfo();

    const char* mName;
    u8 _8[0x18];
    const char* mJointName;
    EmitterSetResourceInfo* mEmitterSetResourceInfo;
};
}  // namespace al
