#pragma once

#include <math/seadVector.h>

#include "Project/AreaObj/AreaObj.hpp"

namespace al {
class ActorInitInfo;
class ClippingJudge;

/**
 * Area that groups actors for clipping and fades them in or out depending on whether the
 * observer is inside the area.
 */
class ClippingAreaActorViewArea : public AreaObj {
public:
    ClippingAreaActorViewArea(const PlacementInfo& rPlacementInfo, const ActorInitInfo& rInfo);

    f32 updateClipping(const sead::Vector3f& rPos, bool isImmediate);
    bool isClipped(ClippingJudge* pJudge) const;

    f32 getFade() const { return mFade; }

    f32 mFade;
    f32 mFadeStep;
    f32 mRadius = 0.0f;
    sead::Vector3f mCenter;
    sead::Vector3f* mBoxPoints = nullptr;
    bool mIsBoundsValid = false;
};
}  // namespace al
