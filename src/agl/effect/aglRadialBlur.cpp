#include "effect/aglRadialBlur.h"

#include <gfx/seadGraphicsContext.h>
#include <gfx/seadViewport.h>
#include <math/seadMathCalcCommon.h>
#include <prim/seadSafeString.h>

#include "common/aglShaderProgram.h"
#include "detail/aglShaderHolder.h"
#include "postfx/aglPostFxUtil.h"
#include "utility/aglDynamicTextureAllocator.h"
#include "utility/aglImageFilter2D.h"
#include "utility/aglPrimitiveShape.h"
#include "utility/aglVertexAttributeHolder.h"

namespace agl::fx {

namespace {

template <typename T>
inline T* getBlockPtr(const GPUMemBlockBase& rBlock)
{
    return reinterpret_cast<T*>(
        static_cast<u8*>(nvnMemoryPoolMap(rBlock.getMemoryPool()->getDriverPool())) +
        rBlock.getByteOffset());
}

inline sead::Vector3f projectPos(const sead::Matrix44f& rMtx, const sead::Vector3f& rPos)
{
    f32 invW = 1.0f / (rMtx(3, 0) * rPos.x + rMtx(3, 1) * rPos.y + rMtx(3, 2) * rPos.z +
                       rMtx(3, 3));
    return sead::Vector3f(
        (rMtx(0, 0) * rPos.x + rMtx(0, 1) * rPos.y + rMtx(0, 2) * rPos.z + rMtx(0, 3)) * invW,
        (rMtx(1, 0) * rPos.x + rMtx(1, 1) * rPos.y + rMtx(1, 2) * rPos.z + rMtx(1, 3)) * invW,
        (rMtx(2, 0) * rPos.x + rMtx(2, 1) * rPos.y + rMtx(2, 2) * rPos.z + rMtx(2, 3)) * invW);
}

}  // namespace

/**
 * Sets up a blur parameter with the default look.
 */
RadialBlur::BlurParameter::BlurParameter()
    : mCenter(sead::Vector3f::zero), mRadius(10.0f), mPower(0.2f), mDepthTestEnable(true),
      mDepthOffset(0.0f), mBlendEnable(true), mBlendFactorSrc(5), mBlendFactorDst(6),
      mBlendEquation(1), mReduceScale(0.5f), mSampleNum(8)
{
    mColor0 = sead::Color4f::cWhite;
    mColor1 = sead::Color4f::cWhite;
    mColor2 = sead::Color4f::cWhite;
    mColor3 = sead::Color4f(sead::Color4f::cWhite.r, sead::Color4f::cWhite.g,
                            sead::Color4f::cWhite.b, 0.0f);
}

/**
 * Constructs an uninitialized radial blur.
 */
RadialBlur::RadialBlur()
{
    mShared.mNum = 0;
}

/**
 * Releases the contexts and the per-blur buffers.
 */
RadialBlur::~RadialBlur()
{
    mContext.freeBuffer();
    mShared.mParam.freeBuffer();
    mShared.mViewMask.freeBuffer();
}

/**
 * Allocates the contexts, the blur parameters and the circle mesh.
 * @param contextNum number of contexts
 * @param blurNum number of blurs per context
 * @param pHeap heap used for allocations
 */
void RadialBlur::initialize(s32 contextNum, s32 blurNum, sead::Heap* pHeap)
{
    mContext.tryAllocBuffer(contextNum, pHeap);
    for (s32 i = 0; i < contextNum; i++)
    {
        mContext[i].initialize(blurNum, pHeap);
    }

    initVertex_(32, 4, pHeap);
    initIndex_(32, 4, pHeap);

    mShared.mNum = 0;
    mShared.mParam.tryAllocBuffer(blurNum, pHeap);
    mShared.mViewMask.tryAllocBuffer(blurNum, pHeap);
    for (s32 i = 0; i < blurNum; i++)
    {
        mShared.mViewMask[i].setDirect(-1);
    }

    mShared.mDefaultViewMask.makeAllZero();

    mShared.mDebugTexturePage.setUp(contextNum, "RadialBlur", pHeap);
}

/**
 * Allocates the per-blur screen data and the work texture.
 * @param blurNum number of blurs
 * @param pHeap heap used for allocations
 */
void RadialBlur::Context::initialize(s32 blurNum, sead::Heap* pHeap)
{
    mScreen.tryAllocBuffer(blurNum, pHeap);
    mOrder.tryAllocBuffer(blurNum, pHeap);
    mReduceTarget.initialize_(TextureType(1), TextureFormat(0x1d), 1, 1, 1, 1,
                              TextureAttribute(0), MultiSampleType(0), true);
}

/**
 * Builds the vertices of the circle mesh.
 * @param divNum number of segments per ring
 * @param ringNum number of rings
 * @param pHeap heap used for allocations
 */
void RadialBlur::initVertex_(s32 divNum, s32 ringNum, sead::Heap* pHeap)
{
    mVertexBlock.allocBuffer((ringNum - 1) * divNum + 1, pHeap, 8, MemoryAttribute(0));
    GPUMemAddr<Vertex> addr(mVertexBlock, 0);

    getBlockPtr<Vertex>(mVertexBlock)[0].mPos = sead::Vector2f::zero;

    s32 vtx = 1;
    for (s32 ring = 1; ring < ringNum; ring++)
    {
        f32 radius = (f32(ring) / (f32(ringNum) - 1.0f)) * 0.5f;
        for (s32 div = 0; div < divNum; div++)
        {
            f32 angle = (f32(div) * sead::Mathf::pi2()) / f32(divNum);
            f32 c = sead::Mathf::cos(angle);
            f32 s = sead::Mathf::sin(angle);
            sead::Vector2f pos(radius * c, radius * s);
            getBlockPtr<Vertex>(mVertexBlock)[vtx].mPos = pos;
            vtx++;
        }
    }

    mVertexBuffer.setUpBuffer(ConstGPUMemVoidAddr(mVertexBlock, 0), sizeof(Vertex),
                              static_cast<u32>(mVertexBlock.getSize()));
    mVertexBuffer.setUpStream(0, VertexStreamFormat(0x16), 0, false);
    mShared.mVertexAttribute.create(1, pHeap);
    mShared.mVertexAttribute.setVertexStream(0, &mVertexBuffer, 0);
    mShared.mVertexAttribute.setUp();
}

void RadialBlur::initIndex_(s32 divNum, s32 ringNum, sead::Heap* pHeap)
{
    mIndexBlock.allocBuffer((ringNum * 6 - 9) * divNum, pHeap, 8, MemoryAttribute(0));
    GPUMemAddr<u16> addr(mIndexBlock, 0);

    s32 idx = 0;
    for (s32 div = 0; div < divNum; div++)
    {
        getBlockPtr<u16>(mIndexBlock)[idx] = 0;
        getBlockPtr<u16>(mIndexBlock)[idx + 1] = div + 1;
        getBlockPtr<u16>(mIndexBlock)[idx + 2] = (div + 1) % divNum + 1;
        idx += 3;
    }

    for (s32 ring = 1; ring < ringNum - 1; ring++)
    {
        s32 base0 = (ring - 1) * divNum + 1;
        s32 base1 = base0 + divNum;
        for (s32 div = 0; div < divNum; div++)
        {
            getBlockPtr<u16>(mIndexBlock)[idx] = base0 + div;
            getBlockPtr<u16>(mIndexBlock)[idx + 1] = base1 + (div + 1) % divNum;
            getBlockPtr<u16>(mIndexBlock)[idx + 2] = base0 + (div + 1) % divNum;
            getBlockPtr<u16>(mIndexBlock)[idx + 3] = base0 + div;
            getBlockPtr<u16>(mIndexBlock)[idx + 4] = base1 + div;
            getBlockPtr<u16>(mIndexBlock)[idx + 5] = base1 + (div + 1) % divNum;
            idx += 6;
        }
    }

    mShared.mIndexStream.setUpStream(GPUMemAddr<u16>(mIndexBlock, 0),
                                     mIndexBlock.getSize() / sizeof(u16));
    mShared.mIndexStream.setPrimitiveType(NVN_DRAW_PRIMITIVE_TRIANGLES);
}

/**
 * Does nothing.
 */
void RadialBlur::calc() {}

/**
 * Updates the screen data of one context.
 * @param index context index
 * @param rView view matrix
 * @param rProj projection matrix
 */
void RadialBlur::calcView(s32 index, const sead::Matrix34f& rView, const sead::Matrix44f& rProj)
{
    mContext[index].calc(mShared, rView, rProj);
}

void RadialBlur::Context::calc(const SharedData& rShared, const sead::Matrix34f& rView,
                               const sead::Matrix44f& rProj)
{
    sead::Matrix44f projView;
    projView.setMul(rProj, rView);
    sead::Vector3f dirZ(rView(2, 0), rView(2, 1), rView(2, 2));
    sead::Vector3f dirX(rView(0, 0), rView(0, 1), rView(0, 2));

    for (s32 i = 0; i < rShared.mNum; i++)
    {
        const BlurParameter& rParam = rShared.mParam[i];
        sead::Vector3f pos = rParam.mCenter + dirZ * rParam.mDepthOffset;
        mScreen[i].mPos = projectPos(projView, pos);

        sead::Vector3f edge = projectPos(projView, pos + dirX * rParam.mRadius);
        mScreen[i].mRadius = (mScreen[i].mPos - edge).length();
        mOrder[i] = i;

        if (!rParam.mDepthTestEnable)
        {
            mScreen[i].mPos.z = -1.0f;
        }
    }

    bool swapped;
    do
    {
        swapped = false;
        for (s32 i = 0; i < rShared.mNum - 1; i++)
        {
            s32 a = mOrder[i];
            s32 b = mOrder[i + 1];
            if (a < b && mScreen[a].mPos.z < mScreen[b].mPos.z)
            {
                mOrder[i] = b;
                mOrder[i + 1] = a;
                swapped = true;
            }
        }
    } while (swapped);

    if (!rShared.mDefaultViewMask.isZero())
    {
        const BlurParameter& rParam = rShared.mDefaultParam;
        mDefaultScreen.mPos = rParam.mCenter + dirZ * rParam.mDepthOffset;
        mDefaultScreen.mRadius = rParam.mRadius;
        if (!rParam.mDepthTestEnable)
        {
            mDefaultScreen.mPos.z = -1.0f;
        }
    }
}

/**
 * Does nothing.
 */
void RadialBlur::updateGPU() {}

/**
 * Does nothing.
 * @param index context index
 */
void RadialBlur::updateViewGPU(s32 index) {}

/**
 * Does nothing.
 */
void RadialBlur::Context::updateGPU() {}

/**
 * Draws every visible blur of one context.
 * @param pDrawContext draw context that receives the commands
 * @param index context index
 * @param rRenderBuffer render buffer to draw into
 * @param rTexture source texture
 */
void RadialBlur::draw(DrawContext* pDrawContext, s32 index, const RenderBuffer& rRenderBuffer,
                      const TextureData& rTexture) const
{
    mContext[index].draw(pDrawContext, index, mShared, rRenderBuffer, rTexture);
}

void RadialBlur::Context::draw(DrawContext* pDrawContext, s32 view, const SharedData& rShared,
                               const RenderBuffer& rRenderBuffer,
                               const TextureData& rTexture) const
{
    sead::Viewport viewport(rRenderBuffer);
    for (s32 i = 0; i < rShared.mNum; i++)
    {
        s32 idx = mOrder[i];
        const BlurParameter& rParam = rShared.mParam[idx];
        if (!rShared.mViewMask[idx].isOnBit(view))
        {
            continue;
        }

        f32 z = mScreen[idx].mPos.z;
        if (z < -1.0f || z > 1.0f)
        {
            continue;
        }

        {
            sead::GraphicsContext context;
            context.setDepthEnable(false, false);
            context.setBlendEnable(false);
            context.apply(pDrawContext);
        }

        drawToReduceTexture_(pDrawContext, view, rShared.mParam[idx], rShared, mScreen[idx],
                             rTexture);

        viewport.apply(pDrawContext, rRenderBuffer);
        rRenderBuffer.bind(pDrawContext);

        {
            sead::GraphicsContext context;
            context.setBlendEnable(rParam.mBlendEnable);
            context.setBlendFactor(0, rParam.mBlendFactorSrc, rParam.mBlendFactorDst);
            context.setBlendEquation(0, rParam.mBlendEquation);
            context.setDepthEnable(rParam.mDepthTestEnable, false);
            context.apply(pDrawContext);
        }

        drawToRenderBuffer_(pDrawContext, view, rShared.mParam[idx], rShared, mScreen[idx]);
    }
}

void RadialBlur::draw2D(DrawContext* pDrawContext, s32 index, const RenderBuffer& rRenderBuffer,
                        const TextureData& rTexture, const TextureData& rUnused) const
{
    mContext[index].allocHalfBufferTexture(pDrawContext, rTexture);
    mContext[index].updateHalfBuffer(pDrawContext, index, mShared, rRenderBuffer, rTexture);
    mContext[index].draw2D(pDrawContext, index, mShared, rRenderBuffer,
                           *mContext[index].mHalfBuffer);
    mContext[index].freeHalfBufferTexture();
}

/**
 * Allocates the half resolution copy of a texture.
 * @param pDrawContext draw context that receives the commands
 * @param rTexture source texture
 */
void RadialBlur::Context::allocHalfBufferTexture(DrawContext* pDrawContext,
                                                 const TextureData& rTexture) const
{
    freeHalfBufferTexture();
    utl::DynamicTextureAllocator* pAllocator = utl::DynamicTextureAllocator::instance();
    u32 width = rTexture.getWidth() / 2;
    u32 height = rTexture.getHeight(0) / 2;
    TextureFormat format = TextureFormat(rTexture.getTextureFormat());
    mHalfBuffer = pAllocator->alloc(pDrawContext, "radial_blur_HalfBuffer", format, width, height,
                                    1, nullptr,
        utl::DynamicTextureAllocator::AllocateType(0), true, false);
    mHalfBufferAddr = mHalfBuffer->getImagePtr();
}

void RadialBlur::Context::updateHalfBuffer(DrawContext* pDrawContext, s32 view,
                                           const SharedData& rShared,
                                           const RenderBuffer& rRenderBuffer,
                                           const TextureData& rTexture) const
{
    u32 width = rTexture.getWidth() / 2;
    u32 height = rTexture.getHeight(0) / 2;
    if (mReduceTarget.getWidth(0) != width || mReduceTarget.getHeight(0) != height ||
        mReduceTarget.getTextureFormat() != rTexture.getTextureFormat() ||
        mReduceTarget.getTextureAttribute() != mHalfBuffer->getTextureAttribute())
    {
        mReduceTarget.initialize_(TextureType(1), TextureFormat(rTexture.getTextureFormat()), width,
                                  height, 1, 1,
                                  TextureAttribute(mHalfBuffer->getTextureAttribute()),
                                  MultiSampleType(0), true);
    }

    {
        GPUMemVoidAddr addr = mHalfBufferAddr;
        mReduceTarget.setImagePtr(addr, 0);
    }

    mRenderTarget.applyTextureData(mReduceTarget);
    mRenderBuffer.setVirtualSize(sead::Vector2f(width, height));
    mRenderBuffer.setPhysicalArea(sead::BoundBox2f(0.0f, 0.0f, width, height));
    mRenderBuffer.setRenderTargetColor(&mRenderTarget);

    sead::Viewport viewport(mRenderBuffer);
    viewport.apply(pDrawContext, mRenderBuffer);
    mRenderBuffer.bind(pDrawContext);

    mSampler.applyTextureData(rTexture);
    mSampler.setLod(0.0f, 1.0f, 0.0f);
    mSampler.setFilter(1, 1, 0);

    {
        sead::GraphicsContext context;
        context.setDepthEnable(false, false);
        context.setBlendEnable(false);
        context.apply(pDrawContext);
    }

    utl::ImageFilter2D::drawReduce(pDrawContext, mSampler, viewport,
                                   utl::ImageFilter2D::cReduceScale_2, 1.0f,
                                   sead::Vector2f::zero);
    mRenderTarget.invalidateGPUCache(pDrawContext);
}

/**
 * Draws the default blur of this context.
 * @param pDrawContext draw context that receives the commands
 * @param view view index
 * @param rShared data shared by all contexts
 * @param rRenderBuffer render buffer to draw into
 * @param rTexture half resolution source texture
 */
void RadialBlur::Context::draw2D(DrawContext* pDrawContext, s32 view, const SharedData& rShared,
                                 const RenderBuffer& rRenderBuffer,
                                 const TextureData& rTexture) const
{
    sead::Viewport viewport(rRenderBuffer);
    if (!rShared.mDefaultViewMask.isOnBit(view))
    {
        return;
    }

    const BlurParameter& rParam = rShared.mDefaultParam;
    {
        sead::GraphicsContext context;
        context.setDepthEnable(false, false);
        context.setBlendEnable(false);
        context.apply(pDrawContext);
    }

    drawToReduceTexture_(pDrawContext, view, rParam, rShared, mDefaultScreen, rTexture);

    viewport.apply(pDrawContext, rRenderBuffer);
    rRenderBuffer.bind(pDrawContext);

    {
        sead::GraphicsContext context;
        context.setBlendEnable(rParam.mBlendEnable);
        context.setBlendFactor(0, rParam.mBlendFactorSrc, rParam.mBlendFactorDst);
        context.setBlendEquation(0, rParam.mBlendEquation);
        context.setDepthEnable(rParam.mDepthTestEnable, false);
        context.apply(pDrawContext);
    }

    drawToRenderBuffer_(pDrawContext, view, rParam, rShared, mDefaultScreen);
}

/**
 * Frees the half resolution texture.
 */
void RadialBlur::Context::freeHalfBufferTexture() const
{
    if (mHalfBuffer)
    {
        utl::DynamicTextureAllocator::instance()->free(mHalfBuffer);
        mHalfBuffer = nullptr;
    }
}

/**
 * Allocates the reduce texture of one context.
 * @param pDrawContext draw context that receives the commands
 * @param index context index
 * @param rTexture source texture
 */
void RadialBlur::allocReduceTexture(DrawContext* pDrawContext, s32 index,
                                    const TextureData& rTexture) const
{
    mContext[index].allocReduceTexture(pDrawContext, rTexture);
}

/**
 * Allocates the reduce texture at the size of a texture.
 * @param pDrawContext draw context that receives the commands
 * @param rTexture source texture
 */
void RadialBlur::Context::allocReduceTexture(DrawContext* pDrawContext,
                                             const TextureData& rTexture) const
{
    freeReduceTexture();
    utl::DynamicTextureAllocator* pAllocator = utl::DynamicTextureAllocator::instance();
    u32 width = rTexture.getWidth(0);
    u32 height = rTexture.getHeight(0);
    TextureFormat format = TextureFormat(rTexture.getTextureFormat());
    mReduceTexture = pAllocator->alloc(pDrawContext, "radial_blur", format, width, height, 1,
                                       nullptr,
        utl::DynamicTextureAllocator::AllocateType(0), true, false);
    mReduceAddr = mReduceTexture->getImagePtr();
}

/**
 * Frees the reduce texture of one context.
 * @param index context index
 */
void RadialBlur::freeReduceTexture(s32 index) const
{
    mContext[index].freeReduceTexture();
}

/**
 * Frees the reduce texture.
 */
void RadialBlur::Context::freeReduceTexture() const
{
    if (mReduceTexture)
    {
        utl::DynamicTextureAllocator::instance()->free(mReduceTexture);
        mReduceTexture = nullptr;
    }
}

/**
 * Generates the host IO messages.
 * @param pContext host IO context
 */
void RadialBlur::genMessage(sead::hostio::Context* pContext)
{
    mShared.mDebugTexturePage.genMessagePage(pContext, this);
    {
        sead::FormatFixedSafeString<32> meta("Min=0, Max=%d", mShared.mParam.size() - 1);
    }

    for (s32 i = 0; i < mShared.mParam.size(); i++)
    {
        {
            sead::FormatFixedSafeString<32> header("GroupHeader=RadialBlur[%d]", i);
            header.cstr();
        }

        for (s32 j = 0; j < 4; j++)
        {
            sead::FormatFixedSafeString<32> color("Color [%d]", j);
        }
    }
}

/**
 * Constructs an empty context.
 */
RadialBlur::Context::Context()
{
    mRenderBuffer.setRenderTargetColor(&mRenderTarget);
}

/**
 * Frees the buffers and the dynamic textures.
 */
RadialBlur::Context::~Context()
{
    mScreen.freeBuffer();
    mOrder.freeBuffer();
    freeReduceTexture();
    freeHalfBufferTexture();
}

void RadialBlur::Context::drawToReduceTexture_(DrawContext* pDrawContext, s32 view,
                                               const BlurParameter& rParam,
                                               const SharedData& rShared,
                                               const ScreenParameter& rScreen,
                                               const TextureData& rTexture) const
{
    s32 width = sead::Mathi::clamp(s32(rTexture.getWidth(0) * rParam.mReduceScale), 1,
                                   s32(mReduceTexture->getWidth(0)));
    s32 height = sead::Mathi::clamp(s32(rTexture.getHeight(0) * rParam.mReduceScale), 1,
                                    s32(mReduceTexture->getHeight(0)));
    if (mReduceTarget.getWidth(0) != u32(width) || mReduceTarget.getHeight(0) != u32(height) ||
        mReduceTarget.getTextureFormat() != rTexture.getTextureFormat() ||
        mReduceTarget.getTextureAttribute() != mReduceTexture->getTextureAttribute())
    {
        mReduceTarget.initialize_(TextureType(1), TextureFormat(rTexture.getTextureFormat()), width,
                                  height, 1, 1,
                                  TextureAttribute(mReduceTexture->getTextureAttribute()),
                                  MultiSampleType(0), true);
    }

    {
        GPUMemVoidAddr addr = mReduceAddr;
        mReduceTarget.setImagePtr(addr, 0);
    }

    s32 variation = rParam.mSampleNum / 2 - 1;
    const ShaderProgram* pProgram =
        detail::ShaderHolder::instance()->getShaderProgramUnsafe(detail::ShaderHolder::cRadialBlur);
    s32 num = pProgram->getVariationProgramNum();
    if (variation > num)
    {
        variation = num;
    }

    if (rParam.mSampleNum < 2)
    {
        variation = 0;
    }

    pProgram = pProgram->getVariation(variation);

    mRenderTarget.applyTextureData(mReduceTarget);
    mRenderBuffer.setVirtualSize(sead::Vector2f(width, height));
    mRenderBuffer.setPhysicalArea(sead::BoundBox2f(0.0f, 0.0f, width, height));
    mRenderBuffer.setRenderTargetColor(&mRenderTarget);

    sead::Viewport viewport(mRenderBuffer);
    viewport.apply(pDrawContext, mRenderBuffer);
    mRenderBuffer.bind(pDrawContext);

    mSampler.applyTextureData(rTexture);
    mSampler.setLod(0.0f, 1.0f, 0.0f);
    mSampler.setFilter(1, 1, 0);

    f32 radius = 2.83f / f32(height) + rScreen.mRadius;
    pProgram->activate(pDrawContext, true);
    {
        struct
        {
            sead::Vector3f mPos;
            f32 mPower;
        } data = {rScreen.mPos, rParam.mPower};
        pProgram->getUniformLocation(1).setUniform(pDrawContext, 4, &data);
    }

    {
        sead::Vector2f scale(radius, radius * f32(width) / f32(height));
        pProgram->getUniformLocation(0).setUniform(pDrawContext, 2, &scale);
    }

    utl::VertexAttributeHolder::instance()
        ->getVertexAttribute(utl::VertexAttributeHolder::cAttribute_CircleTexCoord)
        .activate(pDrawContext);
    mSampler.activate(pDrawContext, pProgram->getSamplerLocation(0), -1, false);
    pfx::detail::drawIndexStream(pDrawContext,
                                 utl::PrimitiveShape::instance()->getCircleIndexStream(0));
    mRenderTarget.invalidateGPUCache(pDrawContext);
}

void RadialBlur::Context::drawToRenderBuffer_(DrawContext* pDrawContext, s32 view,
                                              const BlurParameter& rParam,
                                              const SharedData& rShared,
                                              const ScreenParameter& rScreen) const
{
    const ShaderProgram* pProgram = detail::ShaderHolder::instance()
                                        ->getShaderProgramUnsafe(detail::ShaderHolder::cRadialBlurCompose)
                                        ->getVariation(0);
    mSampler.setFilter(1, 1, 0);
    mSampler.applyTextureData(mReduceTarget);
    f32 width = mReduceTarget.getWidth(0);
    f32 height = mReduceTarget.getHeight(0);
    pProgram->activate(pDrawContext, true);
    {
        struct
        {
            sead::Vector3f mPos;
            f32 mPower;
        } data = {rScreen.mPos, rParam.mPower};
        pProgram->getUniformLocation(1).setUniform(pDrawContext, 4, &data);
    }

    {
        sead::Vector2f scale(rScreen.mRadius, rScreen.mRadius * width / height);
        pProgram->getUniformLocation(0).setUniform(pDrawContext, 2, &scale);
    }

    pProgram->getUniformLocation(2).setUniform(pDrawContext, 16, &rParam.mColor0);
    rShared.mVertexAttribute.activate(pDrawContext);
    mSampler.activate(pDrawContext, pProgram->getSamplerLocation(0), -1, false);
    pfx::detail::drawIndexStream(pDrawContext, rShared.mIndexStream);
}

}  // namespace agl::fx
