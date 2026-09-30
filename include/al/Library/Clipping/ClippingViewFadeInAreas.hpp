#pragma once

#include <math/seadVector.h>

#include "Project/AreaObj/AreaObjGroup.hpp"

namespace al {
class ActorInitInfo;
class PlacementInfo;

class ClippingViewFadeInAreas : public AreaObjGroup {
public:
    ClippingViewFadeInAreas(const char* pLinkName, const PlacementInfo& rPlacementInfo,
                            const ActorInitInfo& rInfo);

    f32 updateClipping(const sead::Vector3f& rPos, bool isForce);

    f32 mRadius = 0.0f;
    sead::Vector3f mCenter;
    f32 mFadeRate;
    f32 mFadeStep;
};

class ClipForceViewArea : public AreaObjGroup {
public:
    ClipForceViewArea(const char* pLinkName, const PlacementInfo& rPlacementInfo,
                      const ActorInitInfo& rInfo);

    bool isInArea(const sead::Vector3f& rPos);
};
}  // namespace al
