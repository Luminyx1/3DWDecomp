#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Camera/CameraPoser_RS.hpp"

namespace al {
class LiveActor;
class RailKeeper;
class Resource;

class CameraPoserFixActor : public CameraPoser_RS {
public:
    CameraPoserFixActor(const LiveActor* pActor);
    void setDirectAngle();
    void setDirectAngle(sead::Vector3f& rDir);
    void setTargetActor(const LiveActor* pActor) { mTargetActor = pActor; }
    void setOffset(const sead::Vector3f& rOffset) { mOffset.set(rOffset); }
    void setDistance(f32 distance) { mDistance = distance; }
    void setAngleH(f32 angle) { mAngleH = angle; }
    void setAngleV(f32 angle) { mAngleV = angle; }

public:
    const LiveActor* mTargetActor;
    u8 _150[0x8];
    sead::Vector3f mOffset;
    f32 mDistance;
    f32 mAngleH;
    f32 mAngleV;
    bool _170;
    bool mIsCalcNearestAtFromPreAt;
    u8 _172[0x9a];
    f32 mTalkAngleH;
};

static_assert(sizeof(CameraPoserFixActor) == 0x210);

class CameraPoserFixTalk : public CameraPoserFixActor {
public:
    CameraPoserFixTalk(const LiveActor* pActor);
};

class CameraPoserFixFishing : public CameraPoserFixActor {
public:
    CameraPoserFixFishing(const LiveActor* pActor);

    void initParam(f32 angleH, const sead::Vector3f& rOffset, const sead::Vector3f& rTarget);

    u8 _210[0x18];
};

static_assert(sizeof(CameraPoserFixFishing) == 0x228);

}  // namespace al
