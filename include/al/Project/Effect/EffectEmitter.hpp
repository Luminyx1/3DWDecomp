#pragma once

#include <math/seadVector.h>

#include "Library/Effect/EmitterSetResourceInfoHolder.hpp"
#include "Library/Model/JointMtxPtr.hpp"

namespace sead::ptcl {
class Handle;
}  // namespace sead::ptcl

namespace al {
struct EffectResourceInfo;
class EffectSystemInfo;

class EffectEmitter {
public:
    EffectEmitter(const EffectSystemInfo* pSystemInfo, EffectResourceInfo* pResourceInfo, s32 handleNum);

    void initMtxPtr(JointMtxPtr mtxPtr);
    void updateMtxPtr(JointMtxPtr mtxPtr);
    void createEmitter(const JointMtxPtr* pMtxPtr, const sead::Vector3f* pPos, s32 groupId,
                       s32 forceCalcFrame, u64 userData);
    bool tryDeleteEmitter(bool isKill);
    bool tryDeleteHandle(sead::ptcl::Handle* pHandle, bool isKill);
    void setStopCalcAndDraw(bool isStop);
    void setEnableDraw(bool isEnable);
    bool isActive() const;
    bool isEnableEmit() const;
    bool isFirstFrame() const;
    void resetFirstFrame();

    sead::ptcl::Handle* getHandle() const { return mHandle; }

    EffectResourceInfo* getResourceInfo() const { return mResourceInfo; }

    inline bool isLoopOrInfinity() const;

    const JointMtxPtr& getJointMtxPtr() const { return mJointMtxPtr; }

private:
    const EffectSystemInfo* mSystemInfo;
    sead::ptcl::Handle* mHandle = nullptr;
    sead::ptcl::Handle** mHandles = nullptr;
    s32 mHandleNum = 0;
    s32 mHandleIndex = 0;
    EffectResourceInfo* mResourceInfo;
    bool mIsFirstFrame = false;
    s32 mEmitFrame = -1;
    s32 mForceCalcFrame = 0;
    JointMtxPtr mJointMtxPtr;
};

struct EffectResourceInfo {
    EffectResourceInfo();

    const char* mName;
    const char* mMaterialName;
    const char* _10;
    const char* _18;
    const char* mJointName;
    EmitterSetResourceInfo* mEmitterSetResourceInfo;
};

bool EffectEmitter::isLoopOrInfinity() const {
    return mResourceInfo->mEmitterSetResourceInfo->mIsLoop ||
           mResourceInfo->mEmitterSetResourceInfo->mIsInfinity;
}
}  // namespace al
