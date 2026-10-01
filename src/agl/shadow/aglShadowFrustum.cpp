#include "shadow/aglShadowFrustum.h"

#include <math/seadBoundBox.hpp>
#include <math/seadMathCalcCommon.h>
#include <math/seadMatrixCalcCommon.h>

#include "common/aglDrawContext.h"
#include "utility/aglDevTools.h"

namespace agl::sdw
{

namespace
{

inline void transformProj(sead::Vector3f* pOut, const sead::Matrix44f& rMtx,
                          const sead::Vector3f& rIn)
{
    const sead::Vector3f p = rIn;
    const f32 w = rMtx(3, 0) * p.x + rMtx(3, 1) * p.y + rMtx(3, 2) * p.z + rMtx(3, 3);
    pOut->x = rMtx(0, 0) / w * p.x + rMtx(0, 1) / w * p.y + rMtx(0, 2) / w * p.z + rMtx(0, 3) / w;
    pOut->y = rMtx(1, 0) / w * p.x + rMtx(1, 1) / w * p.y + rMtx(1, 2) / w * p.z + rMtx(1, 3) / w;
    pOut->z = rMtx(2, 0) / w * p.x + rMtx(2, 1) / w * p.y + rMtx(2, 2) / w * p.z + rMtx(2, 3) / w;
}

inline void transformProj(sead::Vector3f* pPoint, const sead::Matrix44f& rMtx)
{
    transformProj(pPoint, rMtx, *pPoint);
}

inline void transform(sead::Vector3f* pPoint, const sead::Matrix34f& rMtx)
{
    const sead::Vector3f p = *pPoint;
    pPoint->x = rMtx(0, 0) * p.x + rMtx(0, 1) * p.y + rMtx(0, 2) * p.z + rMtx(0, 3);
    pPoint->y = rMtx(1, 0) * p.x + rMtx(1, 1) * p.y + rMtx(1, 2) * p.z + rMtx(1, 3);
    pPoint->z = rMtx(2, 0) * p.x + rMtx(2, 1) * p.y + rMtx(2, 2) * p.z + rMtx(2, 3);
}

inline void setBoxCorners(sead::Vector3f* pPoints, const sead::BoundBox3f& rBox)
{
    const sead::Vector3f& min = rBox.getMin();
    const sead::Vector3f& max = rBox.getMax();
    pPoints[0].set(min.x, min.y, min.z);
    pPoints[1].set(max.x, min.y, min.z);
    pPoints[2].set(max.x, min.y, max.z);
    pPoints[3].set(min.x, min.y, max.z);
    pPoints[4].set(min.x, max.y, min.z);
    pPoints[5].set(max.x, max.y, min.z);
    pPoints[6].set(max.x, max.y, max.z);
    pPoints[7].set(min.x, max.y, max.z);
}

const s32 cBoxPlaneIndices[6][3] = {
    {0, 1, 5}, {3, 7, 6}, {0, 3, 2}, {5, 6, 7}, {4, 7, 3}, {1, 2, 6},
};

}  // namespace

/**
 * Constructs an empty frustum.
 */
ShadowFrustum::ShadowFrustum() : mCurrent(0), mRadius(0.0f), mEpsilon(0.05f) {}

/**
 * Allocates the polygon buffers of the working polytopes.
 * @param pHeap heap to allocate from
 */
void ShadowFrustum::initialize(sead::Heap* pHeap)
{
    mPolytopes[0].initialize(pHeap);
    mPolytopes[1].initialize(pHeap);
    mInter.initialize(pHeap);
}

/**
 * Allocates the polygons of the polytope.
 * @param pHeap heap to allocate from
 */
void ShadowFrustum::Polytope::initialize(sead::Heap* pHeap)
{
    mPolygons.tryAllocBuffer(cPolygonMax, pHeap);

    for (s32 i = 0; i < cPolygonMax; i++)
    {
        mPolygons[i] = new (pHeap) Polygon();
        mPolygons[i]->mPoints.tryAllocBuffer(cPointMax, pHeap, 0x20);
    }
}

/**
 * Clips the polytope by a view frustum.
 * @param rViewProjMtx view projection matrix of the frustum
 */
void ShadowFrustum::clipByFrustum(const sead::Matrix44f& rViewProjMtx)
{
    sead::Matrix44f inv;
    sead::Matrix44CalcCommon<f32>::inverse(inv, rViewProjMtx);
    sead::BoundBox3f box(-sead::Vector3f::ones, sead::Vector3f::ones);
    clipByBoundBox(box, inv, sead::BitFlag8(0xff));
}

/**
 * Clips the polytope by a box transformed by a projective matrix.
 * @param rBox box to clip by
 * @param rMtx matrix applied to the box corners
 * @param planeMask which box planes to clip by
 */
void ShadowFrustum::clipByBoundBox(const sead::BoundBox3f& rBox, const sead::Matrix44f& rMtx,
                                   sead::BitFlag8 planeMask)
{
    sead::Vector3f points[8];
    setBoxCorners(points, rBox);

    for (s32 i = 0; i < 8; i++)
    {
        transformProj(&points[i], rMtx);
    }

    clipByBoundBox_(points, planeMask);
}

/**
 * Clips the polytope by a box transformed by an affine matrix.
 * @param rBox box to clip by
 * @param rMtx matrix applied to the box corners
 * @param planeMask which box planes to clip by
 */
void ShadowFrustum::clipByBoundBox(const sead::BoundBox3f& rBox, const sead::Matrix34f& rMtx,
                                   sead::BitFlag8 planeMask)
{
    sead::Vector3f points[8];
    setBoxCorners(points, rBox);

    for (s32 i = 0; i < 8; i++)
    {
        transform(&points[i], rMtx);
    }

    clipByBoundBox_(points, planeMask);
}

/**
 * Clips the polytope by the planes of a hexahedron.
 * @param pPoints the eight corners of the hexahedron
 * @param planeMask which planes to clip by
 */
void ShadowFrustum::clipByBoundBox_(const sead::Vector3f* pPoints, sead::BitFlag8 planeMask)
{
    const sead::Vector3f center = (pPoints[0] + pPoints[6]) * 0.5f;
    sead::Plane3f plane(sead::Vector3f::ex, 0.0f);

    for (s32 i = 0; i < 6; i++)
    {
        if (!planeMask.isOnBit(i))
        {
            continue;
        }

        const s32* pIndices = cBoxPlaneIndices[i];
        const sead::Vector3f& p0 = pPoints[pIndices[0]];
        const sead::Vector3f& p1 = pPoints[pIndices[1]];
        const sead::Vector3f& p2 = pPoints[pIndices[2]];
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

        const f32 d = n.dot(p0);
        plane = sead::Plane3f(n, d);

        if (center.dot(n) - d > 0.0f)
        {
            plane = sead::Plane3f(-n, -d);
        }

        clipByPlane(plane);
    }
}

/**
 * Clips the polytope by a plane, keeping the part behind it.
 * @param rPlane plane to clip by
 */
void ShadowFrustum::clipByPlane(const sead::Plane3f& rPlane)
{
    mInter.mNum = 0;

    for (s32 i = 0; i < getCurrentPolytope().mNum; i++)
    {
        Polygon* pDst = getNextPolytope().birthBack();
        Polygon* pInter = mInter.birthBack();

        if (pDst == nullptr || pInter == nullptr)
        {
            break;
        }

        clipPointByPlane_(pDst, pInter, *getCurrentPolytope().mPolygons[i], rPlane);

        if (pDst->mNum == 0)
        {
            getNextPolytope().mNum--;
            mInter.mNum--;
        }
    }

    if (appendIntersectionPoint(&getNextPolytope()))
    {
        swapPolytope();
    }

    getNextPolytope().mNum = 0;
}

/**
 * Finds the frustum point nearest to the camera.
 * @param viewMtx camera view matrix
 * @return the nearest point in view space
 */
sead::Vector3f ShadowFrustum::findCameraNearPoint(sead::Matrix34f viewMtx)
{
    sead::Vector3f result(-sead::Mathf::maxNumber(), -sead::Mathf::maxNumber(),
                          -sead::Mathf::maxNumber());
    f32 num = 0.0f;
    const Polytope& polytope = getCurrentPolytope();

    for (s32 i = 0; i < polytope.mNum; i++)
    {
        const Polygon& polygon = *polytope.mPolygons[i];

        for (s32 j = 0; j < polygon.mNum; j++)
        {
            sead::Vector3f p = polygon.mPoints[j];
            transform(&p, viewMtx);
            const f32 diff = result.z - p.z;

            if (diff <= mEpsilon && diff >= -mEpsilon)
            {
                result *= num;
                num += 1.0f;
                result += p;
                result = (1.0f / num) * result;
            }
            else if (p.z > result.z)
            {
                result = p;
                num = 1.0f;
            }
        }
    }

    return result;
}

/**
 * Appends an empty polygon.
 * @return the new polygon, or null if the polytope is full
 */
ShadowFrustum::Polygon* ShadowFrustum::Polytope::birthBack()
{
    if (mNum >= cPolygonMax)
    {
        return nullptr;
    }

    Polygon* pPolygon = mPolygons[mNum];
    pPolygon->clear();
    mNum++;
    return pPolygon;
}

/**
 * Clips a polygon by a plane.
 * @param pDst receives the clipped polygon
 * @param pInter receives the intersection points
 * @param rSrc polygon to clip
 * @param rPlane plane to clip by
 */
void ShadowFrustum::clipPointByPlane_(Polygon* pDst, Polygon* pInter, const Polygon& rSrc,
                                      const sead::Plane3f& rPlane)
{
    if (rSrc.mNum < 1)
    {
        return;
    }

    f32 d0 = rPlane.getNormal().dot(rSrc.mPoints[0]) - rPlane.getD();

    for (s32 i = 0; i < rSrc.mNum; i++)
    {
        const s32 next = (i + 1) % rSrc.mNum;
        const sead::Vector3f& p0 = rSrc.mPoints[i];
        const sead::Vector3f& p1 = rSrc.mPoints[next];
        const f32 d1 = rPlane.getNormal().dot(p1) - rPlane.getD();

        if (d0 > 0.0f && d1 > 0.0f)
        {
        }
        else if (d0 <= 0.0f && d1 <= 0.0f)
        {
            pDst->pushBack(p1);
        }
        else if (d0 > 0.0f && d1 <= 0.0f)
        {
            sead::Segment<sead::Vector3f> segment(p0, p1);
            f32 t;

            if (sead::Geometry::calcIntersectionSegmentToPlane(segment, rPlane, &t) == 1)
            {
                const sead::Vector3f p = p0 + (p1 - p0) * t;
                pDst->pushBack(p);
                pInter->pushBack(p);
            }

            pDst->pushBack(p1);
        }
        else if (d0 <= 0.0f && d1 > 0.0f)
        {
            sead::Segment<sead::Vector3f> segment(p0, p1);
            f32 t;

            if (sead::Geometry::calcIntersectionSegmentToPlane(segment, rPlane, &t) == 1)
            {
                const sead::Vector3f p = p0 + (p1 - p0) * t;
                pDst->pushBack(p);
                pInter->pushBack(p);
            }
        }

        d0 = d1;
    }
}

/**
 * Builds the cap polygon from the collected intersection segments.
 * @param pDst polytope to append the cap polygon to
 * @return whether a cap polygon was appended
 */
bool ShadowFrustum::appendIntersectionPoint(Polytope* pDst)
{
    if (mInter.mNum < 3)
    {
        return false;
    }

    s32 i;

    for (i = mInter.mNum - 1; i > 0; i--)
    {
        if (mInter.mPolygons[i]->mNum == 2)
        {
            break;
        }
    }

    mInter.mNum = i + 1;

    if (mInter.mNum < 3)
    {
        return false;
    }

    Polygon* pPolygon = pDst->birthBack();

    if (pPolygon == nullptr)
    {
        return false;
    }

    const Polygon& last = *mInter.mPolygons[mInter.mNum - 1];
    pPolygon->pushBack(last.mPoints[0]);
    pPolygon->pushBack(last.mPoints[1]);
    mInter.mNum--;

    while (mInter.mNum > 0)
    {
        const s32 index = findSamePointAndSwapFromInter(pPolygon->mPoints[pPolygon->mNum - 1]);

        if (index >= 0)
        {
            pPolygon->pushBack(mInter.mPolygons[mInter.mNum - 1]->mPoints[(index + 1) % 2]);
        }

        mInter.mNum--;
    }

    pPolygon->mNum--;
    return true;
}

/**
 * Finds an intersection segment with an end at a point and moves it to the back.
 * @param rPoint point to look for
 * @return which end of the segment matched, or -1
 */
s32 ShadowFrustum::findSamePointAndSwapFromInter(const sead::Vector3f& rPoint)
{
    for (s32 i = mInter.mNum - 1; i >= 0; i--)
    {
        const Polygon& polygon = *mInter.mPolygons[i];

        if (polygon.mNum != 2)
        {
            continue;
        }

        const s32 found = findSamePointFromPolygon(polygon, rPoint);

        if (found >= 0)
        {
            mInter.swap(i, mInter.mNum - 1);
            return found;
        }
    }

    return -1;
}

/**
 * Finds a point in a polygon.
 * @param rPolygon polygon to search
 * @param rPoint point to look for
 * @return index of the point, or -1
 */
s32 ShadowFrustum::findSamePointFromPolygon(const Polygon& rPolygon, const sead::Vector3f& rPoint)
{
    for (s32 i = 0; i < rPolygon.mNum; i++)
    {
        if (isSamePoint_(rPolygon.mPoints[i], rPoint))
        {
            return i;
        }
    }

    return -1;
}

/**
 * Swaps two polygons.
 * @param a index of the first polygon
 * @param b index of the second polygon
 */
void ShadowFrustum::Polytope::swap(s32 a, s32 b)
{
    Polygon* pTmp = mPolygons[a];
    mPolygons[a] = mPolygons[b];
    mPolygons[b] = pTmp;
}

/**
 * Rebuilds the polytope as the hexahedron spanned by the frustum points.
 */
void ShadowFrustum::resetPolytope()
{
    Polygon* pPolygon = getNextPolytope().birthBack();
    pPolygon->pushBack(mPoints[0]);
    pPolygon->pushBack(mPoints[1]);
    pPolygon->pushBack(mPoints[2]);
    pPolygon->pushBack(mPoints[3]);

    pPolygon = getNextPolytope().birthBack();
    pPolygon->pushBack(mPoints[7]);
    pPolygon->pushBack(mPoints[6]);
    pPolygon->pushBack(mPoints[5]);
    pPolygon->pushBack(mPoints[4]);

    pPolygon = getNextPolytope().birthBack();
    pPolygon->pushBack(mPoints[0]);
    pPolygon->pushBack(mPoints[3]);
    pPolygon->pushBack(mPoints[7]);
    pPolygon->pushBack(mPoints[4]);

    pPolygon = getNextPolytope().birthBack();
    pPolygon->pushBack(mPoints[5]);
    pPolygon->pushBack(mPoints[6]);
    pPolygon->pushBack(mPoints[2]);
    pPolygon->pushBack(mPoints[1]);

    pPolygon = getNextPolytope().birthBack();
    pPolygon->pushBack(mPoints[6]);
    pPolygon->pushBack(mPoints[7]);
    pPolygon->pushBack(mPoints[3]);
    pPolygon->pushBack(mPoints[2]);

    pPolygon = getNextPolytope().birthBack();
    pPolygon->pushBack(mPoints[1]);
    pPolygon->pushBack(mPoints[0]);
    pPolygon->pushBack(mPoints[4]);
    pPolygon->pushBack(mPoints[5]);
    swapPolytope();
    getNextPolytope().mNum = 0;
}

/**
 * Sets the frustum points from a camera and rebuilds the polytope.
 * @param rViewMtx camera view matrix
 * @param rProjMtx camera projection matrix
 * @param near near distance
 * @param far far distance
 */
void ShadowFrustum::updateByViewFrustum(const sead::Matrix34f& rViewMtx,
                                        const sead::Matrix44f& rProjMtx, f32 near, f32 far)
{
    const f32 zNear =
        (rProjMtx(2, 3) - rProjMtx(2, 2) * near) / (rProjMtx(3, 3) - rProjMtx(3, 2) * near);
    const f32 zFar =
        (rProjMtx(2, 3) - rProjMtx(2, 2) * far) / (rProjMtx(3, 3) - rProjMtx(3, 2) * far);

    sead::Vector3f points[8] = {
        {-1.0f, -1.0f, zNear}, {1.0f, -1.0f, zNear}, {1.0f, 1.0f, zNear}, {-1.0f, 1.0f, zNear},
        {-1.0f, -1.0f, zFar},  {1.0f, -1.0f, zFar},  {1.0f, 1.0f, zFar},  {-1.0f, 1.0f, zFar},
    };

    sead::Matrix44f invProj;
    sead::Matrix44CalcCommon<f32>::inverse(invProj, rProjMtx);
    sead::Matrix34f invView;
    sead::Matrix34CalcCommon<f32>::inverse(invView, rViewMtx);

    for (s32 i = 0; i < 4; i++)
    {
        transformProj(&mPoints[i], invProj, points[i]);
        transformProj(&mPoints[i + 4], invProj, points[i + 4]);
    }

    mRadius = 0.0f;

    for (s32 i = 0; i < 8; i++)
    {
        for (s32 j = i + 1; j < 8; j++)
        {
            const f32 dist = (mPoints[i] - mPoints[j]).squaredLength();

            if (mRadius < dist)
            {
                mRadius = dist;
            }
        }
    }

    if (mRadius > 0.0f)
    {
        mRadius = sead::Mathf::sqrt(mRadius);
    }

    for (s32 i = 0; i < 4; i++)
    {
        transform(&mPoints[i], invView);
        transform(&mPoints[i + 4], invView);
    }

    resetPolytope();
}

/**
 * Expands a box by the polytope transformed by a projective matrix.
 * @param pBox box to expand
 * @param rMtx matrix applied to the points
 */
void ShadowFrustum::expandBoundBox(sead::BoundBox3f* pBox, const sead::Matrix44f& rMtx) const
{
    for (s32 i = 0; i < getCurrentPolytope().mNum; i++)
    {
        const Polygon& polygon = *getCurrentPolytope().mPolygons[i];

        for (s32 j = 0; j < polygon.mNum; j++)
        {
            sead::Vector3f p = polygon.mPoints[j];
            transformProj(&p, rMtx);
            pBox->addPoint(p);
        }
    }
}

/**
 * Expands a box by the polytope transformed into a projected view space.
 * @param pBox box to expand
 * @param rProjMtx projection matrix
 * @param rViewMtx view matrix
 */
void ShadowFrustum::expandBoundBox(sead::BoundBox3f* pBox, const sead::Matrix44f& rProjMtx,
                                   const sead::Matrix34f& rViewMtx) const
{
    for (s32 i = 0; i < getCurrentPolytope().mNum; i++)
    {
        const Polygon& polygon = *getCurrentPolytope().mPolygons[i];

        for (s32 j = 0; j < polygon.mNum; j++)
        {
            sead::Vector3f p = polygon.mPoints[j];
            transform(&p, rViewMtx);
            transformProj(&p, rProjMtx);
            pBox->addPoint(p);
        }
    }
}

/**
 * Expands a box by the polytope transformed by two projective matrices.
 * @param pBox box to expand
 * @param rMtx0 second matrix applied to the points
 * @param rMtx1 first matrix applied to the points
 */
void ShadowFrustum::expandBoundBox(sead::BoundBox3f* pBox, const sead::Matrix44f& rMtx0,
                                   const sead::Matrix44f& rMtx1) const
{
    for (s32 i = 0; i < getCurrentPolytope().mNum; i++)
    {
        const Polygon& polygon = *getCurrentPolytope().mPolygons[i];

        for (s32 j = 0; j < polygon.mNum; j++)
        {
            sead::Vector3f p = polygon.mPoints[j];
            transformProj(&p, rMtx1);
            transformProj(&p, rMtx0);
            pBox->addPoint(p);
        }
    }
}

/**
 * Expands a box by the polytope transformed by a projective and an affine matrix.
 * @param pBox box to expand
 * @param rViewMtx second matrix applied to the points
 * @param rProjMtx first matrix applied to the points
 */
void ShadowFrustum::expandBoundBox(sead::BoundBox3f* pBox, const sead::Matrix34f& rViewMtx,
                                   const sead::Matrix44f& rProjMtx) const
{
    for (s32 i = 0; i < getCurrentPolytope().mNum; i++)
    {
        const Polygon& polygon = *getCurrentPolytope().mPolygons[i];

        for (s32 j = 0; j < polygon.mNum; j++)
        {
            sead::Vector3f p = polygon.mPoints[j];
            transformProj(&p, rProjMtx);
            transform(&p, rViewMtx);
            pBox->addPoint(p);
        }
    }
}

/**
 * Expands a box by the polytope transformed by an affine matrix.
 * @param pBox box to expand
 * @param rMtx matrix applied to the points
 */
void ShadowFrustum::expandBoundBox(sead::BoundBox3f* pBox, const sead::Matrix34f& rMtx) const
{
    for (s32 i = 0; i < getCurrentPolytope().mNum; i++)
    {
        const Polygon& polygon = *getCurrentPolytope().mPolygons[i];

        for (s32 j = 0; j < polygon.mNum; j++)
        {
            sead::Vector3f p = polygon.mPoints[j];
            transform(&p, rMtx);
            pBox->addPoint(p);
        }
    }
}

/**
 * Draws the frustum edges and the polytope outline.
 * @param pDrawContext draw context
 * @param rViewMtx view matrix used for drawing
 * @param rProjMtx projection matrix used for drawing
 * @param rColor frustum edge color
 */
void ShadowFrustum::drawFrustum(DrawContext* pDrawContext, const sead::Matrix34f& rViewMtx,
                                const sead::Matrix44f& rProjMtx, const sead::Color4f& rColor) const
{
    utl::DevTools::beginDrawImm(pDrawContext, rViewMtx, rProjMtx);

    for (s32 i = 0; i < 4; i++)
    {
        utl::DevTools::drawLineImm(pDrawContext, mPoints[i], mPoints[(i + 1) % 4], rColor, 1.0f);
    }

    for (s32 i = 0; i < 4; i++)
    {
        utl::DevTools::drawLineImm(pDrawContext, mPoints[i + 4], mPoints[(i + 1) % 4 + 4], rColor,
                                   1.0f);
    }

    for (s32 i = 0; i < getCurrentPolytope().mNum; i++)
    {
        const Polygon& polygon = *getCurrentPolytope().mPolygons[i];
        const s32 num = polygon.mNum;

        if (num == 0)
        {
            continue;
        }

        sead::Vector3f prev = polygon.mPoints[num - 1];

        for (s32 j = 0; j < num; j++)
        {
            const sead::Vector3f& p = polygon.mPoints[j];
            utl::DevTools::drawLineImm(pDrawContext, prev, p, sead::Color4f::cYellow, 1.0f);
            prev = p;
        }
    }
}

/**
 * Constructs an empty polytope.
 */
ShadowFrustum::Polytope::Polytope() : mNum(0) {}

/**
 * Frees the polygons of the polytope.
 */
ShadowFrustum::Polytope::~Polytope()
{
    for (s32 i = 0; i < mPolygons.size(); i++)
    {
        if (mPolygons[i] != nullptr)
        {
            delete mPolygons[i];
        }
    }

    mPolygons.freeBuffer();
}

}  // namespace agl::sdw
