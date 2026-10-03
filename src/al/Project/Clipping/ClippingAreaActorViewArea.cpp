#include "Project/Clipping/ClippingAreaActorViewArea.hpp"

#include <cfloat>
#include <math/seadBoundBox.h>

#include "Library/Actor/ActorInitInfo.hpp"
#include "Library/Clipping/ClippingJudge.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/StageSwitch/StageSwitchFunc.hpp"
#include "Library/Thread/Functor.hpp"

namespace al {
using AreaObjFunctor = FunctorV0M<AreaObj*, void (AreaObj::*)()>;

/**
 * Creates a clipping view area and computes its bounding sphere and box.
 * @param rPlacementInfo placement info of the area
 * @param rInfo actor init info
 */
ClippingAreaActorViewArea::ClippingAreaActorViewArea(const PlacementInfo& rPlacementInfo,
                                                     const ActorInitInfo& rInfo)
    : AreaObj("ClippingAreaViewArea") {
    AreaInitInfo initInfo;
    initInfo.set(rPlacementInfo, rInfo.getStageSwitchDirector());
    init(initInfo);

    s32 fadeTime = 30;
    tryGetArg(&fadeTime, rPlacementInfo, "FadeTime");
    bool isUseBoxClipping = false;
    tryGetArg(&isUseBoxClipping, rPlacementInfo, "UseBoxClipping");
    if (isUseBoxClipping) {
        mBoxPoints = new sead::Vector3f[8];
    }

    if (fadeTime < 0) {
        fadeTime = 30;
    } else if (fadeTime == 0) {
        fadeTime = 1;
    }

    mFadeStep = 1.0f / fadeTime;
    mIsBoundsValid = true;
    mFade = 0.0f;

    AreaShape* shape = mShape;
    sead::BoundBox3f localBox;
    if (shape->calcLocalBoundingBox(&localBox)) {
        const sead::Vector3f& min = localBox.getMin();
        const sead::Vector3f& max = localBox.getMax();
        sead::Vector3f points[8] = {
            {min.x, min.y, min.z}, {max.x, min.y, min.z}, {max.x, min.y, max.z},
            {min.x, min.y, max.z}, {min.x, max.y, min.z}, {max.x, max.y, min.z},
            {max.x, max.y, max.z}, {min.x, max.y, max.z},
        };

        f32 maxX = -FLT_MAX;
        f32 maxY = -FLT_MAX;
        f32 maxZ = -FLT_MAX;
        f32 minX = FLT_MAX;
        f32 minY = FLT_MAX;
        f32 minZ = FLT_MAX;
        for (s32 i = 0; i < 8; i++) {
            sead::Vector3f& point = points[i];
            point.x *= shape->mScale.x;
            point.y *= shape->mScale.y;
            point.z *= shape->mScale.z;
            point.setMul(_28, point);
            if (mBoxPoints != nullptr) {
                mBoxPoints[i] = point;
            }

            if (point.x < minX) {
                minX = point.x;
            }

            if (point.x > maxX) {
                maxX = point.x;
            }

            if (point.y < minY) {
                minY = point.y;
            }

            if (point.y > maxY) {
                maxY = point.y;
            }

            if (point.z < minZ) {
                minZ = point.z;
            }

            if (point.z > maxZ) {
                maxZ = point.z;
            }
        }

        sead::Vector3f worldMin = {minX, minY, minZ};
        sead::Vector3f worldMax = {maxX, maxY, maxZ};
        mRadius = (worldMin - worldMax).length() * 0.5f;
        mCenter = worldMin + (worldMax - worldMin) * 0.5f;
    } else {
        mIsBoundsValid = false;
    }

    if (listenStageSwitchOnOffAppear(this, AreaObjFunctor(this, &AreaObj::validate),
                                     AreaObjFunctor(this, &AreaObj::invalidate))) {
        invalidate();
    }
}

/**
 * Updates the fade value of the area.
 * @param rPos observer position
 * @param isImmediate whether to skip the fade
 * @return the current fade value, 1 meaning fully visible
 */
f32 ClippingAreaActorViewArea::updateClipping(const sead::Vector3f& rPos, bool isImmediate) {
    if (mRadius != 0.0f && mFade == 0.0f && (rPos - mCenter).squaredLength() > mRadius * mRadius) {
        return 0.0f;
    }

    if (mIsValid && !mIsDisabled && _66 && AreaObj::isInVolume(rPos)) {
        if (isImmediate) {
            mFade = 1.0f;
        }

        mFade = mFadeStep + mFade;
        if (mFade > 1.0f) {
            mFade = 1.0f;
            return 1.0f;
        }

        return mFade;
    }

    if (isImmediate) {
        mFade = 0.0f;
    }

    mFade = mFade - mFadeStep;
    if (mFade < 0.0f) {
        mFade = 0.0f;
        return 0.0f;
    }

    return mFade;
}

/**
 * Checks whether the area is outside the view.
 * @param pJudge clipping judge
 * @return whether the area is clipped
 */
bool ClippingAreaActorViewArea::isClipped(ClippingJudge* pJudge) const {
    if (!mIsValid || mIsDisabled || !_66 || !mIsBoundsValid) {
        return false;
    }

    if (pJudge->isJudgedToClipFrustum(mCenter, mRadius, 10.0f, 1)) {
        return true;
    }

    return mBoxPoints != nullptr && pJudge->isJudgedToClipFrustum(mBoxPoints, 8, 10.0f, 1);
}
}  // namespace al
