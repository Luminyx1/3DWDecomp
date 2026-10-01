#include "effect/aglOcclusionRenderer.h"

#include <gfx/seadCamera.h>
#include <hostio/seadHostIOPropertyEvent.h>
#include <math/seadMathCalcCommon.h>
#include <nvn/nvn_FuncPtrInline.h>
#include <prim/seadSafeString.h>

#include "common/aglDrawContext.h"
#include "common/aglShaderProgram.h"
#include "detail/aglShaderHolder.h"
#include "effect/aglGPUCache.h"
#include "postfx/aglPostFxUtil.h"
#include "utility/aglDevTools.h"

namespace agl::fx {

namespace {

template <typename T>
inline T* getBlockPtr(const GPUMemBlockBase& rBlock)
{
    return reinterpret_cast<T*>(
        static_cast<u8*>(nvnMemoryPoolMap(rBlock.getMemoryPool()->getDriverPool())) +
        rBlock.getByteOffset());
}

inline bool isNearZero(const sead::Vector3f& rDir)
{
    return rDir.x <= 0.1f && rDir.x >= -0.1f && rDir.y <= 0.1f && rDir.y >= -0.1f &&
           rDir.z <= 0.1f && rDir.z >= -0.1f;
}

inline void calcOccluderMtx(sead::Matrix34f* pMtx, const sead::Matrix34f& rCamInv,
                            const sead::Vector3f& rPos, f32 radius)
{
    sead::Matrix34f rot;
    rot.setMul(sead::Matrix34f::ident, rCamInv);

    for (s32 i = 0; i < 3; i++)
    {
        (*pMtx)(i, 0) = radius * rot(i, 0);
        (*pMtx)(i, 1) = radius * rot(i, 1);
        (*pMtx)(i, 2) = radius * rot(i, 2);
    }

    (*pMtx)(0, 3) = rPos.x + rot(0, 3);
    (*pMtx)(1, 3) = rPos.y + rot(1, 3);
    (*pMtx)(2, 3) = rPos.z + rot(2, 3);
}

inline sead::Vector3f projectPos(const sead::Matrix44f& rMtx, const sead::Vector3f& rPos)
{
    f32 invW = 1.0f / (rMtx(3, 3) + (rMtx(3, 0) * rPos.x + rMtx(3, 1) * rPos.y +
                                     rMtx(3, 2) * rPos.z));
    return sead::Vector3f(
        invW * (rMtx(0, 3) + (rMtx(0, 0) * rPos.x + rMtx(0, 1) * rPos.y + rMtx(0, 2) * rPos.z)),
        invW * (rMtx(1, 3) + (rMtx(1, 0) * rPos.x + rMtx(1, 1) * rPos.y + rMtx(1, 2) * rPos.z)),
        invW * (rMtx(2, 3) + (rMtx(2, 0) * rPos.x + rMtx(2, 1) * rPos.y + rMtx(2, 2) * rPos.z)));
}

}  // namespace

OcclusionRenderer::OcclusionRenderer()
{
    mClearGraphicsContext.setColorMask(0xff);
    mDrawGraphicsContext.setDepthEnable(false, false);
    mDrawGraphicsContext.setBlendEnable(0, true);
    mDrawGraphicsContext.setBlendEnable(1, true);
    mDrawGraphicsContext.setBlendFactor(0, 2, 2);
    mDrawGraphicsContext.setBlendFactor(1, 2, 2);
    mDrawGraphicsContext.setBlendEquation(0, 1);
    mDrawGraphicsContext.setBlendEquation(1, 1);
    mClearGraphicsContext.setDepthEnable(false, false);
    mClearGraphicsContext.setBlendEnable(0, false);
    mClearGraphicsContext.setBlendEnable(1, false);
}

/**
 * Frees the contexts and their occlusion textures.
 */
OcclusionRenderer::~OcclusionRenderer()
{
    for (auto& rContext : mContext)
    {
        for (auto& rSub : rContext.mSub)
        {
            delete rSub.mTexture;
            rSub.mTexture = nullptr;
            rSub.mAddr.deleteGPUMemBlock();
        }

        rContext.mSub.freeBuffer();
    }

    mContext.freeBuffer();
    mDebugTexturePage.cleanUp();
}

/**
 * Reads back the occlusion rate of one context.
 * @param index context index
 * @return visible rate
 */
f32 OcclusionRenderer::getOcclusionRate(s32 index) const
{
    GPUMemVoidAddr(mContext[index].mSub.front().mAddr, 0).flushCPUCache(4);
    const GPUMemVoidAddr& rAddr = mContext[index].mSub.front().mAddr;
    return *reinterpret_cast<const f32*>(
        static_cast<u8*>(nvnMemoryPoolMap(rAddr.getMemoryPool()->getDriverPool())) +
        rAddr.getByteOffset());
}

/**
 * Reads back the occlusion rate of the core of one context.
 * @param index context index
 * @return visible rate of the core
 */
f32 OcclusionRenderer::getCoreOcclusionRate(s32 index) const
{
    const f32* pRate = static_cast<const f32*>(mContext[index].mSub.front().mAddr.getPtr());
    GPUMemVoidAddr(mContext[index].mSub.front().mAddr, 4).flushCPUCache(4);
    return pRate[1];
}

void OcclusionRenderer::initialize(s32 contextNum, sead::Heap* pHeap)
{
    mContext.tryAllocBuffer(contextNum, pHeap);

    for (auto it = mContext.begin(); it != mContext.end(); ++it)
    {
        Context& rContext = *it;
        rContext.mOffset.set(0.0f, 0.0f, 0.0f);
        rContext.mPos.set(0.0f, 0.0f, 0.0f);

        if (it.getIndex() == 0)
        {
            rContext.mUniformBlock.startDeclare(16, pHeap);
            rContext.mUniformBlock.declare(UniformBlock::cType_Vec4, 4);
            rContext.mUniformBlock.declare(UniformBlock::cType_Vec4, 3);
            rContext.mUniformBlock.declare(UniformBlock::cType_Vec4, 3);
            rContext.mUniformBlock.declare(UniformBlock::cType_Vec4, 4);

            for (s32 i = 0; i < 12; i++)
            {
                rContext.mUniformBlock.declare(UniformBlock::cType_Float, 1);
            }
        }
        else
        {
            rContext.mUniformBlock.declare(mContext.front().mUniformBlock);
        }

        rContext.mUniformBlock.create(pHeap, 2, 1);

        rContext.mSampler.setBorderColor(sead::Color4f(1.0f, 1.0f, 1.0f, 1.0f));
        rContext.mSampler.setWrap(5, 5, 5);

        rContext.mRenderBuffer.setRenderTargetColorNullAll();
        rContext.mRenderBuffer.setRenderTargetDepth(nullptr);

        rContext.mSub.tryAllocBuffer(2, pHeap);

        for (auto itSub = rContext.mSub.begin(); itSub != rContext.mSub.end(); ++itSub)
        {
            SubContext& rSub = *itSub;
            rSub.mTexture = new (pHeap, 8) TextureData;
            rSub.mTexture->initialize_(TextureType(1), TextureFormat(0x2e), 1, 1, 1, 1,
                                       TextureAttribute(1), MultiSampleType(0), true);
            u32 alignment = rSub.mTexture->getAlignment();
            u32 size = rSub.mTexture->getImageByteSize();
            GPUMemBlock<u8>* pBlock = new (pHeap, 8) GPUMemBlock<u8>;
            pBlock->allocBuffer_(size, pHeap, alignment, MemoryAttribute(0));
            rSub.mAddr = GPUMemVoidAddr(*pBlock, 0);
            rSub.mTexture->setImagePtr(rSub.mAddr, 0);
            memset(rSub.mTexture->getImagePtr().getPtr(), 0, rSub.mTexture->getImageByteSize());
            rSub.mTexture->flushCPUCache();

            rSub.mSampler.applyTextureData(*rSub.mTexture);
            rSub.mSampler.setFilter(0, 0, 1);
            rSub.mSampler.setWrap(1, 1, 1);
            rSub.mRenderTarget.applyTextureData(*rSub.mTexture);
            rContext.mRenderBuffer.setRenderTargetColor(&rSub.mRenderTarget, itSub.getIndex());
        }

        rContext.mRenderBuffer.adjustPhysicalAreaAndVirtualSizeFromColorTarget(0);
        rContext.mViewport.setByFrameBuffer(rContext.mRenderBuffer);
    }

    mOcclVtxStream.initialize(0x100, 8, mAngle * sead::Mathf::pi(), pHeap);
    mOcclVtxStream.create(mRingNum, mDivNum, mAngle * sead::Mathf::pi());
    mClearBufVtxStream.initialize(pHeap);

    mDebugTexturePage.setUp(2, "OcclusionRenderer", pHeap);
}

/**
 * Allocates the sample buffers and fills them.
 * @param num number of samples
 * @param divNum number of samples per turn
 * @param angle angle step in radians
 * @param pHeap heap used for allocations
 */
void OcclusionRenderer::OcclVtxStream::initialize(s32 num, s32 divNum, f32 angle,
                                                  sead::Heap* pHeap)
{
    mNum = num;
    mVertexBlock.allocBuffer(num, pHeap, 8, MemoryAttribute(0));
    GPUMemAddr<OcclVtx> vtxAddr(mVertexBlock, 0);
    mIndexBlock.allocBuffer(num, pHeap, 8, MemoryAttribute(0));
    GPUMemAddr<u16> idxAddr(mIndexBlock, 0);
    mVertexAttribute.create(3, pHeap);
    create(num, divNum, angle);
}

void OcclusionRenderer::OcclVtxStream::create(s32 num, s32 divNum, f32 angle)
{
    s32 total = divNum * num;
    f32 invTotal = 1.0f / f32(total);

    if (num >= 1)
    {
        f32 angleStep = f32(divNum) * angle;
        f32 rateStep = invTotal * f32(divNum);

        for (s32 i = 0; i < num; i++)
        {
            f32 fi = f32(i);
            f32 a = angleStep * fi;
            OcclVtx* pVtx = getBlockPtr<OcclVtx>(mVertexBlock) + i;
            pVtx->mRate = rateStep * fi;
            pVtx->mCos = sead::Mathf::cos(a);
            pVtx->mSin = sead::Mathf::sin(a);
            getBlockPtr<u16>(mIndexBlock)[i] = i;
        }
    }

    mScale = 0.0f;
    f32 s = sead::Mathf::sin(angle);
    f32 c = sead::Mathf::cos(angle);
    f32 x = 1.0f;
    f32 y = 0.0f;

    for (s32 i = 0; i < total; i++)
    {
        f32 nx = c * x - s * y;
        f32 ny = c * y + s * x;

        if (nx > 0.0f)
        {
            mScale += invTotal * nx * f32(i);
        }

        x = nx;
        y = ny;
    }

    mScale = 1.0f / mScale;

    mVertexBuffer.setUpBuffer(ConstGPUMemVoidAddr(mVertexBlock, 0), sizeof(OcclVtx),
                              sizeof(OcclVtx) * num);
    mVertexBuffer.setUpStream(0, VertexStreamFormat(10), 0, false);
    mVertexBuffer.setUpStream(1, VertexStreamFormat(10), 4, false);
    mVertexBuffer.setUpStream(2, VertexStreamFormat(10), 8, false);
    mVertexAttribute.setVertexStream(0, &mVertexBuffer, 0);
    mVertexAttribute.setVertexStream(1, &mVertexBuffer, 1);
    mVertexAttribute.setVertexStream(2, &mVertexBuffer, 2);
    mVertexAttribute.setUp();
    mIndexStream.setUpStream(GPUMemAddr<u16>(mIndexBlock, 0), num);
    mIndexStream.setPrimitiveType(NVN_DRAW_PRIMITIVE_POINTS);
}

/**
 * Allocates the single point used to clear the result.
 * @param pHeap heap used for allocations
 */
void OcclusionRenderer::ClearBufVtxStream::initialize(sead::Heap* pHeap)
{
    mVertexBlock.allocBuffer(1, pHeap, 4, MemoryAttribute(0));
    GPUMemAddr<u32> vtxAddr(mVertexBlock, 0);
    mIndexBlock.allocBuffer(1, pHeap, 4, MemoryAttribute(0));
    GPUMemAddr<u16> idxAddr(mIndexBlock, 0);
    mVertexAttribute.create(1, pHeap);
    getBlockPtr<u32>(mVertexBlock)[0] = 0;
    mVertexBuffer.setUpBuffer(ConstGPUMemVoidAddr(mVertexBlock, 0), sizeof(u32), sizeof(u32));
    mVertexBuffer.setUpStream(0, VertexStreamFormat(11), 0, false);
    mVertexAttribute.setVertexStream(0, &mVertexBuffer, 0);
    mVertexAttribute.setUp();
    mIndexStream.setUpStream(GPUMemAddr<u16>(mIndexBlock, 0), 1);
    mIndexStream.setPrimitiveType(NVN_DRAW_PRIMITIVE_POINTS);
}

/**
 * Flips the buffer index while enabled.
 */
void OcclusionRenderer::calc()
{
    if (mEnable)
    {
        mBufferIndex = 1 - mBufferIndex;
    }
}

void OcclusionRenderer::calcContext(s32 index, const sead::Matrix34f& rView,
                                    const sead::Matrix44f& rProj, f32 near, f32 far, f32 fovy,
                                    f32 aspect, const sead::Vector2f& rOffset,
                                    CalcResult* pResult)
{
    if (!mEnable)
    {
        return;
    }

    Context& rContext = mContext[index];
    cull::ViewFrustumCulling& rCulling = rContext.mViewFrustumCulling;
    sead::Vector3f pos(mOffset.x + rContext.mOffset.x, mOffset.y + rContext.mOffset.y,
                       mOffset.z + rContext.mOffset.z);
    rCulling.update(rView, rProj, near, far, fovy, aspect, rOffset);

    sead::Matrix34f invView = rCulling.getViewMtx();
    invView.invert();
    sead::Vector3f dir;
    dir.setRotated(invView, -sead::Vector3f::ez);

    if (isNearZero(dir))
    {
        return;
    }

    sead::LookAtCamera camera(sead::Vector3f::zero, dir, sead::Vector3f::ey);
    camera.updateViewMatrix();
    sead::Matrix34f camInv = camera.getMatrix();
    camInv.invert();

    calcOccluderMtx(&rContext.mOccluderMtx, camInv, pos, mSize * 0.5f);
    rContext.mViewMtx = rCulling.getViewMtx();
    rContext.mProjMtx = rCulling.getProjMtx();
    sead::Matrix34f viewOccluder;
    viewOccluder.setMul(rContext.mViewMtx, rContext.mOccluderMtx);
    rContext.mOccluderProjMtx.setMul(rContext.mProjMtx, viewOccluder);

    f32 angle = mAngle * sead::Mathf::pi();
    rContext.mSin = sead::Mathf::sin(angle);
    rContext.mCos = sead::Mathf::cos(angle);
    rContext.mSampleRate = 1.0f / f32(mRingNum * mDivNum);
    rContext.mSampleScale = 1.0f / ((mSampleSize * f32(mRingNum * mDivNum)) / mSize);

    sead::Vector3f center = projectPos(rContext.mOccluderProjMtx, sead::Vector3f::zero);
    sead::Vector3f edge = projectPos(rContext.mOccluderProjMtx, sead::Vector3f(1.0f, 0.0f, 0.0f));
    f32 length = (edge - center).length();
    rContext.mScreenRadius = length * 0.5f;
    rContext.mAspect = mSampleSize / mSize;
    rContext.mRadiusScale = mOcclVtxStream.mScale / rContext.mScreenRadius;

    if (pResult != nullptr)
    {
        pResult->mScreenPos = center;
        pResult->mDepth = length;
    }

    rContext.mPos = pos;

    if (mAutoDirection)
    {
        sead::Vector3f viewPos(viewOccluder(0, 3), viewOccluder(1, 3), viewOccluder(2, 3));
        f32 dist = viewPos.length();

        if (viewPos.z < 0.0f)
        {
            f32 tanHalf = rCulling.mTanHalfFovy;
            f32 aspectRatio = rCulling.mAspect;
            f32 depth = -viewPos.z;
            f32 halfH = tanHalf * depth;
            f32 halfW = halfH * aspectRatio;
            f32 zz = viewPos.z * viewPos.z;
            f32 marginW = (mSize * 0.5f) * sead::Mathf::sqrt(zz + halfW * halfW) / depth;
            f32 marginH = (mSize * 0.5f) * sead::Mathf::sqrt(zz + halfH * halfH) / depth;

            f32 scale = 1.0f;
            f32 absY = viewPos.y > 0.0f ? viewPos.y : -viewPos.y;

            if (halfH - marginH < absY)
            {
                scale = halfH / absY;
            }

            f32 absX = viewPos.x > 0.0f ? viewPos.x : -viewPos.x;

            if (halfW - marginW < absX)
            {
                f32 s = halfW / absX;

                if (scale > s)
                {
                    scale = s;
                }
            }

            sead::Vector3f n(viewPos.x * scale, viewPos.y * scale, viewPos.z);
            f32 len = n.length();

            if (len > 0.0f)
            {
                n *= 1.0f / len;
            }

            f32 x = dist * n.x;
            f32 y = dist * n.y;
            f32 z = dist * n.z;
            f32 halfH2 = tanHalf * -z;
            f32 halfW2 = aspectRatio * halfH2;
            f32 zz2 = z * z;
            f32 marginW2 = (mSize * 0.5f) * sead::Mathf::sqrt(zz2 + halfW2 * halfW2) / -z;
            f32 marginH2 = (mSize * 0.5f) * sead::Mathf::sqrt(zz2 + halfH2 * halfH2) / z;

            f32 absY2 = y > 0.0f ? y : -y;
            f32 limitH = halfH2 + marginH2;

            if (limitH < absY2)
            {
                f32 s = (absY2 - (absY2 - limitH)) / absY2;
                x *= s;
                y *= s;
            }

            f32 absX2 = x > 0.0f ? x : -x;
            f32 limitW = halfW2 - marginW2;

            if (limitW < absX2)
            {
                f32 s = (absX2 - (absX2 - limitW)) / absX2;
                y *= s;
                x *= s;
            }

            sead::Vector3f m(x, y, z);
            f32 len2 = m.length();

            if (len2 > 0.0f)
            {
                m *= 1.0f / len2;
            }

            sead::Vector3f viewTarget = m * dist;
            const sead::Matrix34f& rInv = rCulling.getViewInvMtx();
            rContext.mPos.x = rInv(0, 3) + (rInv(0, 0) * viewTarget.x + rInv(0, 1) * viewTarget.y +
                                            rInv(0, 2) * viewTarget.z);
            rContext.mPos.y = rInv(1, 3) + (rInv(1, 0) * viewTarget.x + rInv(1, 1) * viewTarget.y +
                                            rInv(1, 2) * viewTarget.z);
            rContext.mPos.z = rInv(2, 3) + (rInv(2, 0) * viewTarget.x + rInv(2, 1) * viewTarget.y +
                                            rInv(2, 2) * viewTarget.z);

            calcOccluderMtx(&rContext.mOccluderMtx, camInv, rContext.mPos, mSize * 0.5f);
            rContext.mViewMtx = rCulling.getViewMtx();
            rContext.mProjMtx = rCulling.getProjMtx();
            viewOccluder.setMul(rContext.mViewMtx, rContext.mOccluderMtx);
            rContext.mOccluderProjMtx.setMul(rContext.mProjMtx, viewOccluder);
        }
    }

    rContext.mUniformBlock.setCurrentBufferIndex(mBufferIndex);
}

/**
 * Does nothing.
 */
void OcclusionRenderer::updateGPU() {}

void OcclusionRenderer::updateViewGPU(s32 index, const RenderBuffer& rRenderBuffer)
{
    if (!mEnable)
    {
        return;
    }

    const Context& rContext = mContext[index];
    const UniformBlock& rBlock = rContext.mUniformBlock;
    const cull::ViewFrustumCulling& rCulling = rContext.mViewFrustumCulling;
    rBlock.dcbz(0);
    rBlock.setData(0, &rContext.mOccluderProjMtx, 0, 4);
    rBlock.setData(1, &rContext.mOccluderMtx, 0, 3);
    rBlock.setData(2, &rContext.mViewMtx, 0, 3);
    rBlock.setData(3, &rContext.mProjMtx, 0, 4);
    {
        f32 value = rCulling.mNear;
        rBlock.setData(4, &value, 0, 1);
    }

    {
        f32 value = rCulling.mFar - rCulling.mNear;
        rBlock.setData(5, &value, 0, 1);
    }

    {
        f32 value = rCulling.mAspect;
        rBlock.setData(6, &value, 0, 1);
    }

    {
        f32 value = mPower;
        rBlock.setData(7, &value, 0, 1);
    }

    {
        f32 value = -mThreshold;
        rBlock.setData(8, &value, 0, 1);
    }

    {
        f32 value = rContext.mSin;
        rBlock.setData(9, &value, 0, 1);
    }

    {
        f32 value = rContext.mCos;
        rBlock.setData(10, &value, 0, 1);
    }

    {
        f32 value = rContext.mScreenRadius;
        rBlock.setData(11, &value, 0, 1);
    }

    {
        f32 value = rContext.mAspect;
        rBlock.setData(12, &value, 0, 1);
    }

    {
        f32 value = rContext.mRadiusScale;
        rBlock.setData(15, &value, 0, 1);
    }

    {
        f32 value = rContext.mSampleRate;
        rBlock.setData(13, &value, 0, 1);
    }

    {
        f32 value = rContext.mSampleScale;
        rBlock.setData(14, &value, 0, 1);
    }

    rBlock.flushCurrentBuffer();
}

/**
 * Renders the occlusion samples of one context against a depth target.
 * @param pDrawContext draw context that receives the commands
 * @param index context index
 * @param rDepth scene depth target
 */
void OcclusionRenderer::draw(DrawContext* pDrawContext, s32 index,
                             const RenderTargetDepth& rDepth) const
{
    if (!mEnable)
    {
        return;
    }

    const ShaderProgram* pProgram =
        detail::ShaderHolder::instance()->getShaderProgram(detail::ShaderHolder::cOcclusionRenderer);
    const Context& rContext = mContext[index];

    mClearGraphicsContext.apply(pDrawContext);
    rContext.mRenderBuffer.bind(pDrawContext);
    rContext.mViewport.apply(pDrawContext, rContext.mRenderBuffer);
    rContext.mRenderBuffer.fastClear(pDrawContext, 0, 1, sead::Color4f::cBlack, 0.0f, 0,
                                     sead::Viewport(rContext.mRenderBuffer), true);
    if (mUseCore)
    {
        rContext.mRenderBuffer.fastClear(pDrawContext, 1, 1, sead::Color4f::cBlack, 0.0f, 0,
                                         sead::Viewport(rContext.mRenderBuffer), true);
    }

    for (const auto& rSub : rContext.mSub)
    {
        rSub.mRenderTarget.invalidateGPUCache(pDrawContext);
    }

    mDrawGraphicsContext.apply(pDrawContext);
    TextureSampler& rSampler = const_cast<TextureSampler&>(rContext.mSampler);
    rSampler.applyTextureData(rDepth);

    if (mBorderBlack)
    {
        rSampler.setBorderColor(sead::Color4f(0.0f, 0.0f, 0.0f, 1.0f));
    }
    else
    {
        rSampler.setBorderColor(sead::Color4f(1.0f, 1.0f, 1.0f, 1.0f));
    }

    rContext.mRenderBuffer.bind(pDrawContext);
    rContext.mViewport.apply(pDrawContext, rContext.mRenderBuffer);

    const ShaderProgram* pVariation = pProgram->getVariation(u8(mUseCore | mUseSoft << 1));
    pVariation->activate(pDrawContext, true);
    rContext.mUniformBlock.activate(pDrawContext, pVariation->getUniformBlockLocation(0));
    rContext.mSampler.activate(pDrawContext, pVariation->getSamplerLocation(0), -1, false);
    mOcclVtxStream.mVertexAttribute.activate(pDrawContext);
    pfx::detail::drawIndexStream(pDrawContext, mOcclVtxStream.mIndexStream);

    for (const auto& rSub : rContext.mSub)
    {
        rSub.mRenderTarget.invalidateGPUCache(pDrawContext);
    }

    GPUCache::invalidateAll(pDrawContext);
}

/**
 * Does nothing.
 * @param pDrawContext draw context that receives the commands
 * @param index context index
 */
void OcclusionRenderer::release(DrawContext* pDrawContext, s32 index) const {}

void OcclusionRenderer::drawDebug(DrawContext* pDrawContext, s32 index,
                                  const sead::Color4f& rColor0, const sead::Color4f& rColor1,
                                  const sead::Color4f& rColor2) const
{
    if (!mEnable)
    {
        return;
    }

    const Context& rContext = mContext[index];

    if (mAutoDirection)
    {
        utl::DevTools::drawPointLight(pDrawContext, rContext.mPos, mSize * 0.5f, rColor0,
                                      rContext.mViewFrustumCulling.getViewMtx(),
                                      rContext.mViewFrustumCulling.getProjMtx());
    }

    {
        sead::Vector3f pos(mOffset.x + rContext.mOffset.x, mOffset.y + rContext.mOffset.y,
                           mOffset.z + rContext.mOffset.z);
        utl::DevTools::drawPointLight(pDrawContext, pos, mSize * 0.5f, rColor0,
                                      mContext[index].mViewFrustumCulling.getViewMtx(),
                                      mContext[index].mViewFrustumCulling.getProjMtx());
    }

    sead::Vector3f pos(mOffset.x + rContext.mOffset.x, mOffset.y + rContext.mOffset.y,
                       mOffset.z + rContext.mOffset.z);
    utl::DevTools::drawPointLight(pDrawContext, pos, mSampleSize * 0.5f, rColor1,
                                  mContext[index].mViewFrustumCulling.getViewMtx(),
                                  mContext[index].mViewFrustumCulling.getProjMtx());
    utl::DevTools::beginDrawImm(pDrawContext, mContext[index].mViewFrustumCulling.getViewMtx(),
                                mContext[index].mViewFrustumCulling.getProjMtx());

    sead::Matrix34f invView = mContext[index].mViewFrustumCulling.getViewMtx();
    invView.invert();
    sead::Vector3f dir;
    dir.setRotated(invView, -sead::Vector3f::ez);

    if (isNearZero(dir))
    {
        return;
    }

    sead::LookAtCamera camera(sead::Vector3f::zero, dir, sead::Vector3f::ey);
    camera.updateViewMatrix();
    sead::Matrix34f camInv = camera.getMatrix();
    camInv.invert();

    sead::Vector3f center(mOffset.x + rContext.mOffset.x, mOffset.y + rContext.mOffset.y,
                          mOffset.z + rContext.mOffset.z);
    sead::Matrix34f mtx;
    calcOccluderMtx(&mtx, camInv, center, mSize * 0.5f);

    f32 invTotal = 1.0f / f32(mRingNum * mDivNum);

    for (s32 ring = 0; ring < mRingNum; ring++)
    {
        f32 step = mAngle * sead::Mathf::pi();
        f32 s = sead::Mathf::sin(step);
        f32 c = sead::Mathf::cos(step);
        f32 start = mAngle * sead::Mathf::pi() * f32(mDivNum * ring);
        f32 s0 = sead::Mathf::sin(start);
        f32 c0 = sead::Mathf::cos(start);
        f32 base = invTotal * f32(mDivNum) * f32(ring);
        f32 x = c0;
        f32 y = s0;

        for (s32 i = 0; i < mDivNum; i++)
        {
            f32 nx = c * x - s * y;
            f32 ny = s * x + c * y;
            x = nx;
            y = ny;
            f32 rate = powf(base + invTotal * f32(i), mPower);
            sead::Vector3f local(x * rate, y * rate, 0.0f * rate);
            sead::Vector3f world;
            world.setMul(mtx, local);
            utl::DevTools::drawPointImm(pDrawContext, world, rColor2, 1.0f);
        }
    }
}

/**
 * Generates the host IO messages.
 * @param pContext host IO context
 */
void OcclusionRenderer::genMessage(sead::hostio::Context* pContext)
{
    mDebugTexturePage.genMessagePage(pContext, this);
}

void OcclusionRenderer::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent)
{
    if ((pEvent->getType() & 2) != 0)
    {
        return;
    }

    const void* id = pEvent->getId();

    if ((id < &mRingNum + 1 && id >= &mRingNum) || (id < &mDivNum + 1 && id >= &mDivNum) ||
        (id < &mAngle + 1 && id >= &mAngle))
    {
        mOcclVtxStream.create(mRingNum, mDivNum, mAngle * sead::Mathf::pi());
    }
}

}  // namespace agl::fx
