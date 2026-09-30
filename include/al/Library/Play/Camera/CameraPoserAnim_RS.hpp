#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Camera/CameraPoser_RS.hpp"

namespace al {
class LiveActor;
class RailKeeper;
class Resource;

class CameraPoserAnim_RS : public CameraPoser_RS {
public:
    CameraPoserAnim_RS();
    void initAnimResource(const Resource* pResource, const sead::Matrix34f* pBaseMtx);
    void setAnim(const char* pName, s32 startStep, s32 endStep, s32 playStep);
    bool isExistAnim(const char* pName) const;
    void setAnimEnd();
    s32 calcStepMax(const char* pName) const;
    bool isAnimPlaying(const char* pName) const;
    bool isAnimEnd() const;

public:
    u8 _142[0x4e];
    s32 mStepMax;
    s32 mStep;
    u8 _198[0x10];
    const sead::Matrix34f* mBaseMtxPtr;
    u8 _1b0[0x5];
    bool mIsRotateBaseUp;
    bool mIsCheckRange;
    u8 _1b7[0x1];
    sead::Vector3f mLookAtOffset;
    u8 _1c4[0x4];
};

static_assert(sizeof(CameraPoserAnim_RS) == 0x1c8);

}  // namespace al
