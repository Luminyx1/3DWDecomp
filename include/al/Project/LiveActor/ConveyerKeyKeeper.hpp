#pragma once

#include <basis/seadTypes.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>

namespace al {
class ActorInitInfo;
struct PlacementInfo;

struct ConveyerKey {
    f32 mMoveDistance;
    f32 mTotalMoveDistance;
    sead::Quatf mQuat;
    sead::Vector3f mMoveDistanceVertical;
    s32 mInterpolateType;
    const PlacementInfo* mPlacementInfo;
};

class ConveyerKeyKeeper {
public:
    ConveyerKeyKeeper();

    void init(const ActorInitInfo& rInfo);
    void calcPosAndQuat(sead::Vector3f* pPos, sead::Quatf* pQuat, s32* pIndex, f32 coord) const;
    void calcPosAndQuatByKeyIndex(sead::Vector3f* pPos, sead::Quatf* pQuat, s32 index) const;
    void calcClippingSphere(sead::Vector3f* pTrans, f32* pRadius, f32 offset) const;
    const ConveyerKey& getConveyerKey(s32 index) const;

    s32 getConveyerKeyCount() const { return mConveyerKeyCount; }

    f32 getTotalMoveDistance() const { return mTotalMoveDistance; }

private:
    ConveyerKey* mConveyerKeys = nullptr;
    s32 mConveyerKeyCount = 0;
    sead::Quatf mQuat = sead::Quatf::unit;
    sead::Vector3f mTrans = sead::Vector3f::zero;
    sead::Vector3f mMoveDirection = sead::Vector3f::ez;
    f32 mTotalMoveDistance = 0.0f;
};

static_assert(sizeof(ConveyerKeyKeeper) == 0x38);
}  // namespace al
