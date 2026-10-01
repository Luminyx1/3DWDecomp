#include "Library/Light/PrePassLineLight.hpp"

#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Math/MatrixUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Rail/RailUtil.hpp"

namespace al {

/**
 * Initializes the line light and spawns one light per additional rail segment.
 * @param rInfo Actor init info.
 */
void PrePassLineLight::init(const ActorInitInfo& rInfo) {
    PrePassLightPlacementBase::init(rInfo);

    if (!isExistRail(rInfo)) {
        return;
    }

    initRailKeeper(rInfo);
    s32 segmentNum = getRailPointNum(this) - 1;

    if (segmentNum <= 0) {
        kill();
        return;
    }

    s32 i = 0;

    do {
        sead::Vector3f begin;
        sead::Vector3f end;
        calcRailPointPos(&begin, this, i);
        calcRailPointPos(&end, this, i + 1);

        if (i == 0) {
            setByBeginEnd(begin, end);
        } else {
            PrePassLineLight* light = new PrePassLineLight("ラインライト【ライトプリパス】");
            light->PrePassLightPlacementBase::init(rInfo);
            light->setByBeginEnd(begin, end);
        }

        i++;
    } while (i < segmentNum);

    if (isLoopRail(this)) {
        sead::Vector3f begin;
        sead::Vector3f end;
        calcRailPointPos(&begin, this, segmentNum);
        calcRailPointPos(&end, this, 0);

        PrePassLineLight* light = new PrePassLineLight("ラインライト【ライトプリパス】");
        light->PrePassLightPlacementBase::init(rInfo);
        light->setByBeginEnd(begin, end);
    }
}

/**
 * Places the line light between two points.
 * @param rBegin Begin position.
 * @param rEnd End position.
 */
void PrePassLineLight::setByBeginEnd(const sead::Vector3f& rBegin, const sead::Vector3f& rEnd) {
    sead::Vector3f dir = rEnd - rBegin;
    f32 length;

    if (separateScalarAndDirection(&length, &dir, dir)) {
        kill();
        return;
    }

    mLight->mParam.mLength = length;
    sead::Matrix34f mtx;
    sead::Vector3f center = (rBegin + rEnd) * 0.5f;
    makeMtxFrontNoSupportPos(&mtx, dir, center);
    updatePoseMtx(this, &mtx);
    mLight->calcClippingInfo(&mClippingPos, &mClippingRadius);
    setClippingInfo(this, mClippingRadius, &mClippingPos);
}

}  // namespace al
