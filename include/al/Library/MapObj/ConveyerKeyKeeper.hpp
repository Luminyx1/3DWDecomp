#pragma once

#include <math/seadQuat.h>
#include <math/seadVector.h>

namespace al {
class ActorInitInfo;
struct PlacementInfo;

struct ConveyerKey {
    f32 mMoveDistance;                      // _0
    f32 mTotalMoveDistance;                 // _4
    sead::Quatf mQuat;                      // _8
    sead::Vector3f mMoveDistanceVertical;   // _18
    s32 mInterpolateType;                   // _24
    const PlacementInfo* mPlacementInfo;    // _28
};

class ConveyerKeyKeeper {
public:
    ConveyerKeyKeeper();

    void init(const ActorInitInfo&);
    void calcPosAndQuat(sead::Vector3f*, sead::Quatf*, s32*, f32) const;
    void calcPosAndQuatByKeyIndex(sead::Vector3f*, sead::Quatf*, s32) const;
    void calcClippingSphere(sead::Vector3f*, f32*, f32) const;
    const ConveyerKey& getConveyerKey(s32) const;

    s32 getConveyerKeyCount() const { return mConveyerKeyNum; }

    f32 getTotalMoveDistance() const { return mTotalMoveDistance; }

    ConveyerKey* mConveyerKeys = nullptr;                   // _0
    s32 mConveyerKeyNum = 0;                                // _8
    sead::Quatf mQuat = sead::Quatf::unit;                  // _c
    sead::Vector3f mTrans = sead::Vector3f::zero;           // _1c
    sead::Vector3f mMoveDirection = sead::Vector3f::ez;     // _28
    f32 mTotalMoveDistance = 0.0f;                          // _34
};
}  // namespace al
