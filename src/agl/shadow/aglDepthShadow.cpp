#include "shadow/aglDepthShadow.h"

#include <math/seadBoundBox.hpp>
#include <math/seadMathCalcCommon.h>
#include <prim/seadSafeString.h>

#include "common/aglDrawContext.h"
#include "driver/aglGraphicsDriverMgr.h"

namespace agl::sdw
{

namespace
{

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

}  // namespace

/**
 * Constructs a depth shadow with default parameters.
 */
DepthShadow::DepthShadow()
    : utl::IParameterIO("aglsdw", 0),
      mCheckBox(&DepthShadow::checkAndUpdateLightSpaceDirectional_<sead::BoundBox3f>),
      mCheckSphere(
          &DepthShadow::checkAndUpdateLightSpaceDirectional_<sead::Sphere<sead::Vector3f>>),
      mCascadeNum(1, "cascade_num", "Cascade Num", &mParamObj),
      mMipLevelNum(1, "mip_level_num", "MipLevel Num", "Min=1, Max=8", &mParamObj),
      mDepthClamp(true, "depth_clamp", "Depth Clamp", &mParamObj),
      mBoundingCalcType(1, "bounding_calc_type", "Bounding Calc Type", &mParamObj),
      mOptimizeOffsetNear(0.0f, "optimize_offset_near", "Optimize Offset Near", "Min=0",
                          &mParamObj),
      mOptimizeOffsetFar(0.0f, "optimize_offset_far", "Optimize Offset Far", "Min=0", &mParamObj),
      mStableTexelWidth(1, "stable_texel_width", "Stable Texel Width", "Min=1, Max=32", &mParamObj),
      mCascadeNear("cascade_near", "cascade_near", &mParamObj),
      mSamePointEpsilon(0.05f, "same_point_epsilon", "Same Point Epsilon", "Min=0, Max=1",
                        &mParamObj),
      mNearFarMargin("near_far_margin_array", "Near Far Margin", &mParamObj)
{
}

/**
 * Accumulates a bounding volume into the light space bounds of the visible cascades.
 * @param rBounding bounding volume
 * @param type bounding type index
 * @param skip cascades to skip
 * @return the cascades the volume is visible in
 */
template <typename T>
sead::BitFlag32 DepthShadow::checkAndUpdateLightSpaceDirectional_(const T& rBounding, s32 type,
                                                                  sead::BitFlag32 skip)
{
    sead::BoundBox3f box;
    sead::BitFlag32 result = 0;
    bool converted = false;

    for (s32 i = 0; i < *mCascadeNum; i++)
    {
        if (skip.isOnBit(i))
        {
            continue;
        }

        DepthShadowUnit& unit = mUnits[i];

        if (unit.isCulled(rBounding))
        {
            continue;
        }

        if (!converted)
        {
            unit.convBoundingToLightSpace(&box, rBounding);
        }

        mergeBox(&mUnits[i].mLightBox[type], box);
        result.setBit(i);
        converted = true;
    }

    return result;
}

/**
 * Destroys the depth shadow and frees the shadow map.
 */
DepthShadow::~DepthShadow()
{
    mShadowMap.free();
    mUnits.freeBuffer();
}

/**
 * Frees the shadow map textures.
 * @param all whether to free every texture or only the full size ones
 */
void DepthShadow::freeShadowMap(bool all) const
{
    if (all)
    {
        const_cast<ShadowMap&>(mShadowMap).free();
    }
    else
    {
        const_cast<ShadowMap&>(mShadowMap).freeFullOnly();
    }
}

/**
 * Allocates the cascades and the shadow map.
 * @param rArg creation parameters
 * @param pHeap heap to allocate from
 */
void DepthShadow::initialize(const CreateArg& rArg, sead::Heap* pHeap)
{
    *mCascadeNum = rArg.mCascadeNum;
    mUnits.tryAllocBuffer(rArg.mCascadeNum, pHeap);
    mCascadeNear.allocateBuffer(pHeap, rArg.mCascadeNum + 1);
    mNearFarMargin.allocateBuffer(pHeap, rArg.mCascadeNum);

    for (s32 i = 0; i < rArg.mCascadeNum; i++)
    {
        mCascadeNear.ref()[i] = i == 0 ? 10.0f : mCascadeNear.ref()[i - 1] + 500.0f;
        mUnits[i].initialize(pHeap);
        mNearFarMargin.ref()[i] = 0.0f;
    }

    mCascadeNear.ref()[rArg.mCascadeNum] =
        sead::Mathf::clampMin(mCascadeNear.ref()[rArg.mCascadeNum - 1] + 500.0f, 20000.0f);

    ShadowMap::CreateArg arg;
    arg.mCascadeNum = rArg.mCascadeNum;
    mShadowMap.initialize(arg, pHeap);

    addObj(&mParamObj, "depth_shadow");

    for (s32 i = 0; i < rArg.mCascadeNum; i++)
    {
        addObj(&mUnits[i], sead::FormatFixedSafeString<32>("shadow_unit_%d", i));
    }

    addObj(&mShadowMap, "shadow_map");
}

/**
 * Allocates the shadow map depth buffer.
 * @param pDrawContext draw context
 */
void DepthShadow::allocShadowMap(DrawContext* pDrawContext) const
{
    const s32 num = getCascadeNum();
    const_cast<ShadowMap&>(mShadowMap).allocDepthBuffer(pDrawContext, num, *mMipLevelNum);
}

/**
 * Renders every cascade into the shadow map.
 * @param pDrawContext draw context
 */
void DepthShadow::drawShadowMap(DrawContext* pDrawContext) const
{
    const s32 num = getCascadeNum();
    DrawArg arg;
    arg.mIndex = 0;
    arg.mShadow = this;
    arg.mDrawContext = pDrawContext;

    ShadowMap& shadowMap = const_cast<ShadowMap&>(mShadowMap);

    for (s32 i = 0; i < num; i++)
    {
        const DepthShadowUnit& unit = mUnits[i];
        shadowMap.beginDepthBuffer(pDrawContext, i, unit.getTexOffset(), unit.getTexScale());

        if (*mDepthClamp)
        {
            driver::GraphicsDriverMgr::instance()->setDepthClamp(pDrawContext, true);
        }

        arg.mIndex = i;

        if (mDrawCallback != nullptr)
        {
            mDrawCallback->invoke(arg);
        }

        if (*mDepthClamp)
        {
            driver::GraphicsDriverMgr::instance()->setDepthClamp(pDrawContext, false);
        }

        shadowMap.endDepthBuffer(pDrawContext, i);
    }

    if (mShadowMap.getShadowMapType() == 1)
    {
        mShadowMap.drawVariance(pDrawContext);
    }
}

/**
 * Draws the reduced shadow maps.
 * @param pDrawContext draw context
 */
void DepthShadow::copyReducedShadowMap(DrawContext* pDrawContext) const
{
    mShadowMap.drawReduce(pDrawContext);
}

/**
 * Rebuilds the caster clip planes of every cascade.
 */
void DepthShadow::updatePlaneClipInfo()
{
    for (s32 i = 0; i < *mCascadeNum; i++)
    {
        mUnits[i].updatePlaneClipInfo();
    }
}

/**
 * Updates every cascade for a new camera and selects the bounding check functions.
 * @param rViewMtx camera view matrix
 * @param rProjMtx camera projection matrix
 */
void DepthShadow::updateSceneMatrix(const sead::Matrix34f& rViewMtx,
                                    const sead::Matrix44f& rProjMtx)
{
    const s32 num = getCascadeNum();
    *mCascadeNum = num;
    for (s32 i = 0; i < num; i++)
    {
        DepthShadowUnit& unit = mUnits[i];
        const f32 near = mCascadeNear.ref()[i] - (i != 0 ? mNearFarMargin.ref()[i] : 0.0f);
        const f32 far = mCascadeNear.ref()[i + 1] + (i != num - 1 ? mNearFarMargin.ref()[i] : 0.0f);
        unit.updateSceneMatrix(mLightMatrix, rViewMtx, rProjMtx, near, far, *mSamePointEpsilon);
    }

    switch (*mBoundingCalcType)
    {
    case 0:
        mCheckBox = &DepthShadow::checkAndUpdateWorld_;
        mCheckSphere = &DepthShadow::checkAndUpdateWorld_;
        break;
    case 1:
        if (mIsDirectional)
        {
            mCheckBox = &DepthShadow::checkAndUpdateLightSpaceDirectional_<sead::BoundBox3f>;
            mCheckSphere =
                &DepthShadow::checkAndUpdateLightSpaceDirectional_<sead::Sphere<sead::Vector3f>>;
        }
        else
        {
            mCheckBox = &DepthShadow::checkAndUpdateLightSpace_<sead::BoundBox3f>;
            mCheckSphere = &DepthShadow::checkAndUpdateLightSpace_<sead::Sphere<sead::Vector3f>>;
        }

        break;
    case 2:
        mCheckBox = &DepthShadow::checkOnly_<sead::BoundBox3f>;
        mCheckSphere = &DepthShadow::checkOnly_<sead::Sphere<sead::Vector3f>>;
        break;
    default:
        mCheckBox = &DepthShadow::noCheck_<sead::BoundBox3f>;
        mCheckSphere = &DepthShadow::noCheck_<sead::Sphere<sead::Vector3f>>;
        break;
    }
}

/**
 * Accumulates a bounding volume into the light space bounds of the visible cascades.
 * @param rBounding bounding volume
 * @param type bounding type index
 * @param skip cascades to skip
 * @return the cascades the volume is visible in
 */
template <typename T>
sead::BitFlag32 DepthShadow::checkAndUpdateLightSpace_(const T& rBounding, s32 type,
                                                       sead::BitFlag32 skip)
{
    sead::BoundBox3f box;
    sead::BitFlag32 result = 0;
    bool converted = false;

    for (s32 i = 0; i < *mCascadeNum; i++)
    {
        if (skip.isOnBit(i))
        {
            continue;
        }

        DepthShadowUnit& unit = mUnits[i];

        if (unit.isCulled(rBounding))
        {
            continue;
        }

        if (!converted)
        {
            unit.convBoundingToLightSpace(&box, rBounding);
        }

        mergeBox(&mUnits[i].mLightBox[type], box);
        result.setBit(i);
        converted = true;
    }

    return result;
}

/**
 * Accumulates a box into the world space bounds of the visible cascades.
 * @param rBox box
 * @param type bounding type index
 * @param skip cascades to skip
 * @return the cascades the box is visible in
 */
sead::BitFlag32 DepthShadow::checkAndUpdateWorld_(const sead::BoundBox3f& rBox, s32 type,
                                                  sead::BitFlag32 skip)
{
    sead::BitFlag32 result = 0;

    for (s32 i = 0; i < *mCascadeNum; i++)
    {
        if (skip.isOnBit(i))
        {
            continue;
        }

        DepthShadowUnit& unit = mUnits[i];

        if (unit.isCulled(rBox))
        {
            continue;
        }

        mergeBox(&unit.mWorldBox[type], rBox);
        result.setBit(i);
    }

    return result;
}

/**
 * Accumulates a sphere into the world space bounds of the visible cascades.
 * @param rSphere sphere
 * @param type bounding type index
 * @param skip cascades to skip
 * @return the cascades the sphere is visible in
 */
sead::BitFlag32 DepthShadow::checkAndUpdateWorld_(const sead::Sphere<sead::Vector3f>& rSphere,
                                                  s32 type, sead::BitFlag32 skip)
{
    sead::BoundBox3f box;
    sead::BitFlag32 result = 0;
    bool converted = false;

    for (s32 i = 0; i < *mCascadeNum; i++)
    {
        if (skip.isOnBit(i))
        {
            continue;
        }

        DepthShadowUnit& unit = mUnits[i];

        if (unit.isCulled(rSphere))
        {
            continue;
        }

        if (!converted)
        {
            unit.convBoundingSphereToBox(&box, rSphere);
        }

        mergeBox(&mUnits[i].mWorldBox[type], box);
        result.setBit(i);
        converted = true;
    }

    return result;
}

/**
 * Checks in which cascades a bounding volume is visible.
 * @param rBounding bounding volume
 * @param type bounding type index (unused)
 * @param skip cascades to skip
 * @return the cascades the volume is visible in
 */
template <typename T>
sead::BitFlag32 DepthShadow::checkOnly_(const T& rBounding, s32 type, sead::BitFlag32 skip)
{
    sead::BitFlag32 result = 0;

    for (s32 i = 0; i < *mCascadeNum; i++)
    {
        if (skip.isOnBit(i))
        {
            continue;
        }

        if (mUnits[i].isCulled(rBounding))
        {
            continue;
        }

        result.setBit(i);
    }

    return result;
}

/**
 * Marks a bounding volume as visible in every cascade.
 * @param rBounding bounding volume (unused)
 * @param type bounding type index (unused)
 * @param skip cascades to skip
 * @return every cascade that is not skipped
 */
template <typename T>
sead::BitFlag32 DepthShadow::noCheck_(const T& rBounding, s32 type, sead::BitFlag32 skip)
{
    sead::BitFlag32 result = 0;

    for (s32 i = 0; i < *mCascadeNum; i++)
    {
        if (!skip.isOnBit(i))
        {
            result.setBit(i);
        }
    }

    return result;
}

/**
 * Clips the frustum polytope of every cascade by a plane.
 * @param rPlane plane to clip by
 */
void DepthShadow::clipByPlane(const sead::Plane3<f32>& rPlane)
{
    for (s32 i = 0; i < *mCascadeNum; i++)
    {
        mUnits[i].clipByPlane(rPlane);
    }
}

/**
 * Computes the shadow matrices of every cascade.
 */
void DepthShadow::updateShadowMatrix()
{
    for (s32 i = 0; i < *mCascadeNum; i++)
    {
        const f32 offsetNear = i == 0 ? *mOptimizeOffsetNear : 0.0f;
        const f32 offsetFar = i == *mCascadeNum - 1 ? *mOptimizeOffsetFar : 0.0f;
        mUnits[i].updateShadowMatrix(mShadowMap.getWidth() / *mStableTexelWidth,
                                     mShadowMap.getHeight() / *mStableTexelWidth, offsetNear,
                                     offsetFar);
    }
}

/**
 * Draws the debug views of the cascades selected by the debug flags.
 * @param pDrawContext draw context
 * @param rViewMtx view matrix used for drawing
 * @param rProjMtx projection matrix used for drawing
 */
void DepthShadow::drawDebug(DrawContext* pDrawContext, const sead::Matrix34f& rViewMtx,
                            const sead::Matrix44f& rProjMtx) const
{
    if (mDebugDrawFlag == 0)
    {
        return;
    }

    const s32 num = *mCascadeNum;

    for (s32 i = 0; i < num; i++)
    {
        const_cast<DepthShadowUnit&>(mUnits[i]).setDrawDebug((mDebugDrawFlag & (1 << i)) != 0);

        if (mDebugDrawFlag & (1 << i))
        {
            mUnits[i].drawDebug(pDrawContext, mLightMatrix, rViewMtx, rProjMtx);
        }

        if (mDebugDrawFlag & (1 << (i + mUnits.size())))
        {
            const_cast<ShadowMap&>(mShadowMap)
                .drawDebug(pDrawContext, i, mUnits[i].getTexMtx(), rViewMtx, rProjMtx);
        }
    }
}

/**
 * Generates the host IO message.
 * @param pContext host IO context
 */
void DepthShadow::genMessage(sead::hostio::Context* pContext)
{
    genMessageIO(pContext, 0xf);
    mShadowMap.genMessageParameter(pContext, this);
    const s32 num = *mCascadeNum;

    for (s32 i = 0; i < num; i++)
    {
        mUnits[i].genMessageParameter(pContext, this);
    }

    genMessageDebugParameter(pContext, this);
}

/**
 * Generates the parameter host IO messages.
 * @param pContext host IO context
 * @param pNode parent node
 */
void DepthShadow::genMessageParameter(sead::hostio::Context* pContext, sead::hostio::Node* pNode)
{
    mShadowMap.genMessageParameter(pContext, pNode);
    const s32 num = *mCascadeNum;

    for (s32 i = 0; i < num; i++)
    {
        mUnits[i].genMessageParameter(pContext, pNode);
    }
}

/**
 * Generates the debug parameter host IO messages.
 * @param pContext host IO context
 * @param pNode parent node
 */
void DepthShadow::genMessageDebugParameter(sead::hostio::Context* pContext,
                                           sead::hostio::Node* pNode)
{
    {
        sead::FormatFixedSafeString<32> meta("Min=1, Max=%d", mUnits.size());
    }

    const s32 num = *mCascadeNum;

    for (s32 i = 0; i < num; i++)
    {
        sead::FormatFixedSafeString<32> frustum("Cascade %d Frustum", i);
        sead::FormatFixedSafeString<32> depth("Cascade %d Depth", i);
    }

    const s32 cascadeNum = *mCascadeNum;

    for (s32 i = 0; i < cascadeNum; i++)
    {
        sead::FormatFixedSafeString<32> near("Frustum Near %d", i);
        mUnits[i].genMessageDebugParameter(pContext, this);
    }

    mOptimizeOffsetNear.genMessageParameter(pContext, mOptimizeOffsetNear.getMeta());
    mOptimizeOffsetFar.genMessageParameter(pContext, mOptimizeOffsetFar.getMeta());
    mStableTexelWidth.genMessageParameter(pContext, mStableTexelWidth.getMeta());
    mDepthClamp.genMessageParameter(pContext, mDepthClamp.getMeta());
    mShadowMap.genMessageDebugParameter(pContext, pNode);
    mSamePointEpsilon.genMessageParameter(pContext, mSamePointEpsilon.getMeta());
}

/**
 * Handles a host IO property event.
 * @param pEvent property event
 */
void DepthShadow::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent)
{
    listenPropertyEventIO(this, pEvent);
    mShadowMap.listenPropertyEventParameter(pEvent);
    mShadowMap.listenPropertyEventDebugParameter(pEvent);
}

/**
 * Handles a parameter host IO property event.
 * @param pEvent property event
 */
void DepthShadow::listenPropertyEventParameter(const sead::hostio::PropertyEvent* pEvent)
{
    mShadowMap.listenPropertyEventParameter(pEvent);
}

/**
 * Handles a debug parameter host IO property event.
 * @param pEvent property event
 */
void DepthShadow::listenPropertyEventDebugParameter(const sead::hostio::PropertyEvent* pEvent)
{
    mShadowMap.listenPropertyEventDebugParameter(pEvent);
}

}  // namespace agl::sdw
