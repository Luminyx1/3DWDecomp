#pragma once

#include <math/seadVector.h>

namespace al {
class LiveActor;
struct PlacementInfo;

class PlacementClippingExpander {
public:
    PlacementClippingExpander();

    void init(LiveActor* pActor, const PlacementInfo& rInfo);

    sead::Vector3f mTrans = sead::Vector3f::zero;
    f32 mRadius = 0.0f;
    sead::Vector3f mClippingPos = sead::Vector3f::zero;
    f32 mClippingRadius = 0.0f;
};

static_assert(sizeof(PlacementClippingExpander) == 0x20);
}  // namespace al
