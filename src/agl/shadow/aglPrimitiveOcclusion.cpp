#include "shadow/aglPrimitiveOcclusion.h"

#include <arm_neon.h>
#include <attributes.h>
#include <gfx/seadCamera.h>
#include <gfx/seadColor.h>
#include <gfx/seadGraphicsContext.h>
#include <gfx/seadProjection.h>
#include <gfx/seadViewport.h>
#include <hostio/seadHostIOPropertyEvent.h>
#include <math/seadMathCalcCommon.h>
#include <math/seadQuat.h>

#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/Memory/Util.hpp"
#include "common/aglDrawContext.h"
#include "common/aglGPUMemBlock.h"
#include "common/aglRenderBuffer.h"
#include "common/aglRenderTarget.h"
#include "common/aglShaderProgram.h"
#include "common/aglTextureData.h"
#include "common/aglTextureSampler.h"
#include "postfx/aglPostFxUtil.h"
#include "utility/aglDevTools.h"
#include "utility/aglPrimitiveShape.h"
#include "utility/aglVertexAttributeHolder.h"

namespace agl::sdw
{

namespace
{

/**
 * Gets the draw context of the running framework.
 * @return the draw context
 */
inline DrawContext* getDrawContext()
{
    return al::GameFrameworkNx::getAglDrawContext();
}

void bindGBuffer(const TextureData& rNormal, const TextureData& rDepth);

/**
 * Multiplies two 4x4 matrices.
 * @param pDst destination
 * @param rA left operand
 * @param rB right operand
 */
ALWAYS_INLINE inline void multiplyMtx44(sead::Matrix44f* pDst, const sead::Matrix44f& rA,
                                        const sead::Matrix44f& rB)
{
    const float32x4_t b0 = vld1q_f32(rB.m[0]);
    const float32x4_t b1 = vld1q_f32(rB.m[1]);
    const float32x4_t b2 = vld1q_f32(rB.m[2]);
    const float32x4_t b3 = vld1q_f32(rB.m[3]);

    for (s32 i = 0; i < 4; i++)
    {
        const float32x4_t a = vld1q_f32(rA.m[i]);
        float32x4_t row = vmulq_laneq_f32(b0, a, 0);
        row = vfmaq_laneq_f32(row, b1, a, 1);
        row = vfmaq_laneq_f32(row, b2, a, 2);
        row = vfmaq_laneq_f32(row, b3, a, 3);
        vst1q_f32(pDst->m[i], row);
    }
}

/**
 * Writes one float member of a uniform block.
 * @param rUbo uniform block
 * @param memberIndex member to write
 * @param value value to write
 */
inline void setDataFloat(const UniformBlock& rUbo, s32 memberIndex, f32 value)
{
    rUbo.setData(memberIndex, &value, 0, 1);
}

/**
 * Writes one vec4 array element of a uniform block.
 * @param rUbo uniform block
 * @param memberIndex member to write
 * @param value value to write
 * @param arrayIndex array element to write
 */
inline void setDataVec4(const UniformBlock& rUbo, s32 memberIndex, sead::Vector4f value,
                        s32 arrayIndex)
{
    rUbo.setData(memberIndex, &value, arrayIndex, 1);
}

/**
 * Flushes the CPU cache of the current buffer of a uniform block.
 * @param rUbo uniform block
 */
inline void flushUbo(const UniformBlock& rUbo)
{
    const u32 offset = rUbo.getCurrentBlockOffset(0);
    GPUMemVoidAddr addr = rUbo.getBuffer();
    GPUMemVoidAddr(addr, offset).flushCPUCache(rUbo.getBlockSize());
}

/**
 * Copies a vector as plain data.
 * @param pDst destination vector
 * @param rSrc source vector
 */
inline void copyVec3(sead::Vector3f* pDst, const sead::Vector3f& rSrc)
{
    static_cast<sead::BaseVec3<f32>&>(*pDst) = rSrc;
}

/**
 * Binds one slice of a texture as the only color target of a render buffer while alive.
 */
class RenderTargetBinder
{
public:
    /**
     * Binds the slice.
     * @param pRenderBuffer render buffer to bind the target to
     * @param pTexture texture to render to
     * @param slice slice of the texture
     */
    RenderTargetBinder(RenderBuffer* pRenderBuffer, const TextureData* pTexture, s32 slice)
        : mRenderBuffer(pRenderBuffer)
    {
        mRenderBuffer->setVirtualSize(
            sead::Vector2f(pTexture->getWidth(0), pTexture->getHeight(0)));
        mRenderBuffer->setPhysicalArea(
            sead::BoundBox2f(0.0f, 0.0f, pTexture->getWidth(0), pTexture->getHeight(0)));
        mRenderBuffer->setRenderTargetColorNullAll();
        mRenderBuffer->setRenderTargetDepth(nullptr);
        mTexture[0] = pTexture;

        if (pTexture != nullptr)
        {
            mColor[0].applyTextureData(*pTexture);
            mColor[0].setSlice(slice);
            mColor[0].setMipLevel(0);
            mRenderBuffer->setRenderTargetColor(&mColor[0], 0);
        }

        for (s32 i = 1; i < 4; i++)
        {
            mTexture[i] = nullptr;
        }
    }

    /**
     * Unbinds the targets and invalidates the caches of the rendered textures.
     */
    ~RenderTargetBinder()
    {
        mRenderBuffer->setRenderTargetColorNullAll();
        mRenderBuffer->setRenderTargetDepth(nullptr);

        for (s32 i = 0; i < 4; i++)
        {
            if (mTexture[i] != nullptr)
            {
                mColor[i].invalidateGPUCache(getDrawContext());
            }
        }
    }

    RenderBuffer* getRenderBuffer() const { return mRenderBuffer; }

private:
    RenderBuffer* mRenderBuffer;
    RenderTargetColor mColor[4];
    const TextureData* mTexture[4];
};

}  // namespace

/**
 * Sets the ambient occlusion parameters.
 * @param rPos center of the sphere
 * @param radius radius of the sphere
 * @param intensity ambient occlusion intensity
 */
void SphereOcclusion::setParam(const sead::Vector3f& rPos, f32 radius, f32 intensity)
{
    copyVec3(&mPos, rPos);
    mRadius = radius;
    mAoIntensity = intensity;
}

/**
 * Sets the directional occlusion parameters.
 * @param rDir direction of the occlusion cone
 * @param length length of the occlusion cone
 * @param intensity directional occlusion intensity
 */
void SphereOcclusion::setDoParam(const sead::Vector3f& rDir, f32 length, f32 intensity)
{
    copyVec3(&mDir, rDir);
    mDoLength = length;
    mDoIntensity = intensity;
}

/**
 * Writes the ambient occlusion uniform data in view space.
 * @param pDst destination uniform data
 * @param rViewMtx view matrix
 */
void SphereOcclusion::storeUboStructAo(SphereAo* pDst, const sead::Matrix34f& rViewMtx) const
{
    const f32 range = mRadius * 3.0f;
    const f32 rangeSq = range * range;
    sead::Vector3f viewPos;
    viewPos.setMul(rViewMtx, mPos);
    const sead::Vector4f viewPosIntensity(viewPos.x, viewPos.y, viewPos.z, mAoIntensity);
    *reinterpret_cast<sead::Vector4f*>(&pDst->mViewPos) = viewPosIntensity;
    pDst->mRadiusSq = mRadius * mRadius;
    pDst->mRangeInvSq = 1.0f / rangeSq;
}

/**
 * Writes the directional occlusion uniform data in view space.
 * @param pDst destination uniform data
 * @param rViewMtx view matrix
 */
void SphereOcclusion::storeUboStructDo(SphereDo* pDst, const sead::Matrix34f& rViewMtx) const
{
    sead::Vector3f viewPos;
    viewPos.setMul(rViewMtx, mPos);
    sead::Vector3f viewDir;
    viewDir.setRotated(rViewMtx, mDir);
    const sead::Vector4f viewPosIntensity(viewPos.x, viewPos.y, viewPos.z, mDoIntensity);
    *reinterpret_cast<sead::Vector4f*>(&pDst->mViewPos) = viewPosIntensity;
    const sead::Vector4f viewDirRadius(viewDir.x, viewDir.y, viewDir.z, mRadius);
    *reinterpret_cast<sead::Vector4f*>(&pDst->mViewDir) = viewDirRadius;
    pDst->mLengthInv = 1.0f / mDoLength;
}

/**
 * Declares and creates the per sphere ambient occlusion uniform blocks.
 * @param pBuffer uniform blocks (sphere x context)
 */
void PrimitiveOcclusion::initSphereAoUbo(sead::Buffer2<UniformBlock>* pBuffer)
{
    const s32 width = pBuffer->getWidth();
    const s32 height = pBuffer->getHeight();

    for (s32 x = 0; x < width; x++)
    {
        for (s32 y = 0; y < height; y++)
        {
            UniformBlock* pUbo = pBuffer->get(x, y);
            pUbo->startDeclare(3, nullptr);
            pUbo->declare(UniformBlock::cType_Vec4, 1);
            pUbo->declare(UniformBlock::cType_Vec4, 1);
            pUbo->declare(UniformBlock::cType_Vec4, 4);
            pUbo->create(nullptr, 2, 1);
        }
    }
}

/**
 * Declares and creates the per sphere directional occlusion uniform blocks.
 * @param pBuffer uniform blocks (sphere x context)
 */
void PrimitiveOcclusion::initSphereDoUbo(sead::Buffer2<UniformBlock>* pBuffer)
{
    const s32 width = pBuffer->getWidth();
    const s32 height = pBuffer->getHeight();

    for (s32 x = 0; x < width; x++)
    {
        for (s32 y = 0; y < height; y++)
        {
            UniformBlock* pUbo = pBuffer->get(x, y);
            pUbo->startDeclare(5, nullptr);
            pUbo->declare(UniformBlock::cType_Vec4, 1);
            pUbo->declare(UniformBlock::cType_Vec4, 1);
            pUbo->declare(UniformBlock::cType_Vec4, 1);
            pUbo->declare(UniformBlock::cType_Vec4, 4);
            pUbo->declare(UniformBlock::cType_Vec4, 1);
            pUbo->create(nullptr, 2, 1);
        }
    }
}

/**
 * Constructs the primitive occlusion with its default parameters.
 */
PrimitiveOcclusion::PrimitiveOcclusion()
    : mConeDegreeMin(10.0f, "SphereDoConeDegreeMin", "スフィアＤＯコーン角度最小(Degree)",
                     "Min=5, Max=360", &mParameterObj),
      mConeDegreeMax(60.0f, "SphereDoConeDegreeMax", "スフィアＤＯコーン角度最大(Degree)",
                     "Min=5, Max=360", &mParameterObj),
      mConeDegree(10.0f, "SphereDoConeDegree", "スフィアＤＯコーン角度(Degree)", "Min=5, Max=360",
                  &mParameterObj)
{
}

/**
 * Sets the ambient occlusion shader and searches its uniform blocks.
 * @param pProgram shader program
 */
void PrimitiveOcclusion::setShaderSphereAo(const ShaderProgram* pProgram)
{
    mSphereAoProgram = pProgram;
    mSphereAoContextLocation.search(*mSphereAoProgram);
    mSphereAoViewLocation.search(*mSphereAoProgram);
}

/**
 * Sets the directional occlusion shader and searches its uniform blocks.
 * @param pProgram shader program
 */
void PrimitiveOcclusion::setShaderSphereDo(const ShaderProgram* pProgram)
{
    mSphereDoProgram = pProgram;
    mSphereDoContextLocation.search(*mSphereDoProgram);
    mSphereDoViewLocation.search(*mSphereDoProgram);
}

/**
 * Sets the shader that bakes the directional occlusion table.
 * @param pProgram shader program
 */
void PrimitiveOcclusion::setShaderMakeTableSphereDo(const ShaderProgram* pProgram)
{
    mMakeTableSphereDoProgram = pProgram;
}

/**
 * Allocates the contexts, uniform blocks, sphere pool and the directional occlusion table.
 * @param rArg creation arguments
 */
void PrimitiveOcclusion::init(const CreateArg& rArg)
{
    mArg = rArg;

    mSphereAttribute.create(1, nullptr);
    mSphereAttribute.setVertexStream(0, &utl::PrimitiveShape::instance()->getSphereVertexBuffer(),
                                     0);
    mSphereAttribute.setUp();

    mConeAttribute.create(1, nullptr);
    mConeAttribute.setVertexStream(0, &utl::PrimitiveShape::instance()->getConeVertexBuffer(), 0);
    mConeAttribute.setUp();

    mContexts.tryAllocBuffer(rArg.mContextNum, nullptr);

    for (s32 i = 0; i < rArg.mContextNum; i++)
    {
        UniformBlock& rUbo = mContexts[i].mUbo;
        rUbo.startDeclare(4, nullptr);
        rUbo.declare(UniformBlock::cType_Vec4, 4);
        rUbo.declare(UniformBlock::cType_Vec4, 3);
        rUbo.declare(UniformBlock::cType_Vec4, 3);
        rUbo.declare(UniformBlock::cType_Vec4, 2);
        rUbo.create(nullptr, 2, 1);
    }

    mEnvUbo.startDeclare(2, nullptr);
    mEnvUbo.declare(UniformBlock::cType_Float, 1);
    mEnvUbo.declare(UniformBlock::cType_Float, 1);
    mEnvUbo.create(nullptr, 2, 1);

    mSphereAoUbo.tryAllocBuffer(rArg.mSphereNum, rArg.mContextNum, nullptr);
    initSphereAoUbo(&mSphereAoUbo);
    mSphereDoUbo.tryAllocBuffer(rArg.mSphereNum, rArg.mContextNum, nullptr);
    initSphereDoUbo(&mSphereDoUbo);

    mSpheres.tryAllocBuffer(rArg.mSphereNum, nullptr);
    mAoList.initOffset(offsetof(SphereOcclusion, mAoNode));
    mDoList.initOffset(offsetof(SphereOcclusion, mDoNode));

    sead::Heap* pHeap = al::getCurrentHeap();
    mSphereDoTable = new TextureData();
    mSphereDoTable->initialize_(TextureType::cTextureType_3D,
                                TextureFormat::cTextureFormat_R16_float, 256, 128, 1, 1,
                                TextureAttribute(0), MultiSampleType(0), true);
    const u32 alignment = mSphereDoTable->getAlignment();
    const u32 size = mSphereDoTable->getImageByteSize();
    GPUMemBlockU8* pBlock = new (pHeap, 8) GPUMemBlockU8;
    pBlock->allocBuffer_(size, pHeap, alignment, MemoryAttribute::Default);
    mSphereDoTableBuffer = GPUMemVoidAddr(*pBlock, 0);
    mSphereDoTable->setImagePtr(mSphereDoTableBuffer);

    mSphereDoTableSampler = new TextureSampler(*mSphereDoTable);
    mSphereDoTableSampler->setWrap(7, 7, 7);
    mSphereDoTableSampler->setFilter(1, 1, 0);

    mSphereDoTableRenderBuffer = new RenderBuffer();

    mDebugTexturePage.setUp(rArg.mContextNum, "PrimitiveOcclusion", nullptr);
    clearRequest();
}

/**
 * Drops all sphere requests of the previous frame and updates the shared uniform block.
 */
void PrimitiveOcclusion::clearRequest()
{
    mMakeTableRequest.swap();
    mBufferIndex = mBufferIndex == 0;

    f32 coneRatio;

    if (mSphereDoTable->getMipSlice(0) == 1)
    {
        coneRatio = 0.0f;
    }
    else
    {
        coneRatio = (*mConeDegree - *mConeDegreeMin) / (*mConeDegreeMax - *mConeDegreeMin);
    }

    mEnvUbo.setCurrentBufferIndex(mBufferIndex);
    mEnvUbo.dcbz(0);
    setDataFloat(mEnvUbo, 0, coneRatio);
    setDataFloat(mEnvUbo, 1, mDoEnvParam);
    flushUbo(mEnvUbo);

    mRequestPool.reset(mSpheres.getBufferPtr());
    mAoList.clear();
    mDoList.clear();

    if (mIsRequestDebugSphere)
    {
        sead::Vector3f dir = mDebugDir;
        dir.normalize();
        requestSphere_(mDebugPos, dir, mDebugRadius, mDebugIntensity, mDebugIntensity,
                       mDebugDoLength, cRequestType_Both);
    }
}

/**
 * Releases everything allocated by init().
 */
PrimitiveOcclusion::~PrimitiveOcclusion()
{
    mSphereAoUbo.freeBuffer();
    mSphereDoUbo.freeBuffer();

    for (s32 i = 0; i < mContexts.size(); i++)
    {
        mContexts[i].mUbo.destroy();
    }

    mContexts.freeBuffer();

    if (mSphereDoTable != nullptr)
    {
        delete mSphereDoTable;
        mSphereDoTable = nullptr;
    }

    if (mSphereDoTableBuffer.isValid())
    {
        mSphereDoTableBuffer.deleteGPUMemBlock();
        mSphereDoTableBuffer.invalidate();
    }

    mSphereAttribute.destroy();
    mConeAttribute.destroy();

    if (mSphereDoTableSampler != nullptr)
    {
        delete mSphereDoTableSampler;
    }

    mSphereDoTableSampler = nullptr;

    if (mSphereDoTableRenderBuffer != nullptr)
    {
        delete mSphereDoTableRenderBuffer;
    }

    mSphereDoTableRenderBuffer = nullptr;
}

/**
 * Updates a context from a camera and a projection.
 * @param context context index
 * @param rCamera camera
 * @param rProjection projection
 */
void PrimitiveOcclusion::calcContext(s32 context, const sead::Camera& rCamera,
                                     const sead::Projection& rProjection)
{
    mContexts[context].mCulling.update(rCamera.getMatrix(), rProjection);
    calcContext_(context);
}

/**
 * Derives the camera values of a context and writes its uniform block.
 * @param context context index
 */
void PrimitiveOcclusion::calcContext_(s32 context)
{
    Context& rContext = mContexts[context];
    const cull::ViewFrustumCulling& rCulling = rContext.mCulling;

    rContext.mDepthRatio = 1.0f - rCulling.mNear / rCulling.mFar;
    rContext.mViewMtx = rCulling.mViewMtx;
    rContext.mViewProjMtx.setMul(rCulling.mProjMtx, rCulling.mViewMtx);
    rContext.mRange = rCulling.mFar - rCulling.mNear;
    rContext.mRangeInv = 1.0f / rContext.mRange;
    rContext.mTanHalfFovx = rCulling.mAspect * rCulling.mTanHalfFovy;
    rContext.mSignZ = -1.0f;
    rContext.mScreenOffset.x = rContext.mTanHalfFovx * (rCulling.mOffset.x * 2);
    rContext.mScreenOffset.y = rCulling.mTanHalfFovy * (rCulling.mOffset.y * 2);

    UniformBlock& rUbo = rContext.mUbo;
    rUbo.setCurrentBufferIndex(mBufferIndex);
    rUbo.dcbz(0);
    rUbo.setData(0, &rContext.mViewProjMtx, 0, 4);
    rUbo.setData(1, &rCulling.mViewMtx, 0, 3);
    rUbo.setData(2, &rCulling.mViewInvMtx, 0, 3);
    setDataVec4(rUbo, 3, sead::Vector4f(rCulling.mNear, rContext.mRange, rContext.mRangeInv,
                                        rContext.mDepthRatio),
                0);
    setDataVec4(rUbo, 3, sead::Vector4f(rContext.mTanHalfFovx,
                                        rContext.mSignZ * rCulling.mTanHalfFovy,
                                        rContext.mScreenOffset.x, rContext.mScreenOffset.y),
                1);
    flushUbo(rUbo);
}

/**
 * Updates a context from explicit matrices.
 * @param context context index
 * @param rViewMtx view matrix
 * @param rProjMtx projection matrix
 * @param near near clip distance
 * @param far far clip distance
 * @param fovy vertical field of view
 * @param aspect aspect ratio
 * @param rOffset projection offset
 */
void PrimitiveOcclusion::calcContext(s32 context, const sead::Matrix34f& rViewMtx,
                                     const sead::Matrix44f& rProjMtx, f32 near, f32 far, f32 fovy,
                                     f32 aspect, const sead::Vector2f& rOffset)
{
    mContexts[context].mCulling.update(rViewMtx, rProjMtx, near, far, fovy, aspect, rOffset);
    calcContext_(context);
}

/**
 * Updates a context and writes the uniform blocks of every requested sphere.
 * @param context context index
 * @param pCamera camera
 * @param pProjection projection
 */
void PrimitiveOcclusion::calcView(s32 context, const sead::Camera* pCamera,
                                  const sead::PerspectiveProjection* pProjection)
{
    Context& rContext = mContexts[context];
    rContext.mCulling.update(pCamera->getMatrix(), *pProjection);
    calcContext_(context);

    s32 index = 0;

    for (const SphereOcclusion& rSphere : mAoList)
    {
        UniformBlock* pUbo = mSphereAoUbo.get(index, context);
        const f32 range = rSphere.mRadius * 3.0f;
        pUbo->setCurrentBufferIndex(mBufferIndex);
        pUbo->dcbz(0);

        SphereAo sphereAo;
        rSphere.storeUboStructAo(&sphereAo, rContext.mViewMtx);
        pUbo->setData(0, &sphereAo.mViewPos, 0, 1);
        pUbo->setData(1, &sphereAo.mRadiusSq, 0, 1);

        const f32 scale = (range * mAoRangeScale) * 2;
        const sead::Matrix44f worldMtx(scale, 0.0f, 0.0f, rSphere.mPos.x, 0.0f, scale, 0.0f,
                                       rSphere.mPos.y, 0.0f, 0.0f, scale, rSphere.mPos.z, 0.0f,
                                       0.0f, 0.0f, 1.0f);
        sead::Matrix44f mvpMtx;
        multiplyMtx44(&mvpMtx, rContext.mViewProjMtx, worldMtx);
        pUbo->setData(2, &mvpMtx, 0, 4);
        pUbo->flushCurrentBuffer();
        index++;
    }

    const f32 angle = (mSphereDoTable->getMipSlice(0) == 1 ? *mConeDegreeMin : *mConeDegree) *
                      sead::Mathf::deg2rad(1.0f);
    const f32 sinAngle = sead::Mathf::sin(angle);
    index = 0;

    for (const SphereOcclusion& rSphere : mDoList)
    {
        UniformBlock* pUbo = mSphereDoUbo.get(index, context);
        pUbo->setCurrentBufferIndex(mBufferIndex);
        pUbo->dcbz(0);

        SphereDo sphereDo;
        rSphere.storeUboStructDo(&sphereDo, rContext.mViewMtx);
        pUbo->setData(0, &sphereDo.mViewPos, 0, 1);
        pUbo->setData(1, &sphereDo.mViewDir, 0, 1);
        pUbo->setData(2, &sphereDo.mLengthInv, 0, 1);

        const f32 cosAngle = sead::Mathf::cos(angle);
        const f32 coneScale = 1.0f / cosAngle - 1.0f;
        const f32 offset = rSphere.mRadius / sinAngle + mDoOffset;
        const sead::Vector3f apex(rSphere.mPos.x + offset * rSphere.mDir.x,
                                  rSphere.mPos.y + offset * rSphere.mDir.y,
                                  rSphere.mPos.z + offset * rSphere.mDir.z);
        const f32 length = offset + rSphere.mDoLength;
        const f32 radius = length * sead::Mathf::tan(angle);

        sead::Quatf rotation;

        if (!rotation.makeVectorRotation(sead::Vector3f::ey, rSphere.mDir))
        {
            rotation.setAxisRadian(sead::Vector3f::ex, sead::numbers::pi);
        }

        sead::Matrix34f rotateMtx;
        rotateMtx.fromQuat(rotation);
        const f32 height = length * sead::Mathf::cos(angle);
        rotateMtx.scaleBases(radius * 2, height, radius * 2);
        rotateMtx.setTranslation(apex);

        const sead::Matrix34f coneMtx(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -0.5f, 0.0f, 0.0f,
                                      1.0f, 0.0f);
        sead::Matrix34f worldMtx;
        worldMtx.setMul(rotateMtx, coneMtx);
        sead::Matrix44f mvpMtx;
        mvpMtx.setMul(rContext.mViewProjMtx, worldMtx);
        pUbo->setData(3, &mvpMtx, 0, 4);

        setDataVec4(*pUbo, 4, sead::Vector4f(coneScale, 0.0f, 0.0f, 0.0f), 0);
        pUbo->flushCurrentBuffer();
        index++;
    }
}

/**
 * Draws the ambient occlusion of every visible requested sphere.
 * @param context context index
 * @param rNormal view normal G-buffer
 * @param rDepth depth G-buffer
 * @param isView whether the shadow intensity is output
 * @param shaderMode current shader mode
 * @return the shader mode after drawing
 */
ShaderMode PrimitiveOcclusion::drawAo(s32 context, const TextureData& rNormal,
                                      const TextureData& rDepth, bool isView,
                                      ShaderMode shaderMode) const
{
    const Context& rContext = mContexts[context];
    bool isFirst = true;
    s32 index = 0;

    for (const SphereOcclusion& rSphere : mAoList)
    {
        if (rContext.mCulling.isInside(rSphere.mPos, rSphere.mRadius))
        {
            if (isFirst)
            {
                const char* macro = "IS_OUTPUT_SHADOW_INTENSITY";
                const char* value = "1";

                if (!isView)
                {
                    value = "0";
                }

                const ShaderProgram* pProgram =
                    mSphereAoProgram->searchVariation(1, &macro, &value);
                pProgram->activate(getDrawContext(), true);
                rContext.mUbo.activate(getDrawContext(), mSphereAoContextLocation);
                UniformBlockLocation envLocation("OcclusionEnv");
                envLocation.search(*pProgram);
                mEnvUbo.activate(getDrawContext(), envLocation);
                bindGBuffer(rNormal, rDepth);
                mSphereAttribute.activate(getDrawContext());
                isFirst = false;
            }

            const UniformBlock* pUbo = mSphereAoUbo.get(index, context);
            pUbo->activate(getDrawContext(), mSphereAoViewLocation);
            pfx::detail::drawIndexStream(getDrawContext(),
                                         utl::PrimitiveShape::instance()->getSphereIndexStream(1));
        }

        index++;
    }

    return shaderMode;
}

namespace
{

/**
 * Binds the G-buffer normal and depth textures to the first two sampler slots.
 * @param rNormal view normal texture
 * @param rDepth depth texture
 */
NOINLINE void bindGBuffer(const TextureData& rNormal, const TextureData& rDepth)
{
    SamplerLocation normalLocation(0, "cNormal");
    SamplerLocation depthLocation(1, "cDepth");
    TextureSampler normalSampler(rNormal);
    TextureSampler depthSampler(rDepth);
    normalSampler.activate(getDrawContext(), normalLocation, -1, false);
    depthSampler.activate(getDrawContext(), depthLocation, -1, false);
}

}  // namespace

/**
 * Draws the directional occlusion of every requested sphere.
 * @param context context index
 * @param rNormal view normal G-buffer
 * @param rDepth depth G-buffer
 * @param isView whether the shadow intensity is output
 * @param shaderMode current shader mode
 * @return the shader mode after drawing
 */
ShaderMode PrimitiveOcclusion::drawDo(s32 context, const TextureData& rNormal,
                                      const TextureData& rDepth, bool isView,
                                      ShaderMode shaderMode) const
{
    const Context& rContext = mContexts[context];
    bool isFirst = true;
    s32 index = 0;

    for (const SphereOcclusion& rSphere : mDoList)
    {
        if (isFirst)
        {
            const char* macro = "IS_OUTPUT_SHADOW_INTENSITY";
            const char* value = "1";

            if (!isView)
            {
                value = "0";
            }

            const ShaderProgram* pProgram = mSphereDoProgram->searchVariation(1, &macro, &value);
            pProgram->activate(getDrawContext(), true);
            rContext.mUbo.activate(getDrawContext(), mSphereDoContextLocation);
            UniformBlockLocation envLocation("OcclusionEnv");
            envLocation.search(*pProgram);
            mEnvUbo.activate(getDrawContext(), envLocation);
            bindGBuffer(rNormal, rDepth);
            SamplerLocation tableLocation(2, "cSphereDoTable");
            mSphereDoTableSampler->activate(getDrawContext(), tableLocation, -1, false);
            mConeAttribute.activate(getDrawContext());
        }

        const UniformBlock* pUbo = mSphereDoUbo.get(index, context);
        pUbo->activate(getDrawContext(), mSphereDoViewLocation);
        pfx::detail::drawIndexStream(
            getDrawContext(), utl::PrimitiveShape::instance()->getConeTriangleIndexStream(1));
        isFirst = false;
        index++;
    }

    return shaderMode;
}

/**
 * Draws debug shapes for the requested spheres.
 * @param context context index
 * @param shaderMode current shader mode
 * @return the shader mode after drawing
 */
ShaderMode PrimitiveOcclusion::drawDebug(s32 context, ShaderMode shaderMode) const
{
    const Context& rContext = mContexts[context];

    if (mIsDrawDebugSphere)
    {
        const SphereOcclusion* pSpheres = mSpheres.getBufferPtr();
        const u32 num = mRequestPool.mNum;

        for (u32 i = 0; i < num; i++)
        {
            utl::DevTools::drawPointLight(getDrawContext(), pSpheres[i].mPos, pSpheres[i].mRadius,
                                          sead::Color4f::cBlack, rContext.mCulling.mViewMtx,
                                          rContext.mCulling.mProjMtx);
        }
    }

    if (mIsDrawDebugAo)
    {
        for (const SphereOcclusion& rSphere : mAoList)
        {
            utl::DevTools::drawPointLight(getDrawContext(), rSphere.mPos,
                                          rSphere.mRadius * 3.0f * mAoRangeScale,
                                          sead::Color4f::cBlack, rContext.mCulling.mViewMtx,
                                          rContext.mCulling.mProjMtx);
        }
    }

    const f32 angle = (mSphereDoTable->getMipSlice(0) == 1 ? *mConeDegreeMin : *mConeDegree) *
                      sead::Mathf::deg2rad(1.0f);
    const f32 sinAngle = sead::Mathf::sin(angle);

    if (mIsDrawDebugDo)
    {
        for (const SphereOcclusion& rSphere : mDoList)
        {
            const f32 offset = rSphere.mRadius / sinAngle + mDoOffset;
            const f32 length = offset + rSphere.mDoLength;
            const sead::Vector3f apex(offset * rSphere.mDir.x + rSphere.mPos.x,
                                      offset * rSphere.mDir.y + rSphere.mPos.y,
                                      offset * rSphere.mDir.z + rSphere.mPos.z);
            const f32 height = length * sead::Mathf::cos(angle);
            const sead::Vector3f dir(-rSphere.mDir.x, -rSphere.mDir.y, -rSphere.mDir.z);
            utl::DevTools::drawSpotLight(getDrawContext(), apex, dir, sead::Color4f::cBlack, angle,
                                         height, rContext.mCulling.mViewMtx,
                                          rContext.mCulling.mProjMtx);
        }
    }

    return shaderMode;
}

/**
 * Bakes the directional occlusion table, one slice per cone angle, when requested.
 * @param context context index (only context 0 bakes)
 * @param shaderMode current shader mode
 * @return the shader mode after drawing
 */
ShaderMode PrimitiveOcclusion::drawPrecomputeSphereDo(s32 context, ShaderMode shaderMode) const
{
    if (context != 0)
    {
        return shaderMode;
    }

    if (!mMakeTableRequest.isRequested())
    {
        return shaderMode;
    }

    mMakeTableSphereDoProgram->activate(getDrawContext(), true);
    utl::VertexAttributeHolder::instance()
        ->getVertexAttribute(utl::VertexAttributeHolder::cAttribute_QuadTriangleTexCoord)
        .activate(getDrawContext());

    sead::GraphicsContext graphicsContext;
    graphicsContext.setDepthEnable(false, false);
    graphicsContext.setColorMask(true, true, true, true);
    graphicsContext.setBlendEnable(false);
    graphicsContext.apply(getDrawContext());

    UniformLocation tanLocation("uTanConeAngle");
    tanLocation.search(*mMakeTableSphereDoProgram);

    const s32 sliceNum = mSphereDoTable->getMipSlice(0);

    for (s32 i = 0; i < sliceNum; i++)
    {
        const f32 rate = sead::Mathf::clamp(f32(i) / f32(sliceNum), 0.0f, 1.0f);
        const f32 degree = (1.0 - rate) * *mConeDegreeMin + *mConeDegreeMax * rate;
        tanLocation.setUniform(getDrawContext(),
                               sead::Mathf::tan(degree * sead::Mathf::deg2rad(1.0f) * 0.5f));

        RenderTargetBinder binder(mSphereDoTableRenderBuffer, mSphereDoTable, i);
        const sead::Viewport viewport(*binder.getRenderBuffer());
        viewport.apply(getDrawContext(), *binder.getRenderBuffer());
        binder.getRenderBuffer()->bind(getDrawContext());
        pfx::detail::drawIndexStream(getDrawContext(),
                                     utl::PrimitiveShape::instance()->getQuadTriangleIndexStream());
    }

    return shaderMode;
}

/**
 * Requests a sphere for this frame.
 * @param rPos center of the sphere
 * @param rDir direction of the directional occlusion cone
 * @param radius radius of the sphere
 * @param aoIntensity ambient occlusion intensity
 * @param doIntensity directional occlusion intensity
 * @param doLength directional occlusion length
 * @param type which occlusion the sphere casts
 */
void PrimitiveOcclusion::requestSphereOpp(const sead::Vector3f& rPos, const sead::Vector3f& rDir,
                                          f32 radius, f32 aoIntensity, f32 doIntensity,
                                          f32 doLength, RequestType type)
{
    requestSphere_(rPos, rDir, radius, aoIntensity, doIntensity, doLength, type);
}

/**
 * Generates the host IO messages of the parameters.
 * @param pContext host IO context
 */
void PrimitiveOcclusion::genMessage(sead::hostio::Context* pContext)
{
    mConeDegreeMin.genMessageParameter(pContext, mConeDegreeMin.getMeta());
    mConeDegreeMax.genMessageParameter(pContext, mConeDegreeMax.getMeta());
    mConeDegree.genMessageParameter(pContext, mConeDegree.getMeta());
}

/**
 * Requests a rebake of the directional occlusion table when a parameter changed.
 * @param pEvent property event
 */
void PrimitiveOcclusion::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent)
{
    if (pEvent->getIdValue() == 1000)
    {
        mMakeTableRequest.requestNext();
    }
}

}  // namespace agl::sdw
