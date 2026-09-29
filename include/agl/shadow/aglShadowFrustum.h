#pragma once

#include <basis/seadTypes.h>
#include <container/seadBuffer.h>
#include <container/seadSafeArray.h>
#include <gfx/seadColor.h>
#include <math/seadBoundBox.h>
#include <math/seadGeometry.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>
#include <prim/seadBitFlag.h>

namespace sead
{
class Heap;
}

namespace agl
{

class DrawContext;

namespace sdw
{

class ShadowFrustum
{
public:
    static constexpr s32 cPointMax = 16;
    static constexpr s32 cPolygonMax = 16;

    class Polygon
    {
    public:
        Polygon() : mNum(0) {}
        ~Polygon() { mPoints.freeBuffer(); }

        void clear()
        {
            memset(mPoints.getBufferPtr(), 0, mPoints.getByteSize());
            mNum = 0;
        }

        void pushBack(const sead::Vector3f& rPoint) { mPoints[mNum++] = rPoint; }

        sead::Buffer<sead::Vector3f> mPoints;
        s32 mNum;
    };

    class Polytope
    {
    public:
        Polytope();
        ~Polytope();

        void initialize(sead::Heap* pHeap);
        Polygon* birthBack();
        void swap(s32 a, s32 b);

        sead::Buffer<Polygon*> mPolygons;
        s32 mNum;
    };

    ShadowFrustum();

    void initialize(sead::Heap* pHeap);
    void clipByFrustum(const sead::Matrix44f& rViewProjMtx);
    void clipByBoundBox(const sead::BoundBox3f& rBox, const sead::Matrix44f& rMtx,
                        sead::BitFlag8 planeMask);
    void clipByBoundBox(const sead::BoundBox3f& rBox, const sead::Matrix34f& rMtx,
                        sead::BitFlag8 planeMask);
    void clipByPlane(const sead::Plane3<f32>& rPlane);
    sead::Vector3f findCameraNearPoint(sead::Matrix34f viewMtx);
    void resetPolytope();
    void updateByViewFrustum(const sead::Matrix34f& rViewMtx, const sead::Matrix44f& rProjMtx,
                             f32 near, f32 far);
    void expandBoundBox(sead::BoundBox3f* pBox, const sead::Matrix44f& rMtx) const;
    void expandBoundBox(sead::BoundBox3f* pBox, const sead::Matrix44f& rProjMtx,
                        const sead::Matrix34f& rViewMtx) const;
    void expandBoundBox(sead::BoundBox3f* pBox, const sead::Matrix44f& rMtx0,
                        const sead::Matrix44f& rMtx1) const;
    void expandBoundBox(sead::BoundBox3f* pBox, const sead::Matrix34f& rViewMtx,
                        const sead::Matrix44f& rProjMtx) const;
    void expandBoundBox(sead::BoundBox3f* pBox, const sead::Matrix34f& rMtx) const;
    void drawFrustum(DrawContext* pDrawContext, const sead::Matrix34f& rViewMtx,
                     const sead::Matrix44f& rProjMtx, const sead::Color4f& rColor) const;

    Polytope& getCurrentPolytope() { return mPolytopes[mCurrent]; }
    const Polytope& getCurrentPolytope() const { return mPolytopes[mCurrent]; }
    Polytope& getNextPolytope() { return mPolytopes[(mCurrent + 1) & 1]; }
    void swapPolytope() { mCurrent = (mCurrent + 1) & 1; }

    const sead::Vector3f& getPoint(s32 index) const { return mPoints[index]; }
    f32 getRadius() const { return mRadius; }
    void setEpsilon(f32 epsilon) { mEpsilon = epsilon; }

private:
    void clipByBoundBox_(const sead::Vector3f* pPoints, sead::BitFlag8 planeMask);
    void clipPointByPlane_(Polygon* pDst, Polygon* pInter, const Polygon& rSrc,
                           const sead::Plane3<f32>& rPlane);
    bool appendIntersectionPoint(Polytope* pDst);
    s32 findSamePointAndSwapFromInter(const sead::Vector3f& rPoint);
    s32 findSamePointFromPolygon(const Polygon& rPolygon, const sead::Vector3f& rPoint);

    bool isSamePoint_(const sead::Vector3f& rA, const sead::Vector3f& rB) const
    {
        const f32 dx = rA.x - rB.x;
        if (dx <= mEpsilon && dx >= -mEpsilon)
        {
            const f32 dy = rA.y - rB.y;
            if (dy <= mEpsilon && dy >= -mEpsilon)
            {
                const f32 dz = rA.z - rB.z;
                if (dz <= mEpsilon && dz >= -mEpsilon)
                {
                    return true;
                }
            }
        }
        return false;
    }

    Polytope mInter;
    sead::SafeArray<Polytope, 2> mPolytopes;
    s32 mCurrent;
    sead::Vector3f mPoints[8];
    f32 mRadius;
    f32 mEpsilon;
};
static_assert(sizeof(ShadowFrustum) == 0xb8);

}  // namespace sdw
}  // namespace agl
