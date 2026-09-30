#pragma once

#include <container/seadBuffer.h>
#include <hostio/seadHostIONode.h>
#include <math/seadBoundBox.h>
#include <math/seadGeometry.h>
#include <math/seadMatrix.h>
#include <prim/seadBitFlag.h>
#include <prim/seadDelegate.h>

#include "shadow/aglDepthShadowUnit.h"
#include "shadow/aglLightMatrix.h"
#include "shadow/aglShadowMap.h"
#include "utility/aglParameter.h"
#include "utility/aglParameterIO.h"
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

class DepthShadow : public utl::IParameterIO, public sead::hostio::Node
{
public:
    struct CreateArg
    {
        s32 mCascadeNum;
    };

    struct DrawArg
    {
        s32 mIndex;
        const DepthShadow* mShadow;
        DrawContext* mDrawContext;
    };

    using CheckBoxFunc = sead::BitFlag32 (DepthShadow::*)(const sead::BoundBox3f&, s32,
                                                          sead::BitFlag32);
    using CheckSphereFunc = sead::BitFlag32 (DepthShadow::*)(const sead::Sphere<sead::Vector3f>&,
                                                             s32, sead::BitFlag32);

    DepthShadow();
    ~DepthShadow() override;

    void freeShadowMap(bool all) const;
    void initialize(const CreateArg& rArg, sead::Heap* pHeap);
    void allocShadowMap(DrawContext* pDrawContext) const;
    void drawShadowMap(DrawContext* pDrawContext) const;
    void copyReducedShadowMap(DrawContext* pDrawContext) const;
    void updatePlaneClipInfo();
    void updateSceneMatrix(const sead::Matrix34f& rViewMtx, const sead::Matrix44f& rProjMtx);
    void clipByPlane(const sead::Plane3<f32>& rPlane);
    void updateShadowMatrix();
    void drawDebug(DrawContext* pDrawContext, const sead::Matrix34f& rViewMtx,
                   const sead::Matrix44f& rProjMtx) const;

    void genMessage(sead::hostio::Context* pContext);
    void genMessageParameter(sead::hostio::Context* pContext, sead::hostio::Node* pNode);
    void genMessageDebugParameter(sead::hostio::Context* pContext, sead::hostio::Node* pNode);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);
    void listenPropertyEventParameter(const sead::hostio::PropertyEvent* pEvent);
    void listenPropertyEventDebugParameter(const sead::hostio::PropertyEvent* pEvent);

    sead::BitFlag32 checkAndUpdate(const sead::BoundBox3f& rBox, s32 type, sead::BitFlag32 skip)
    {
        return (this->*mCheckBox)(rBox, type, skip);
    }
    sead::BitFlag32 checkAndUpdate(const sead::Sphere<sead::Vector3f>& rSphere, s32 type,
                                   sead::BitFlag32 skip)
    {
        return (this->*mCheckSphere)(rSphere, type, skip);
    }

    s32 getCascadeNum() const
    {
        return sead::Mathi::min(sead::Mathi::max(*mCascadeNum, 1), mUnits.size());
    }
    DepthShadowUnit& getUnit(s32 index) { return mUnits[index]; }
    const DepthShadowUnit& getUnit(s32 index) const { return mUnits[index]; }
    LightMatrix& getLightMatrix() { return mLightMatrix; }
    ShadowMap& getShadowMap() { return mShadowMap; }
    void setDrawCallback(sead::IDelegate1<const DrawArg&>* pCallback) { mDrawCallback = pCallback; }
    void setDebugDrawFlag(u32 flag) { mDebugDrawFlag = flag; }

    sead::BitFlag32 checkBox(const sead::BoundBox3f& rBox, s32 type, sead::BitFlag32 skip)
    {
        return (this->*mCheckBox)(rBox, type, skip);
    }
    sead::BitFlag32 checkSphere(const sead::Sphere<sead::Vector3f>& rSphere, s32 type,
                                sead::BitFlag32 skip)
    {
        return (this->*mCheckSphere)(rSphere, type, skip);
    }

private:
    template <typename T>
    sead::BitFlag32 checkAndUpdateLightSpaceDirectional_(const T& rBounding, s32 type,
                                                         sead::BitFlag32 skip);
    template <typename T>
    sead::BitFlag32 checkAndUpdateLightSpace_(const T& rBounding, s32 type, sead::BitFlag32 skip);
    sead::BitFlag32 checkAndUpdateWorld_(const sead::BoundBox3f& rBox, s32 type,
                                         sead::BitFlag32 skip);
    sead::BitFlag32 checkAndUpdateWorld_(const sead::Sphere<sead::Vector3f>& rSphere, s32 type,
                                         sead::BitFlag32 skip);
    template <typename T>
    sead::BitFlag32 checkOnly_(const T& rBounding, s32 type, sead::BitFlag32 skip);
    template <typename T>
    sead::BitFlag32 noCheck_(const T& rBounding, s32 type, sead::BitFlag32 skip);

    CheckBoxFunc mCheckBox;
    CheckSphereFunc mCheckSphere;
    utl::ParameterObj mParamObj;
    utl::Parameter<s32> mCascadeNum;
    utl::Parameter<s32> mMipLevelNum;
    utl::Parameter<bool> mDepthClamp;
    utl::Parameter<s32> mBoundingCalcType;
    utl::Parameter<f32> mOptimizeOffsetNear;
    utl::Parameter<f32> mOptimizeOffsetFar;
    utl::Parameter<s32> mStableTexelWidth;
    utl::ParameterBuffer<f32> mCascadeNear;
    utl::Parameter<f32> mSamePointEpsilon;
    utl::ParameterBuffer<f32> mNearFarMargin;
    sead::Buffer<DepthShadowUnit> mUnits;
    ShadowMap mShadowMap;
    bool mIsDirectional = true;
    LightMatrix mLightMatrix;
    sead::IDelegate1<const DrawArg&>* mDrawCallback = nullptr;
    u8 _1548[0x20];
    u32 mDebugDrawFlag = 0;
};
static_assert(sizeof(DepthShadow) == 0x1570);

}  // namespace sdw
}  // namespace agl
