#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace al {
class LiveActor;
class ScreenPointCheckGroup;
class ScreenPointer;

class ScreenPointTarget {
public:
    ScreenPointTarget(LiveActor*, const char*, f32, const sead::Vector3f*, const sead::Matrix34f*,
                      const sead::Vector3f&);

    void update();
    void validate();
    void invalidate();
    void validateBySystem();
    void invalidateBySystem();

    bool mIsValidBySystem = false;                       // _0
    bool mIsValid = true;                                // _1
    const char* mName;                                   // _8
    f32 mRadius;                                         // _10
    const sead::Vector3f* mTrans;                        // _18
    const sead::Matrix34f* mJointMtx;                    // _20
    sead::Vector3f mOffset;                              // _28
    sead::Vector3f mPos = sead::Vector3f::zero;          // _34
    LiveActor* mActor;                                   // _40
    ScreenPointCheckGroup* mCheckGroup = nullptr;        // _48
};

static_assert(sizeof(ScreenPointTarget) == 0x50);

const sead::Vector3f& getHitScreenPointTargetPos(const ScreenPointer*);
const sead::Vector3f& getHitScreenPointTargetNormal(const ScreenPointer*);
ScreenPointTarget* getScreenPointTarget(LiveActor*, const char*);
ScreenPointTarget* getScreenPointTarget(LiveActor*, s32);
f32 getScreenPointTargetRadius(LiveActor*, const char*);
const sead::Vector3f& getScreenPointTargetPos(LiveActor*, const char*);
const sead::Vector3f& getScreenPointTargetPos(const ScreenPointTarget*);
f32 getScreenPointTargetRadius(const ScreenPointTarget*);
LiveActor* getScreenPointTargetHost(ScreenPointTarget*);
const sead::Vector3f& getScreenPointTargetOffset(LiveActor*, const char*);
void setScreenPointTargetRadius(LiveActor*, const char*, f32);
void setScreenPointTargetOffset(LiveActor*, const char*, const sead::Vector3f&);
void validateScreenPointTargetAll(LiveActor*);
void invalidateScreenPointTargetAll(LiveActor*);
bool isScreenPointTargetName(const ScreenPointTarget*, const char*);
bool isScreenPointTargetValid(const ScreenPointTarget*);
void validateScreenPointTarget(LiveActor*, const char*);
void invalidateScreenPointTarget(LiveActor*, const char*);
}  // namespace al
