#pragma once

#include <math/seadVector.h>

namespace al {
class LiveActor;
struct PlacementInfo;

class PlacementClippingExpander {
public:
    PlacementClippingExpander();

    void init(LiveActor*, const PlacementInfo&);

    sead::Vector3f mTrans = sead::Vector3f::zero;         // _0
    f32 mRadius = 0.0f;                                    // _c
    sead::Vector3f mClippingCenter = sead::Vector3f::zero;  // _10
    f32 mClippingRadius = 0.0f;                            // _1c
};
}  // namespace al
