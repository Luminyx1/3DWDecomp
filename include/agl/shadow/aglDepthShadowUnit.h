#pragma once

#include <container/seadSafeArray.h>
#include <gfx/seadColor.h>
#include <hostio/seadHostIONode.h>
#include <math/seadBoundBox.h>
#include <math/seadGeometry.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "shadow/aglShadowFrustum.h"
#include "utility/aglParameter.h"
#include "utility/aglParameterObj.h"

namespace sead
{
class Heap;
namespace hostio
{
class Context;
class PropertyEvent;
}  // namespace hostio
}  // namespace sead

namespace agl
{

class DrawContext;

namespace sdw
{

class LightMatrix;

enum MatrixCalcType
{
    cMatrixCalcType_LiSPSM = 0,
    cMatrixCalcType_Uniform = 1,
    cMatrixCalcType_TexelStable = 2,
};

class DepthShadowUnit : public utl::IParameterObj, public sead::hostio::Node
{
    friend class DepthShadow;

public:
    using ConvBoxFunc = void (DepthShadowUnit::*)(sead::BoundBox3f*, const sead::BoundBox3f&) const;
    using ConvSphereFunc = void (DepthShadowUnit::*)(sead::BoundBox3f*,
                                                     const sead::Sphere<sead::Vector3f>&) const;

    struct ClipPlane : public sead::Plane3<f32>
    {
        ClipPlane() : sead::Plane3<f32>(sead::Vector3f::ex, 0.0f) {}

        ClipPlane& operator=(const sead::Plane3<f32>& rOther)
        {
            sead::Plane3<f32>::operator=(rOther);
            return *this;
        }
    };

    static constexpr s32 cClipPlaneMax = 24;

    DepthShadowUnit();
    ~DepthShadowUnit() override;

    void initialize(sead::Heap* pHeap);
    void updateSceneMatrix(const LightMatrix& rLight, const sead::Matrix34f& rViewMtx,
                           const sead::Matrix44f& rProjMtx, f32 near, f32 far, f32 epsilon);
    void convBoundingSphereToBox(sead::BoundBox3f* pBox,
                                 const sead::Sphere<sead::Vector3f>& rSphere) const;
    void clipByPlane(const sead::Plane3<f32>& rPlane);
    void updatePlaneClipInfo();
    void updateShadowMatrix(s32 width, s32 height, f32 param0, f32 param1);
    f32 adjustTexelStable(f32 value, f32 size) const;
    void drawDebug(DrawContext* pDrawContext, const LightMatrix& rLight,
                   const sead::Matrix34f& rViewMtx, const sead::Matrix44f& rProjMtx) const;

    void genMessage(sead::hostio::Context* pContext);
    void genMessageParameter(sead::hostio::Context* pContext, sead::hostio::Node* pNode);
    void genMessageDebugParameter(sead::hostio::Context* pContext, sead::hostio::Node* pNode);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);
    void listenPropertyEventParameter(const sead::hostio::PropertyEvent* pEvent);
    void listenPropertyEventDebugParameter(const sead::hostio::PropertyEvent* pEvent);

    void convBoundingToLightSpace(sead::BoundBox3f* pOut, const sead::BoundBox3f& rBox) const
    {
        (this->*mConvBox)(pOut, rBox);
    }
    void convBoundingToLightSpace(sead::BoundBox3f* pOut,
                                  const sead::Sphere<sead::Vector3f>& rSphere) const
    {
        (this->*mConvSphere)(pOut, rSphere);
    }

    const sead::Matrix34f& getLightViewMatrix() const { return mLightViewMtx; }
    const sead::Matrix44f& getLightProjMatrix() const { return mLightProjMtx; }
    const sead::Matrix34f& getShadowViewMatrix() const { return mShadowViewMtx; }
    const sead::Matrix44f& getShadowProjMatrix() const { return mShadowProjMtx; }
    const sead::Matrix44f& getTexMatrix() const { return mTexMtx; }
    bool isDirectional() const { return mIsDirectional; }
    s32 getClipPlaneNum() const { return mClipPlaneNum; }
    const sead::Plane3<f32>& getClipPlane(s32 index) const { return mClipPlanes[index]; }
    sead::BoundBox3f& getWorldBox(s32 index) { return mWorldBox[index]; }
    const sead::Matrix44f& getTexMtx() const { return mTexMtx; }
    const sead::Vector2f& getTexOffset() const { return mTexOffset; }
    const sead::Vector2f& getTexScale() const { return mTexScale; }
    void setDrawDebug(bool enable) { mIsDrawDebug = enable; }
    ShadowFrustum& getFrustum() { return mFrustum; }

    bool isCulled(const sead::BoundBox3f& rBox) const
    {
        for (s32 i = 0; i < mClipPlaneNum; i++)
        {
            const sead::Plane3<f32>& plane = mClipPlanes[i];
            const sead::Vector3f& n = plane.getNormal();
            const f32 x = n.x > 0.0f ? rBox.getMin().x : rBox.getMax().x;
            const f32 y = n.y > 0.0f ? rBox.getMin().y : rBox.getMax().y;
            const f32 z = n.z > 0.0f ? rBox.getMin().z : rBox.getMax().z;
            if (n.x * x + n.y * y + n.z * z - plane.getD() > 0.0f)
            {
                return true;
            }
        }
        return false;
    }

    bool isCulled(const sead::Sphere<sead::Vector3f>& rSphere) const
    {
        if (rSphere.getRadius() <= 0.0f)
        {
            return true;
        }
        for (s32 i = 0; i < mClipPlaneNum; i++)
        {
            const sead::Plane3<f32>& plane = mClipPlanes[i];
            if (rSphere.getRadius() < rSphere.getCenter().dot(plane.getNormal()) - plane.getD())
            {
                return true;
            }
        }
        return false;
    }

private:
    void convBoundingToLightSpaceDirectional_(sead::BoundBox3f* pOut,
                                              const sead::Sphere<sead::Vector3f>& rSphere) const;
    void convBoundingToLightSpaceDirectional_(sead::BoundBox3f* pOut,
                                              const sead::BoundBox3f& rBox) const;
    void convBoundingToLightSpaceNotDirectional_(sead::BoundBox3f* pOut,
                                                 const sead::BoundBox3f& rBox) const;
    void convBoundingToLightSpaceNotDirectional_(sead::BoundBox3f* pOut,
                                                 const sead::Sphere<sead::Vector3f>& rSphere) const;
    void updateShadowViewProjection_(const sead::Matrix34f& rViewMtx, MatrixCalcType type,
                                     f32 param0, f32 param1, const sead::Vector2i& rSize);
    void addCasterClipPlanes_(const sead::Matrix34f& rViewMtx, const sead::Matrix44f& rProjMtx,
                              const sead::Vector3f* pSweepDir, f32 zNear, f32 zFar, bool sweep);
    void updateDirectionalBoundingIndex_();
    bool calcLightSpacePerspectiveMatrix_(sead::Matrix44f* pOut, const sead::Matrix44f& rProjMtx,
                                          const sead::Matrix34f& rLightViewMtx,
                                          const sead::Matrix34f& rViewMtx, f32 param0, f32 param1);
    bool addCasterClipPlane_(const sead::Plane3<f32>& rPlane);

    sead::Plane3<f32>* findSameClipPlane_(const sead::Vector3f& rNormal)
    {
        for (s32 i = 0; i < mClipPlaneNum; i++)
        {
            sead::Plane3<f32>& plane = mClipPlanes[i];
            const f32 diff = rNormal.dot(plane.getNormal()) - 1.0f;
            if (diff <= 1.1920929e-06f && diff >= -1.1920929e-06f)
            {
                return &plane;
            }
        }
        return nullptr;
    }
    void addCasterClipPlanesSweepDir_(const sead::Vector3f& rDir, const sead::Vector3f* pPoints,
                                      s32 face, const sead::Vector3f& rNormal);

    ConvSphereFunc mConvSphere;
    ConvBoxFunc mConvBox;
    utl::Parameter<s32> mMatrixCalcType;
    bool mIsDrawDebug = false;
    ShadowFrustum mFrustum;
    s32 mMinIndex[3];
    f32 mNear = 0.0f;
    s32 mMaxIndex[3];
    f32 mFar = 0.0f;
    sead::Matrix34f mLightViewMtx = sead::Matrix34f::ident;
    sead::Matrix44f mLightProjMtx = sead::Matrix44f::ident;
    sead::Matrix44f _1c8;
    sead::Matrix34f _208 = sead::Matrix34f::ident;
    sead::Matrix44f _238 = sead::Matrix44f::ident;
    sead::Matrix34f mShadowViewMtx = sead::Matrix34f::ident;
    sead::Matrix44f mShadowProjMtx = sead::Matrix44f(sead::Matrix34f::ident);
    sead::Matrix44f mTexMtx = sead::Matrix44f::ident;
    sead::Matrix34f mCameraViewMtx = sead::Matrix34f::ident;
    sead::Matrix44f mCameraProjMtx = sead::Matrix44f::ident;
    sead::SafeArray<sead::BoundBox3f, 3> mWorldBox;
    sead::SafeArray<sead::BoundBox3f, 3> mLightBox;
    sead::BoundBox3f mTotalBox;
    s32 mClipPlaneNum = 0;
    sead::SafeArray<ClipPlane, cClipPlaneMax> mClipPlanes;
    bool mIsDirectional = true;
    sead::Vector2f mTexOffset = sead::Vector2f::zero;
    sead::Vector2f mTexScale = sead::Vector2f::ones;
};
static_assert(sizeof(DepthShadowUnit) == 0x5d8);

}  // namespace sdw
}  // namespace agl
