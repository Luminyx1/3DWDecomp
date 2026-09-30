#include "Library/Clipping/ClippingViewFadeInAreas.hpp"

#include <cfloat>
#include <math/seadBoundBox.h>
#include <math/seadMathCalcCommon.h>

#include "Library/Actor/ActorInitInfo.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementInfo.hpp"
#include "Project/AreaObj/AreaObj.hpp"
#include "Project/AreaObj/AreaShape.hpp"

namespace al {
/**
 * Creates the fade in areas linked to a placement.
 * @param pLinkName name of the link to the areas
 * @param rPlacementInfo placement info owning the link
 * @param rInfo actor init info
 */
ClippingViewFadeInAreas::ClippingViewFadeInAreas(const char* pLinkName,
                                                 const PlacementInfo& rPlacementInfo,
                                                 const ActorInitInfo& rInfo)
    : AreaObjGroup(pLinkName) {
    AreaInitInfo areaInitInfo;
    s32 fadeTime = 30;
    tryGetArg(&fadeTime, rPlacementInfo, "FadeTime");

    if (fadeTime < 0) {
        fadeTime = 30;
    } else if (fadeTime == 0) {
        fadeTime = 1;
    }

    mFadeStep = 1.0f / fadeTime;
    mFadeRate = 0.0f;

    s32 num = calcLinkChildNum(rPlacementInfo, pLinkName);
    createBuffer(num);

    sead::BoundBox3f localBox;
    sead::Vector3f min = {FLT_MAX, FLT_MAX, FLT_MAX};
    sead::Vector3f max = {-FLT_MAX, -FLT_MAX, -FLT_MAX};
    bool isValidBox = true;

    for (s32 i = 0; i < num; i++) {
        PlacementInfo placementInfo;
        getLinksInfoByIndex(&placementInfo, rPlacementInfo, pLinkName, i);
        areaInitInfo.set(placementInfo, rInfo.mStageSwitchDirector);
        AreaObj* areaObj = new AreaObj("linkName");
        areaObj->init(areaInitInfo);
        resisterAreaObj(areaObj);

        if (!isValidBox || !areaObj->mShape->calcLocalBoundingBox(&localBox)) {
            isValidBox = false;
            continue;
        }

        const sead::Vector3f& boxMin = localBox.getMin();
        const sead::Vector3f& boxMax = localBox.getMax();
        sead::Vector3f corners[8] = {
            {boxMin.x, boxMin.y, boxMin.z}, {boxMax.x, boxMin.y, boxMin.z},
            {boxMax.x, boxMin.y, boxMax.z}, {boxMin.x, boxMin.y, boxMax.z},
            {boxMin.x, boxMax.y, boxMin.z}, {boxMax.x, boxMax.y, boxMin.z},
            {boxMax.x, boxMax.y, boxMax.z}, {boxMin.x, boxMax.y, boxMax.z},
        };

        const sead::Vector3f& scale = areaObj->mShape->mScale;

        for (s32 j = 0; j < 8; j++) {
            sead::Vector3f& corner = corners[j];
            corner.x *= scale.x;
            corner.y *= scale.y;
            corner.z *= scale.z;
            corner.setMul(areaObj->_28, corner);

            if (corner.x < min.x) {
                min.x = corner.x;
            }

            if (corner.x > max.x) {
                max.x = corner.x;
            }

            if (corner.y < min.y) {
                min.y = corner.y;
            }

            if (corner.y > max.y) {
                max.y = corner.y;
            }

            if (corner.z < min.z) {
                min.z = corner.z;
            }

            if (corner.z > max.z) {
                max.z = corner.z;
            }
        }
    }

    if (isValidBox) {
        mRadius = (min - max).length() * 0.5f;
        mCenter = min + (max - min) * 0.5f;
    }
}

/**
 * Updates the fade rate of the areas.
 * @param rPos view position
 * @param isForce whether to skip fading
 * @return the fade rate
 */
f32 ClippingViewFadeInAreas::updateClipping(const sead::Vector3f& rPos, bool isForce) {
    if (mRadius != 0.0f && mFadeRate == 0.0f &&
        (rPos - mCenter).squaredLength() > mRadius * mRadius) {
        return 0.0f;
    }

    if (getInVolumeAreaObj(rPos)) {
        if (isForce) {
            mFadeRate = 1.0f;
            return 1.0f;
        }

        mFadeRate += mFadeStep;

        if (mFadeRate > 1.0f) {
            mFadeRate = 1.0f;
            return 1.0f;
        }

        return mFadeRate;
    }

    if (isForce) {
        mFadeRate = 0.0f;
        return 0.0f;
    }

    mFadeRate -= mFadeStep;

    if (mFadeRate < 0.0f) {
        mFadeRate = 0.0f;
        return 0.0f;
    }

    return mFadeRate;
}

/**
 * Creates the force view areas linked to a placement.
 * @param pLinkName name of the link to the areas
 * @param rPlacementInfo placement info owning the link
 * @param rInfo actor init info
 */
ClipForceViewArea::ClipForceViewArea(const char* pLinkName, const PlacementInfo& rPlacementInfo,
                                     const ActorInitInfo& rInfo)
    : AreaObjGroup(pLinkName) {
    AreaInitInfo areaInitInfo;
    s32 num = calcLinkChildNum(rPlacementInfo, pLinkName);
    createBuffer(num);

    for (s32 i = 0; i < num; i++) {
        PlacementInfo placementInfo;
        getLinksInfoByIndex(&placementInfo, rPlacementInfo, pLinkName, i);
        areaInitInfo.set(placementInfo, rInfo.mStageSwitchDirector);
        AreaObj* areaObj = new AreaObj("linkName");
        areaObj->init(areaInitInfo);
        resisterAreaObj(areaObj);
    }
}

/**
 * Checks whether a position is inside any of the areas.
 * @param rPos position to check
 * @return true if an area contains the position
 */
bool ClipForceViewArea::isInArea(const sead::Vector3f& rPos) {
    for (s32 i = 0; i < mNumAreas; i++) {
        if (getAreaObj(i)->isInVolume(rPos)) {
            return true;
        }
    }

    return false;
}
}  // namespace al
