#include "shadow/aglDepthShadowUnit.h"

#include <gfx/seadGraphicsContext.h>
#include <math/seadBoundBox.hpp>
#include <math/seadMathCalcCommon.h>
#include <math/seadMatrixCalcCommon.h>

#include "common/aglDrawContext.h"
#include "shadow/aglLightMatrix.h"
#include "shadow/aglShadowMathUtil.h"
#include "shadow/aglShadowUtil.h"
#include "utility/aglDevTools.h"

namespace agl::sdw
{

namespace
{

using detail::setBoxCorners;
using detail::transform;
using detail::transformProj;

const s32 cFaceIndices[6][4] = {
    {3, 7, 6, 2}, {1, 5, 4, 0}, {0, 4, 7, 3}, {2, 6, 5, 1}, {7, 4, 5, 6}, {2, 1, 0, 3},
};

inline void mergeBox(sead::BoundBox3f* pDst, const sead::BoundBox3f& rSrc)
{
    sead::Vector3f& min = const_cast<sead::Vector3f&>(pDst->getMin());
    sead::Vector3f& max = const_cast<sead::Vector3f&>(pDst->getMax());

    if (min.x > rSrc.getMin().x)
    {
        min.x = rSrc.getMin().x;
    }

    if (min.y > rSrc.getMin().y)
    {
        min.y = rSrc.getMin().y;
    }

    if (min.z > rSrc.getMin().z)
    {
        min.z = rSrc.getMin().z;
    }

    if (max.x < rSrc.getMax().x)
    {
        max.x = rSrc.getMax().x;
    }

    if (max.y < rSrc.getMax().y)
    {
        max.y = rSrc.getMax().y;
    }

    if (max.z < rSrc.getMax().z)
    {
        max.z = rSrc.getMax().z;
    }
}

inline sead::Vector3f getViewDir(const sead::Matrix34f& rViewMtx)
{
    sead::Vector3f dir;
    ShadowUtil::calcViewDir(&dir, rViewMtx);
    return dir;
}

}  // namespace

/**
 * Constructs a shadow unit with default matrices.
 */
DepthShadowUnit::DepthShadowUnit()
    : mConvSphere(&DepthShadowUnit::convBoundingToLightSpaceDirectional_),
      mConvBox(&DepthShadowUnit::convBoundingToLightSpaceDirectional_),
      mMatrixCalcType(0, "matrix_calc_type", "Matrix Calc Type", this)
{
}

/**
 * Converts a sphere into a light space box for a directional light.
 * @param pOut output box
 * @param rSphere sphere to convert
 */
void DepthShadowUnit::convBoundingToLightSpaceDirectional_(
    sead::BoundBox3f* pOut, const sead::Sphere<sead::Vector3f>& rSphere) const
{
    sead::BoundBox3f box;
    convBoundingSphereToBox(&box, rSphere);
    convBoundingToLightSpaceDirectional_(pOut, box);
}

/**
 * Converts a box into a light space box for a directional light.
 * @param pOut output box
 * @param rBox box to convert
 */
void DepthShadowUnit::convBoundingToLightSpaceDirectional_(sead::BoundBox3f* pOut,
                                                           const sead::BoundBox3f& rBox) const
{
    sead::Vector3f points[8];
    setBoxCorners(points, rBox);

    const sead::Matrix34f& m = mLightViewMtx;
    const sead::Vector3f& p0 = points[mMinIndex[0]];
    const f32 minX = p0.x * m(0, 0) + p0.y * m(0, 1) + p0.z * m(0, 2) + m(0, 3);
    const sead::Vector3f& p1 = points[mMinIndex[1]];
    const f32 minY = p1.x * m(1, 0) + p1.y * m(1, 1) + p1.z * m(1, 2) + m(1, 3);
    const sead::Vector3f& p3 = points[mMaxIndex[0]];
    const f32 maxX = m(0, 0) * p3.x + m(0, 1) * p3.y + m(0, 2) * p3.z + m(0, 3);
    const sead::Vector3f& p4 = points[mMaxIndex[1]];
    const f32 maxY = m(1, 0) * p4.x + m(1, 1) * p4.y + m(1, 2) * p4.z + m(1, 3);
    const sead::Vector3f& p2 = points[mMinIndex[2]];
    const f32 minZ = p2.x * m(2, 0) + p2.y * m(2, 1) + p2.z * m(2, 2) + m(2, 3);
    const sead::Vector3f& p5 = points[mMaxIndex[2]];
    const f32 maxZ = m(2, 0) * p5.x + m(2, 1) * p5.y + m(2, 2) * p5.z + m(2, 3);
    pOut->set(sead::Vector3f(minX, minY, minZ), sead::Vector3f(maxX, maxY, maxZ));
}

/**
 * Destroys the shadow unit.
 */
DepthShadowUnit::~DepthShadowUnit() = default;

/**
 * Allocates the frustum polytopes.
 * @param pHeap heap to allocate from
 */
void DepthShadowUnit::initialize(sead::Heap* pHeap)
{
    mFrustum.initialize(pHeap);
}

/**
 * Updates the light and camera matrices for a new frame.
 * @param rLight light
 * @param rViewMtx camera view matrix
 * @param rProjMtx camera projection matrix
 * @param near camera near distance used for the shadow
 * @param far camera far distance used for the shadow
 * @param epsilon tolerance used when merging polytope points
 */
void DepthShadowUnit::updateSceneMatrix(const LightMatrix& rLight, const sead::Matrix34f& rViewMtx,
                                        const sead::Matrix44f& rProjMtx, f32 near, f32 far,
                                        f32 epsilon)
{
    mNear = near;
    mFar = far;

    if (mMatrixCalcType.ref() == cMatrixCalcType_TexelStable)
    {
        rLight.calcLightSpace(&mLightViewMtx, &mLightProjMtx, sead::Vector3f::ez);
    }
    else
    {
        rLight.calcLightSpace(&mLightViewMtx, &mLightProjMtx, rViewMtx);
    }

    mFrustum.setEpsilon(epsilon);
    mFrustum.updateByViewFrustum(rViewMtx, rProjMtx, near, far);
    mCameraViewMtx = rViewMtx;
    mCameraProjMtx = rProjMtx;
    mIsDirectional = rLight.isDirectional();
    mClipPlaneNum = 0;

    if (mIsDirectional)
    {
        mConvBox = &DepthShadowUnit::convBoundingToLightSpaceDirectional_;
        mConvSphere = &DepthShadowUnit::convBoundingToLightSpaceDirectional_;
        updateShadowViewProjection_(mCameraViewMtx, cMatrixCalcType_Uniform, 0.0f, 0.0f,
                                    sead::Vector2i::ones);
        const sead::Vector3f lightDir = getViewDir(mLightViewMtx);
        const f32 zNear = (mCameraProjMtx(2, 3) - mCameraProjMtx(2, 2) * mNear) /
                          (mCameraProjMtx(3, 3) - mNear * mCameraProjMtx(3, 2));
        const f32 zFar = (mCameraProjMtx(2, 3) - mCameraProjMtx(2, 2) * mFar) /
                         (mCameraProjMtx(3, 3) - mCameraProjMtx(3, 2) * mFar);
        addCasterClipPlanes_(mCameraViewMtx, mCameraProjMtx, &lightDir, zNear, zFar, true);
        updateDirectionalBoundingIndex_();
    }
    else
    {
        mConvBox = &DepthShadowUnit::convBoundingToLightSpaceNotDirectional_;
        mConvSphere = &DepthShadowUnit::convBoundingToLightSpaceNotDirectional_;
        mShadowViewMtx = mLightViewMtx;
        mShadowProjMtx = mLightProjMtx;
        addCasterClipPlanes_(mShadowViewMtx, mShadowProjMtx, nullptr, -1.0f, 1.0f, false);
    }

    for (s32 i = 0; i < 3; i++)
    {
        mWorldBox[i].setUndef();
        mLightBox[i].setUndef();
    }
}

/**
 * Fits the shadow view projection to the frustum polytope.
 * @param rViewMtx camera view matrix
 * @param type how to compute the projection
 * @param param0 first perspective parameter
 * @param param1 second perspective parameter
 * @param rSize shadow map size
 */
void DepthShadowUnit::updateShadowViewProjection_(const sead::Matrix34f& rViewMtx,
                                                  MatrixCalcType type, f32 param0, f32 param1,
                                                  const sead::Vector2i& rSize)
{
    sead::Matrix44f projMtx = mLightProjMtx;

    if (type == cMatrixCalcType_LiSPSM)
    {
        sead::Matrix44f lispsm;

        if (calcLightSpacePerspectiveMatrix_(&lispsm, projMtx, mLightViewMtx, rViewMtx, param0,
                                             param1))
        {
            detail::multiplyMtx44(projMtx, lispsm, projMtx);
        }
    }

    sead::BoundBox3f box;
    mFrustum.expandBoundBox(&box, projMtx, mLightViewMtx);

    if (box.isUndef())
    {
        return;
    }

    const f32 halfWidth = rSize.x * 0.5f;
    const f32 halfHeight = rSize.y * 0.5f;
    f32 minX = box.getMin().x;
    f32 minY = box.getMin().y;
    f32 maxX = box.getMax().x;
    f32 maxY = box.getMax().y;
    f32 width;
    f32 height;

    if (type == cMatrixCalcType_TexelStable && mIsDirectional)
    {
        const f32 sizeX = maxX - minX;
        const f32 radius = mFrustum.getRadius() * 0.5f;
        const f32 centerX = (minX + maxX) * 0.5f;
        const f32 left = centerX - radius;
        const f32 right = centerX + radius;
        const f32 sizeY = maxY - minY;
        const f32 centerY = (minY + maxY) * 0.5f;
        const f32 bottom = centerY - radius;
        const f32 top = centerY + radius;
        const f32 fitWidth = right - left;
        const f32 fitHeight = top - bottom;
        mTexOffset.x = (minX - left) / fitWidth;
        mTexOffset.y = (minY - bottom) / fitHeight;
        mTexScale.x = sizeX / fitWidth;
        mTexScale.y = sizeY / fitHeight;
        minX = left;
        maxX = right;
        minY = bottom;
        maxY = top;
        width = fitWidth;
        height = fitHeight;
    }
    else
    {
        width = maxX - minX;
        height = maxY - minY;
        mTexOffset = sead::Vector2f(0.0f, 0.0f);
        mTexScale = sead::Vector2f(1.0f, 1.0f);
    }

    const f32 minZ = box.getMin().z;
    const f32 maxZ = box.getMax().z;
    const f32 depth = maxZ - minZ;
    sead::Matrix44f fitMtx;
    fitMtx(0, 0) = 2.0f / width;
    fitMtx(0, 1) = 0.0f;
    fitMtx(0, 2) = 0.0f;
    fitMtx(0, 3) = -maxX / width - minX / width;
    fitMtx(1, 0) = 0.0f;
    fitMtx(1, 1) = 2.0f / height;
    fitMtx(1, 2) = 0.0f;
    fitMtx(1, 3) = -maxY / height - minY / height;
    fitMtx(2, 0) = 0.0f;
    fitMtx(2, 1) = 0.0f;
    fitMtx(2, 2) = 2.0f / depth;
    fitMtx(2, 3) = -maxZ / depth - minZ / depth;
    fitMtx(3, 0) = 0.0f;
    fitMtx(3, 1) = 0.0f;
    fitMtx(3, 2) = 0.0f;
    fitMtx(3, 3) = 1.0f;

    if (type == cMatrixCalcType_TexelStable && mIsDirectional)
    {
        fitMtx(0, 3) = adjustTexelStable(fitMtx(0, 3), halfWidth);
        fitMtx(1, 3) = adjustTexelStable(fitMtx(1, 3), halfHeight);
    }

    detail::multiplyMtx44(projMtx, fitMtx, projMtx);
    mShadowViewMtx = mLightViewMtx;
    mShadowProjMtx = projMtx;
}

/**
 * Adds the clip planes of a view frustum that can contain shadow casters.
 * @param rViewMtx view matrix of the frustum
 * @param rProjMtx projection matrix of the frustum
 * @param pSweepDir light direction to sweep the planes along, or null
 * @param zNear normalized depth of the near plane
 * @param zFar normalized depth of the far plane
 * @param sweep whether to add the swept planes
 */
void DepthShadowUnit::addCasterClipPlanes_(const sead::Matrix34f& rViewMtx,
                                           const sead::Matrix44f& rProjMtx,
                                           const sead::Vector3f* pSweepDir, f32 zNear, f32 zFar,
                                           bool sweep)
{
    sead::Matrix44f invProj;
    sead::Matrix44CalcCommon<f32>::inverse(invProj, rProjMtx);
    sead::Matrix34f invView;
    sead::Matrix34CalcCommon<f32>::inverse(invView, rViewMtx);

    sead::Vector3f points[8] = {
        {-1.0f, -1.0f, zFar}, {1.0f, -1.0f, zFar}, {1.0f, -1.0f, zNear}, {-1.0f, -1.0f, zNear},
        {-1.0f, 1.0f, zFar},  {1.0f, 1.0f, zFar},  {1.0f, 1.0f, zNear},  {-1.0f, 1.0f, zNear},
    };

    for (s32 i = 0; i < 8; i++)
    {
        transformProj(&points[i], invProj);
        transform(&points[i], invView);
    }

    const sead::Vector3f center = (points[0] + points[6]) * 0.5f;
    const bool doSweep = pSweepDir != nullptr && sweep;

    for (s32 i = 0; i < 6; i++)
    {
        const s32* pIndices = cFaceIndices[i];
        const sead::Vector3f& p0 = points[pIndices[0]];
        const sead::Vector3f& p1 = points[pIndices[1]];
        const sead::Vector3f& p2 = points[pIndices[2]];
        sead::Vector3f a = p2 - p0;
        a.normalize();
        sead::Vector3f b = p1 - p0;
        b.normalize();
        sead::Vector3f n;
        n.setCross(a, b);
        const f32 len = n.normalize();
        const f32 sqLen = n.squaredLength();

        if (!(len > 0.0f) || sead::Mathf::isNan(len))
        {
            continue;
        }

        const f32 diff = sqLen - 1.0f;

        if (diff < -1.1920929e-06f || diff > 1.1920929e-06f)
        {
            continue;
        }

        sead::Plane3f& plane = mClipPlanes[mClipPlaneNum];
        const f32 d = p0.dot(n);
        plane = sead::Plane3f(n, d);

        if (center.dot(n) - d > 0.0f)
        {
            plane = sead::Plane3f(-n, -d);
        }

        if (pSweepDir == nullptr || pSweepDir->dot(plane.getNormal()) > -0.001f)
        {
            addCasterClipPlane_(plane);
        }

        if (doSweep)
        {
            addCasterClipPlanesSweepDir_(*pSweepDir, points, i, plane.getNormal());
        }
    }
}

/**
 * Finds the corners of a unit box that are extreme along each light space axis.
 */
void DepthShadowUnit::updateDirectionalBoundingIndex_()
{
    for (s32 i = 0; i < 3; i++)
    {
        mMinIndex[i] = -1;
        mMaxIndex[i] = -1;
    }

    sead::Vector3f points[8];
    setBoxCorners(points, sead::BoundBox3f(-sead::Vector3f::ones, sead::Vector3f::ones));

    sead::BoundBox3f box;

    for (s32 i = 0; i < 8; i++)
    {
        transform(&points[i], mLightViewMtx);
        box.addPoint(points[i]);

        if (points[i].x == box.getMin().x)
        {
            mMinIndex[0] = i;
        }

        if (points[i].y == box.getMin().y)
        {
            mMinIndex[1] = i;
        }

        if (points[i].z == box.getMin().z)
        {
            mMinIndex[2] = i;
        }

        if (points[i].x == box.getMax().x)
        {
            mMaxIndex[0] = i;
        }

        if (points[i].y == box.getMax().y)
        {
            mMaxIndex[1] = i;
        }

        if (points[i].z == box.getMax().z)
        {
            mMaxIndex[2] = i;
        }
    }
}

/**
 * Converts a box into a light space box for a perspective light.
 * @param pOut output box
 * @param rBox box to convert
 */
void DepthShadowUnit::convBoundingToLightSpaceNotDirectional_(sead::BoundBox3f* pOut,
                                                              const sead::BoundBox3f& rBox) const
{
    sead::Matrix44f viewProj;
    sead::Matrix44CalcCommon<f32>::multiply(viewProj, mLightProjMtx, mLightViewMtx);
    sead::Vector3f points[8];
    setBoxCorners(points, rBox);
    pOut->setUndef();

    for (s32 i = 0; i < 8; i++)
    {
        transformProj(&points[i], viewProj);
        points[i].x = sead::Mathf::clamp(points[i].x, -1.0f, 1.0f);
        points[i].y = sead::Mathf::clamp(points[i].y, -1.0f, 1.0f);
        points[i].z = sead::Mathf::clamp(points[i].z, -1.0f, 1.0f);
        pOut->addPoint(points[i]);
    }
}

/**
 * Converts a sphere into a light space box for a perspective light.
 * @param pOut output box
 * @param rSphere sphere to convert
 */
void DepthShadowUnit::convBoundingToLightSpaceNotDirectional_(
    sead::BoundBox3f* pOut, const sead::Sphere<sead::Vector3f>& rSphere) const
{
    sead::BoundBox3f box;
    convBoundingSphereToBox(&box, rSphere);
    convBoundingToLightSpaceNotDirectional_(pOut, box);
}

/**
 * Computes the bounding box of a sphere.
 * @param pBox output box
 * @param rSphere sphere
 */
void DepthShadowUnit::convBoundingSphereToBox(sead::BoundBox3f* pBox,
                                              const sead::Sphere<sead::Vector3f>& rSphere) const
{
    const sead::Vector3f& center = rSphere.getCenter();
    const f32 radius = rSphere.getRadius();
    const sead::Vector3f min(center.x - radius, center.y - radius, center.z - radius);
    const sead::Vector3f max(radius + center.x, radius + center.y, radius + center.z);
    pBox->set(min, max);
}

/**
 * Clips the frustum polytope by a plane, keeping the part in front of it.
 * @param rPlane plane to clip by
 */
void DepthShadowUnit::clipByPlane(const sead::Plane3f& rPlane)
{
    mFrustum.clipByPlane(sead::Plane3f(-rPlane.getNormal(), -rPlane.getD()));
}

/**
 * Rebuilds the caster clip planes of a directional light.
 */
void DepthShadowUnit::updatePlaneClipInfo()
{
    if (!mIsDirectional)
    {
        return;
    }

    updateShadowViewProjection_(mCameraViewMtx, cMatrixCalcType_Uniform, 0.0f, 0.0f,
                                sead::Vector2i::ones);
    const sead::Vector3f lightDir = getViewDir(mLightViewMtx);
    addCasterClipPlanes_(mShadowViewMtx, mShadowProjMtx, &lightDir, -1.0f, 1.0f, false);
}

/**
 * Computes the shadow matrices from the registered bounding boxes.
 * @param width shadow map width
 * @param height shadow map height
 * @param param0 first perspective parameter
 * @param param1 second perspective parameter
 */
void DepthShadowUnit::updateShadowMatrix(s32 width, s32 height, f32 param0, f32 param1)
{
    sead::Matrix44f lightViewProj;
    sead::Matrix44CalcCommon<f32>::multiply(lightViewProj, mLightProjMtx, mLightViewMtx);

    mTotalBox.setUndef();

    for (s32 i = 0; i < 3; i++)
    {
        if (!mWorldBox[i].isUndef())
        {
            sead::BoundBox3f box;
            convBoundingToLightSpace(&box, mWorldBox[i]);
            mergeBox(&mLightBox[i], box);
        }

        mergeBox(&mTotalBox, mLightBox[i]);
    }

    if (!mTotalBox.isUndef())
    {
        if (mIsDirectional)
        {
            sead::Matrix34f inv;
            sead::Matrix34CalcCommon<f32>::inverse(inv, mLightViewMtx);
            mFrustum.clipByBoundBox(mTotalBox, inv, sead::BitFlag8(0xfe));
        }
        else
        {
            sead::Matrix44f inv;
            sead::Matrix44CalcCommon<f32>::multiply(inv, mLightProjMtx, mLightViewMtx);
            sead::Matrix44CalcCommon<f32>::inverse(inv, inv);
            mFrustum.clipByBoundBox(mTotalBox, inv, sead::BitFlag8(0xff));
        }
    }

    if (mFrustum.getCurrentPolytope().mNum <= 0)
    {
        mFrustum.resetPolytope();
    }

    if (!mIsDirectional)
    {
        mFrustum.clipByFrustum(lightViewProj);
    }

    updateShadowViewProjection_(mCameraViewMtx, MatrixCalcType(mMatrixCalcType.ref()), param0,
                                param1, sead::Vector2i(width, height));

    static const sead::Matrix44f cBiasMtx(0.5f, 0.0f, 0.0f, 0.5f, 0.0f, -0.5f, 0.0f, 0.5f, 0.0f,
                                          0.0f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f);
    sead::Matrix44f viewProj;
    sead::Matrix44CalcCommon<f32>::multiply(viewProj, mShadowProjMtx, mShadowViewMtx);
    detail::multiplyMtx44(mTexMtx, cBiasMtx, viewProj);
}

/**
 * Snaps a value to the texel grid.
 * @param value value to snap
 * @param size number of texels per unit
 * @return the snapped value
 */
f32 DepthShadowUnit::adjustTexelStable(f32 value, f32 size) const
{
    return sead::Mathf::floor(value * size) / size;
}

/**
 * Adds a caster clip plane unless an equivalent plane exists.
 * @param rPlane plane to add
 * @return whether the plane was added
 */
bool DepthShadowUnit::addCasterClipPlane_(const sead::Plane3f& rPlane)
{
    sead::Plane3f* pSame = findSameClipPlane_(rPlane.getNormal());

    if (pSame)
    {
        pSame->setD(sead::Mathf::max(rPlane.getD(), pSame->getD()));
        return false;
    }

    mClipPlanes[mClipPlaneNum] = rPlane;
    mClipPlaneNum++;
    return true;
}

/**
 * Adds the plane swept along the light direction from a frustum face edge.
 * @param rDir light direction
 * @param pPoints frustum corners
 * @param face frustum face index
 * @param rNormal normal of the frustum face
 */
void DepthShadowUnit::addCasterClipPlanesSweepDir_(const sead::Vector3f& rDir,
                                                   const sead::Vector3f* pPoints, s32 face,
                                                   const sead::Vector3f& rNormal)
{
    sead::Vector3f a;
    a.setCross(rDir, rNormal);
    a.normalize();
    sead::Vector3f n;
    n.setCross(a, rDir);
    const f32 len = n.normalize();
    const f32 sqLen = n.squaredLength();

    if (!(len > 0.0f) || sead::Mathf::isNan(len))
    {
        return;
    }

    const f32 diff = sqLen - 1.0f;

    if (diff < -1.1920929e-06f || diff > 1.1920929e-06f)
    {
        return;
    }

    sead::Plane3f& plane = mClipPlanes[mClipPlaneNum];
    const s32* pIndices = cFaceIndices[face];
    f32 d = n.dot(pPoints[pIndices[0]]);
    plane = sead::Plane3f(n, d);

    for (s32 i = 1; i < 4; i++)
    {
        const f32 dist = n.dot(pPoints[pIndices[i]]);

        if (dist - d > 0.0f)
        {
            d = dist;
            plane = sead::Plane3f(n, dist);
        }
    }

    for (s32 i = 0; i < 8; i++)
    {
        if (n.dot(pPoints[i]) - d > 0.0f)
        {
            return;
        }
    }

    addCasterClipPlane_(plane);
}

/**
 * Draws the light frustum, the shadow frustum and the caster bounds.
 * @param pDrawContext draw context
 * @param rLight light
 * @param rViewMtx view matrix used for drawing
 * @param rProjMtx projection matrix used for drawing
 */
void DepthShadowUnit::drawDebug(DrawContext* pDrawContext, const LightMatrix& rLight,
                                const sead::Matrix34f& rViewMtx,
                                const sead::Matrix44f& rProjMtx) const
{
    if (!mIsDrawDebug)
    {
        return;
    }

    sead::GraphicsContext context;
    context.setCullingMode(2);
    context.setBlendEnable(false);
    context.setAlphaTestEnable(false);
    context.setDepthEnable(false, true);
    context.apply(pDrawContext);

    if (!mIsDirectional)
    {
        ShadowUtil::drawFrustum(pDrawContext, mLightViewMtx, mLightProjMtx, rViewMtx, rProjMtx,
                                sead::Color4f::cRed);
    }

    ShadowUtil::drawFrustum(pDrawContext, mShadowViewMtx, mShadowProjMtx, rViewMtx, rProjMtx,
                            sead::Color4f::cBlack);

    if (!mTotalBox.isUndef())
    {
        sead::Vector3f points[8];
        setBoxCorners(points, mTotalBox);

        if (mIsDirectional)
        {
            sead::Matrix34f inv;
            sead::Matrix34CalcCommon<f32>::inverse(inv, mLightViewMtx);

            for (s32 i = 0; i < 8; i++)
            {
                transform(&points[i], inv);
            }
        }
        else
        {
            sead::Matrix44f inv;
            sead::Matrix44CalcCommon<f32>::multiply(inv, mLightProjMtx, mLightViewMtx);
            sead::Matrix44CalcCommon<f32>::inverse(inv, inv);

            for (s32 i = 0; i < 8; i++)
            {
                transformProj(&points[i], inv);
            }
        }

        utl::DevTools::drawLineImm(pDrawContext, points[0], points[1], sead::Color4f::cWhite, 1.0f);
        utl::DevTools::drawLineImm(pDrawContext, points[4], points[5], sead::Color4f::cWhite, 1.0f);
        utl::DevTools::drawLineImm(pDrawContext, points[0], points[4], sead::Color4f::cWhite, 1.0f);
        utl::DevTools::drawLineImm(pDrawContext, points[1], points[5], sead::Color4f::cWhite, 1.0f);
        utl::DevTools::drawLineImm(pDrawContext, points[2], points[3], sead::Color4f::cWhite, 1.0f);
        utl::DevTools::drawLineImm(pDrawContext, points[6], points[7], sead::Color4f::cWhite, 1.0f);
        utl::DevTools::drawLineImm(pDrawContext, points[2], points[6], sead::Color4f::cWhite, 1.0f);
        utl::DevTools::drawLineImm(pDrawContext, points[3], points[7], sead::Color4f::cWhite, 1.0f);
        utl::DevTools::drawLineImm(pDrawContext, points[0], points[3], sead::Color4f::cWhite, 1.0f);
        utl::DevTools::drawLineImm(pDrawContext, points[1], points[2], sead::Color4f::cWhite, 1.0f);
        utl::DevTools::drawLineImm(pDrawContext, points[4], points[7], sead::Color4f::cWhite, 1.0f);
        utl::DevTools::drawLineImm(pDrawContext, points[5], points[6], sead::Color4f::cWhite, 1.0f);
    }

    mFrustum.drawFrustum(pDrawContext, rViewMtx, rProjMtx, sead::Color4f::cBlue);
}

/**
 * Generates the host IO message (empty in release builds).
 * @param pContext host IO context
 */
void DepthShadowUnit::genMessage(sead::hostio::Context* pContext) {}

/**
 * Generates the parameter host IO message (empty in release builds).
 * @param pContext host IO context
 * @param pNode parent node
 */
void DepthShadowUnit::genMessageParameter(sead::hostio::Context* pContext,
                                          sead::hostio::Node* pNode)
{
}

/**
 * Generates the debug parameter host IO message (empty in release builds).
 * @param pContext host IO context
 * @param pNode parent node
 */
void DepthShadowUnit::genMessageDebugParameter(sead::hostio::Context* pContext,
                                               sead::hostio::Node* pNode)
{
}

/**
 * Handles a host IO property event (empty in release builds).
 * @param pEvent property event
 */
void DepthShadowUnit::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent) {}

/**
 * Handles a parameter host IO property event (empty in release builds).
 * @param pEvent property event
 */
void DepthShadowUnit::listenPropertyEventParameter(const sead::hostio::PropertyEvent* pEvent) {}

/**
 * Handles a debug parameter host IO property event (empty in release builds).
 * @param pEvent property event
 */
void DepthShadowUnit::listenPropertyEventDebugParameter(const sead::hostio::PropertyEvent* pEvent)
{
}

}  // namespace agl::sdw
