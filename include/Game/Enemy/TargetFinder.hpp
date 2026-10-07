#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
    class LiveActor;
};

class TargetFinderParam {
public:
    TargetFinderParam();
    TargetFinderParam(f32, f32, f32, u32, f32, f32, f32, f32, bool);

    f32 _0;
    f32 _4;
    f32 _8;
    u32 _C;
    f32 _10;
    f32 _14;
    f32 _18;
    f32 _1C;
    bool _20;
};

class TargetFinder {
public:
    TargetFinder(al::LiveActor*, const TargetFinderParam*);

    void refindTarget();
    void update();
    void setFrontDir(sead::Vector3f* pFrontDir);
    const sead::Vector3f& getTargetPos() const;

    bool isExistTarget() const { return _8 != nullptr; }

    al::LiveActor* mActor;  // 0x00
    al::LiveActor* _8;
    u64 _10;
    sead::Vector3f* mFrontDir;      // 0x18
    sead::Vector3f* mSupportUpDir;  // 0x20
    u64 _28;
    TargetFinderParam* mParams;  // 0x30
    al::LiveActor* _38;
};