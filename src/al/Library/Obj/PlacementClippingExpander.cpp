#include "Library/Obj/PlacementClippingExpander.hpp"

#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementUtil.hpp"

namespace al {
    /**
     * @brief Constructs an empty clipping expander.
     */
    PlacementClippingExpander::PlacementClippingExpander() = default;

    /**
     * @brief Expands an actor's clipping sphere to also contain a placed sphere.
     * @param pActor The actor whose clipping is expanded.
     * @param rInfo The placement info of the sphere (its trans and x scale).
     */
    void PlacementClippingExpander::init(LiveActor* pActor, const PlacementInfo& rInfo) {
        tryGetTrans(&mTrans, rInfo);
        sead::Vector3f scale = {1.0f, 1.0f, 1.0f};
        tryGetScale(&scale, rInfo);
        mRadius = scale.x * 100.0f;
        calcSphereMargeSpheres(&mClippingCenter, &mClippingRadius, mTrans, mRadius,
                               getClippingCenterPos(pActor), getClippingRadius(pActor));
        setClippingInfo(pActor, mClippingRadius, &mClippingCenter);
    }
}  // namespace al
