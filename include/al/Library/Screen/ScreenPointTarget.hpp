#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace al {
class LiveActor;
class ScreenPointCheckGroup;
class ScreenPointer;

class ScreenPointTarget {
public:
    ScreenPointTarget(LiveActor* pHost, const char* pName, f32 radius,
                      const sead::Vector3f* pFollowPos, const sead::Matrix34f* pFollowMtx,
                      const sead::Vector3f& rOffset);

    void update();
    void validate();
    void invalidate();
    void validateBySystem();
    void invalidateBySystem();

    const char* getName() const { return mName; }
    f32 getRadius() const { return mRadius; }
    void setRadius(f32 radius) { mRadius = radius; }
    const sead::Vector3f& getOffset() const { return mOffset; }
    void setOffset(const sead::Vector3f& rOffset) { mOffset.e = rOffset.e; }
    const sead::Vector3f& getPos() const { return mPos; }
    LiveActor* getHost() const { return mHost; }
    bool isValid() const { return mIsValid && mIsValidBySystem; }
    void setCheckGroup(ScreenPointCheckGroup* pCheckGroup) { mCheckGroup = pCheckGroup; }

private:
    bool mIsValidBySystem = false;
    bool mIsValid = true;
    const char* mName;
    f32 mRadius;
    const sead::Vector3f* mFollowPos;
    const sead::Matrix34f* mFollowMtx;
    sead::Vector3f mOffset;
    sead::Vector3f mPos = sead::Vector3f::zero;
    LiveActor* mHost;
    ScreenPointCheckGroup* mCheckGroup = nullptr;
};

const sead::Vector3f& getHitScreenPointTargetPos(const ScreenPointer* pPointer);
const sead::Vector3f& getHitScreenPointTargetNormal(const ScreenPointer* pPointer);
ScreenPointTarget* getScreenPointTarget(LiveActor* pActor, const char* pName);
ScreenPointTarget* getScreenPointTarget(LiveActor* pActor, s32 index);
f32 getScreenPointTargetRadius(LiveActor* pActor, const char* pName);
const sead::Vector3f& getScreenPointTargetPos(LiveActor* pActor, const char* pName);
const sead::Vector3f& getScreenPointTargetPos(const ScreenPointTarget* pTarget);
f32 getScreenPointTargetRadius(const ScreenPointTarget* pTarget);
LiveActor* getScreenPointTargetHost(ScreenPointTarget* pTarget);
const sead::Vector3f& getScreenPointTargetOffset(LiveActor* pActor, const char* pName);
void setScreenPointTargetRadius(LiveActor* pActor, const char* pName, f32 radius);
void setScreenPointTargetOffset(LiveActor* pActor, const char* pName,
                                const sead::Vector3f& rOffset);
void validateScreenPointTargetAll(LiveActor* pActor);
void invalidateScreenPointTargetAll(LiveActor* pActor);
bool isScreenPointTargetName(const ScreenPointTarget* pTarget, const char* pName);
bool isScreenPointTargetValid(const ScreenPointTarget* pTarget);
void validateScreenPointTarget(LiveActor* pActor, const char* pName);
void invalidateScreenPointTarget(LiveActor* pActor, const char* pName);
}  // namespace al
