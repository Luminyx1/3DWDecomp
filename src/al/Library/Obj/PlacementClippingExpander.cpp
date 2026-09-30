#include "Library/Obj/PlacementClippingExpander.hpp"

#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"

namespace al {
/**
 * Constructs an empty clipping expander.
 */
PlacementClippingExpander::PlacementClippingExpander() = default;

/**
 * Expands the clipping sphere of an actor by the sphere of a placement.
 * @param pActor actor
 * @param rInfo placement whose scale gives the sphere radius
 */
void PlacementClippingExpander::init(LiveActor* pActor, const PlacementInfo& rInfo) {
    tryGetTrans(&mTrans, rInfo);
    sead::Vector3f scale = {1.0f, 1.0f, 1.0f};
    tryGetScale(&scale, rInfo);
    mRadius = scale.x * 100.0f;
    calcSphereMargeSpheres(&mClippingPos, &mClippingRadius, mTrans, mRadius,
                           getClippingCenterPos(pActor), getClippingRadius(pActor));
    setClippingInfo(pActor, mClippingRadius, &mClippingPos);
}
}  // namespace al
