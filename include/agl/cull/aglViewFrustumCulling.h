#pragma once

#include <basis/seadTypes.h>
#include <math/seadBoundBox.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace sead {
class Projection;
}

namespace agl::cull {

class ViewFrustumCulling {
public:
    ViewFrustumCulling();
    ViewFrustumCulling& operator=(const ViewFrustumCulling& rOther);

    void update(const sead::Matrix34f& rViewMtx, const sead::Projection& rProjection);
    void update(const sead::Matrix34f& rViewMtx);
    void update(const sead::Matrix34f& rViewMtx, const sead::Matrix44f& rProjMtx, f32 near,
                f32 far, f32 fovy, f32 aspect, const sead::Vector2f& rOffset);
    void update(const sead::Matrix44f& rProjMtx, f32 near, f32 far, f32 fovy, f32 aspect,
                const sead::Vector2f& rOffset);
    void update(const sead::Projection& rProjection);
    bool isInside(const sead::Vector3f& rPos, f32 radius) const;
    bool isInside(const sead::Vector3f& rMin, const sead::Vector3f& rMax) const;

    const sead::Matrix34f& getViewMtx() const { return mViewMtx; }
    const sead::Matrix34f& getViewInvMtx() const { return mViewInvMtx; }
    const sead::Matrix44f& getProjMtx() const { return mProjMtx; }
    f32 getNear() const { return mNear; }
    f32 getFar() const { return mFar; }

    sead::Matrix34f mViewMtx;
    sead::Matrix34f mViewInvMtx;
    sead::Matrix44f mProjMtx;
    sead::Matrix44f mProjInvMtx;
    f32 mNear;
    f32 mFar;
    f32 mFovy;
    f32 mAspect;
    f32 _f0;
    f32 mTanHalfFovy;
    f32 _f8;
    sead::Vector2f mOffset;
    sead::Vector3f mSidePlaneNormal[4];
    f32 mSidePlaneDist[4];
    u8 _144[0x1a4 - 0x144];
    sead::Vector4f mPlane[6];
    u8 _204[0x21c - 0x204];
    sead::BoundBox3f mBoundBox;
    u8 _234[0x238 - 0x234];

private:
    void update_();

    f32 calcSidePlaneDist_(s32 index, const sead::Vector3f& rViewPos) const
    {
        return rViewPos.dot(mSidePlaneNormal[index]) - mSidePlaneDist[index];
    }
};
static_assert(sizeof(ViewFrustumCulling) == 0x238);

/**
 * Checks whether a sphere touches the view frustum.
 * @param rPos center of the sphere in world space
 * @param radius radius of the sphere
 * @return true if the sphere is at least partially inside
 */
inline bool ViewFrustumCulling::isInside(const sead::Vector3f& rPos, f32 radius) const
{
    f32 viewZ = mViewMtx(2, 0) * rPos.x + mViewMtx(2, 1) * rPos.y + mViewMtx(2, 2) * rPos.z +
                mViewMtx(2, 3);
    f32 depth = -viewZ;

    if (mNear - radius > depth)
    {
        return false;
    }

    if (mFar < depth - radius)
    {
        return false;
    }

    f32 viewX = mViewMtx(0, 0) * rPos.x + mViewMtx(0, 1) * rPos.y + mViewMtx(0, 2) * rPos.z +
                mViewMtx(0, 3);
    f32 viewY = mViewMtx(1, 0) * rPos.x + mViewMtx(1, 1) * rPos.y + mViewMtx(1, 2) * rPos.z +
                mViewMtx(1, 3);
    const sead::Vector3f viewPos(viewX, viewY, viewZ);

    if (!(calcSidePlaneDist_(0, viewPos) < radius))
    {
        return false;
    }

    if (!(calcSidePlaneDist_(1, viewPos) < radius))
    {
        return false;
    }

    if (!(calcSidePlaneDist_(2, viewPos) < radius))
    {
        return false;
    }

    return calcSidePlaneDist_(3, viewPos) < radius;
}

/**
 * Checks whether an axis aligned box touches the view frustum.
 * @param rMin minimum corner of the box in world space
 * @param rMax maximum corner of the box in world space
 * @return true if the box is at least partially inside
 */
inline bool ViewFrustumCulling::isInside(const sead::Vector3f& rMin,
                                         const sead::Vector3f& rMax) const
{
    const sead::Vector3f& rBoxMin = mBoundBox.getMin();
    const sead::Vector3f& rBoxMax = mBoundBox.getMax();

    if (rBoxMax.x < rMin.x || rMax.x < rBoxMin.x || rBoxMax.y < rMin.y || rMax.y < rBoxMin.y ||
        rBoxMax.z < rMin.z || rMax.z < rBoxMin.z)
    {
        return false;
    }

    for (s32 i = 0; i < 6; i++)
    {
        const sead::Vector4f& rPlane = mPlane[i];
        f32 x = rPlane.x > 0.0f ? rMin.x : rMax.x;
        f32 y = rPlane.y > 0.0f ? rMin.y : rMax.y;
        f32 z = rPlane.z > 0.0f ? rMin.z : rMax.z;

        if (rPlane.w + (rPlane.x * x + rPlane.y * y + rPlane.z * z) > 0.0f)
        {
            return false;
        }
    }

    return true;
}

}  // namespace agl::cull
