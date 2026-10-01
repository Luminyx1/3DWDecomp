#include "environment/aglCubeMap.h"

#include <gfx/seadGraphicsContext.h>
#include <gfx/seadViewport.h>
#include <math/seadMathCalcCommon.h>

#include "common/aglDrawContext.h"
#include "common/aglGPUMemBlock.h"
#include "utility/aglDynamicTextureAllocator.h"
#include "utility/aglImageFilter2D.h"

namespace agl::utl::DevTools {
void drawTexture(DrawContext*, const TextureSampler&, const sead::Matrix34f&,
                 const sead::Matrix44f&, const sead::Color4f&);
}  // namespace agl::utl::DevTools

namespace agl::env {

namespace {

GPUMemVoidAddr nullAddr()
{
    GPUMemVoidAddr addr;
    return addr;
}

}  // namespace

const sead::Matrix34f CubeMap::scDrawCubeMapScale[2] = {
    {2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 2.0f, 0.0f},
    {2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 2.0f, 0.0f},
};

/**
 * Constructs an empty cube map.
 */
CubeMap::CubeMap()
{
    mTextureData.setDebugLabel("agl::env::CubeMap");
}

/**
 * Frees the owned buffers.
 */
CubeMap::~CubeMap()
{
    destroyBuffer();
}

/**
 * Frees the owned image, color and depth buffers.
 */
void CubeMap::destroyBuffer()
{
    if (!mFlag.isOn(1 << 0))
    {
        return;
    }

    if (mFlag.isOn(1 << 1))
    {
        if (mImageAddr.isValid())
        {
            mImageAddr.deleteGPUMemBlock();
        }

        if (mMipAddr.isValid())
        {
            mMipAddr.deleteGPUMemBlock();
        }
    }

    mImageAddr.invalidate();
    mMipAddr.invalidate();
    mFlag.reset(0x87);
}

/**
 * Allocates a cube map texture.
 * @param pHeap heap used for allocations
 * @param format texture format
 * @param width face size
 * @param mipLevelNum number of mip levels
 */
void CubeMap::initialize(sead::Heap* pHeap, TextureFormat format, u32 width, u32 mipLevelNum)
{
    destroyBuffer();
    mTextureData.initialize_(TextureType(8), format, width, width, 6, mipLevelNum,
                             TextureAttribute(0), MultiSampleType(0), true);

    u32 size = mTextureData.getSurface().mStorageSize;

    if (size != 0)
    {
        u32 alignment = mTextureData.getSurface().mAlignment;
        auto* pBlock = new (pHeap, 8) GPUMemBlock<u8>;
        pBlock->allocBuffer_(size, pHeap, alignment, MemoryAttribute(0));
        mImageAddr = GPUMemVoidAddr(*pBlock, 0);
    }

    size = mTextureData.getSurface()._14;

    if (size != 0)
    {
        u32 alignment = mTextureData.getSurface().mAlignment;
        auto* pBlock = new (pHeap, 8) GPUMemBlock<u8>;
        pBlock->allocBuffer_(size, pHeap, alignment, MemoryAttribute(0));
        mMipAddr = GPUMemVoidAddr(*pBlock, 0);
    }

    initialize_();
    mFlag.set(1 << 1);
}

void CubeMap::initialize_()
{
    mTextureData.setImagePtr(mImageAddr, 0);
    mImageAddr.flushCPUCache(mTextureData.getSurface().mStorageSize);

    if (mMipAddr.isValid())
    {
        mTextureData.setMipPtr(mMipAddr);
        mMipAddr.flushCPUCache(mTextureData.getSurface()._14);
    }

    applyTextureData(mTextureData);
    mFlag.set(1 << 0);
}

/**
 * Sets up a cube map texture on an existing buffer.
 * @param addr image buffer
 * @param size buffer size
 * @param format texture format
 * @param width face size
 * @param mipLevelNum number of mip levels
 */
void CubeMap::initialize(GPUMemVoidAddr addr, u32 size, TextureFormat format, u32 width,
                         u32 mipLevelNum)
{
    destroyBuffer();
    mTextureData.initialize_(TextureType(8), format, width, width, 6, mipLevelNum,
                             TextureAttribute(0), MultiSampleType(0), true);

    GPUMemVoidAddr cur = addr;

    if (mTextureData.getSurface().mStorageSize != 0)
    {
        cur.roundUp(mTextureData.getSurface().mAlignment);
        mImageAddr = cur;
        cur = GPUMemVoidAddr(cur, mTextureData.getSurface().mStorageSize);
    }

    if (mTextureData.getSurface()._14 != 0)
    {
        cur.roundUp(mTextureData.getSurface().mAlignment);
        mMipAddr = cur;
        cur = GPUMemVoidAddr(cur, mTextureData.getSurface()._14);
    }

    initialize_();
    mFlag.set(1 << 2);
}

/**
 * Allocates a cube map array texture.
 * @param pHeap heap used for allocations
 * @param format texture format
 * @param width face size
 * @param arrayNum number of cube maps
 * @param mipLevelNum number of mip levels
 */
void CubeMap::initialize(sead::Heap* pHeap, TextureFormat format, u32 width, u32 arrayNum,
                         u32 mipLevelNum)
{
    destroyBuffer();
    mTextureData.initializeCubeMapArray(format, width, width, arrayNum, mipLevelNum,
                                        TextureAttribute(0));

    u32 size = mTextureData.getSurface().mStorageSize;

    if (size != 0)
    {
        u32 alignment = mTextureData.getSurface().mAlignment;
        auto* pBlock = new (pHeap, 8) GPUMemBlock<u8>;
        pBlock->allocBuffer_(size, pHeap, alignment, MemoryAttribute(0));
        mImageAddr = GPUMemVoidAddr(*pBlock, 0);
    }

    size = mTextureData.getSurface()._14;

    if (size != 0)
    {
        u32 alignment = mTextureData.getSurface().mAlignment;
        auto* pBlock = new (pHeap, 8) GPUMemBlock<u8>;
        pBlock->allocBuffer_(size, pHeap, alignment, MemoryAttribute(0));
        mMipAddr = GPUMemVoidAddr(*pBlock, 0);
    }

    initialize_();
    mFlag.set(1 << 1 | 1 << 7);
}

/**
 * Sets up a cube map array texture on an existing buffer.
 * @param addr image buffer
 * @param size buffer size
 * @param format texture format
 * @param width face size
 * @param arrayNum number of cube maps
 * @param mipLevelNum number of mip levels
 */
void CubeMap::initialize(GPUMemVoidAddr addr, u32 size, TextureFormat format, u32 width,
                         u32 arrayNum, u32 mipLevelNum)
{
    destroyBuffer();
    mTextureData.initializeCubeMapArray(format, width, width, arrayNum, mipLevelNum,
                                        TextureAttribute(0));

    GPUMemVoidAddr cur = addr;

    if (mTextureData.getSurface().mStorageSize != 0)
    {
        cur.roundUp(mTextureData.getSurface().mAlignment);
        mImageAddr = cur;
        cur = GPUMemVoidAddr(cur, mTextureData.getSurface().mStorageSize);
    }

    if (mTextureData.getSurface()._14 != 0)
    {
        cur.roundUp(mTextureData.getSurface().mAlignment);
        mMipAddr = cur;
        cur = GPUMemVoidAddr(cur, mTextureData.getSurface()._14);
    }

    initialize_();
    mFlag.set(1 << 2 | 1 << 7);
}

void CubeMap::initialize(const TextureData& rTextureData)
{
    destroyBuffer();
    mTextureData = rTextureData;
    mImageAddr = mTextureData.getImagePtr();
    mMipAddr = mTextureData.getMipPtr();
    initialize_();

    if (mTextureData.getMipSlice(0) >= 7)
    {
        mFlag.reset(1 << 7);
    }
}

bool CubeMap::begin(DrawContext* pDrawContext, bool useColorBuffer, bool useDepthBuffer)
{
    u32 size = mTextureData.getWidth() > 1 ? mTextureData.getWidth() : 1;
    mZCullBuffer.invalidate();

    if (useColorBuffer)
    {
        mFlag.set(1 << 6);
        size *= 2;
        mColorBuffer = utl::DynamicTextureAllocator::instance()->alloc(
            pDrawContext, "render_cubemap_color", TextureFormat(mTextureData.getTextureFormat()),
            size, size, 1, nullptr, utl::DynamicTextureAllocator::cAllocateType_0, true, false);
    }
    else
    {
        mColorBuffer = nullptr;
    }

    mDepthBuffer = useDepthBuffer ?
                       utl::DynamicTextureAllocator::instance()->alloc(
                           pDrawContext, "render_cubemap_depth", TextureFormat(0x3c), size, size,
                           1, &mZCullBuffer, utl::DynamicTextureAllocator::cAllocateType_0, true,
                           false) :
                       nullptr;
    mFlag.set(1 << 3);

    if (!useDepthBuffer || mDepthBuffer != nullptr)
    {
        mFlag.set(1 << 5);
    }

    mCurrentSize = -1;
    return mFlag.isOn(1 << 5);
}

/**
 * Binds one face of the first cube map for drawing.
 * @param pDrawContext draw context that receives the commands
 * @param face face index
 * @param mipLevel mip level
 * @return true if the face was bound
 */
bool CubeMap::preDraw(DrawContext* pDrawContext, u32 face, u32 mipLevel)
{
    return preDraw(pDrawContext, 0, face, mipLevel);
}

bool CubeMap::preDraw(DrawContext* pDrawContext, u32 slice, u32 face, u32 mipLevel)
{
    if (!mFlag.isOn(1 << 5))
    {
        return false;
    }

    s32 width = mTextureData.getWidth() >> mipLevel;
    u32 size = (width > 1 ? width : 1) * (mFlag.isOn(1 << 6) ? 2 : 1);

    if (!mFlag.isOn(1 << 4) || mCurrentSize != s32(size))
    {
        if (mDepthBuffer != nullptr)
        {
            mDepthTexture.initialize_(TextureType(1), TextureFormat(0x3c), size, size, 1, 1,
                                      TextureAttribute(0), MultiSampleType(0), true);
            mDepthTexture.setImagePtr(mDepthBuffer->getImagePtr(), 0);
        }

        if (mColorBuffer != nullptr)
        {
            mColorTexture.initialize_(TextureType(1),
                                      TextureFormat(mColorBuffer->getTextureFormat()), size, size,
                                      1, 1, TextureAttribute(mColorBuffer->getTextureAttribute()),
                                      MultiSampleType(0), true);
            mColorTexture.setImagePtr(mColorBuffer->getImagePtr(), 0);
        }
    }

    mCurrentSlice = slice;
    mCurrentFace = face;
    mCurrentMipLevel = mipLevel;
    mCurrentSize = size;

    if (mColorBuffer != nullptr)
    {
        if (mDepthBuffer != nullptr)
        {
            bindRenderBuffer(pDrawContext, &mColorTexture, 0, 0, &mDepthTexture, mZCullBuffer,
                             true);
        }
        else
        {
            bindRenderBuffer(pDrawContext, &mColorTexture, 0, 0, nullptr, nullAddr(), true);
        }
    }
    else
    {
        if (mDepthBuffer != nullptr)
        {
            bindRenderBuffer(pDrawContext, &mTextureData, mipLevel, slice * 6 + face,
                             &mDepthTexture, mZCullBuffer, true);
        }
        else
        {
            bindRenderBuffer(pDrawContext, &mTextureData, mipLevel, slice * 6 + face, nullptr,
                             nullAddr(), true);
        }
    }

    mFlag.set(1 << 4);
    return true;
}

/**
 * Binds color and depth targets to the internal render buffer.
 * @param pDrawContext draw context that receives the commands
 * @param pColor color texture
 * @param mipLevel mip level
 * @param slice slice index
 * @param pDepth depth texture, or nullptr
 * @param zcullBuffer ZCull buffer
 * @param invalidate unused
 */
void CubeMap::bindRenderBuffer(DrawContext* pDrawContext, const TextureData* pColor,
                               u32 mipLevel, u32 slice, const TextureData* pDepth,
                               GPUMemVoidAddr zcullBuffer, bool invalidate)
{
    s32 width = pColor->getWidth() >> mipLevel;
    f32 size = width > 1 ? u32(width) : 1u;

    mRenderBuffer.setRenderTargetColorNullAll();
    mRenderBuffer.setRenderTargetDepth(nullptr);
    mRenderTargetColor.applyTextureData(*pColor, mipLevel, slice);
    mRenderBuffer.setRenderTargetColor(&mRenderTargetColor);

    if (invalidate)
    {
        mRenderTargetColor.invalidateGPUCache(pDrawContext);
    }

    if (pDepth != nullptr)
    {
        mRenderTargetDepth.applyTextureData(*pDepth, 0, 0);
        GPUMemVoidAddr zcull = zcullBuffer;
        mRenderTargetDepth.setZCullBuffer(zcull);
        mRenderBuffer.setRenderTargetDepth(&mRenderTargetDepth);

        if (invalidate)
        {
            mRenderTargetDepth.invalidateGPUCache(pDrawContext);
        }
    }

    mRenderBuffer.setPhysicalArea(0.0f, 0.0f, size, size);
    mRenderBuffer.setVirtualSize(sead::Vector2f(size, size));
    mRenderBuffer.bind(pDrawContext);
}

void CubeMap::postDraw(DrawContext* pDrawContext, bool flip)
{
    if (!mFlag.isOn(1 << 5))
    {
        return;
    }

    if (mColorBuffer != nullptr)
    {
        mRenderTargetColor.invalidateGPUCache(pDrawContext);
        sead::GraphicsContext context;
        TextureSampler sampler;
        sampler.applyTextureData(mColorTexture);
        bindRenderBuffer(pDrawContext, &mTextureData, mCurrentMipLevel,
                         mCurrentSlice * 6 + mCurrentFace, nullptr, nullAddr(), true);
        context.setDepthEnable(false, false);
        context.setCullingMode(0);
        context.setBlendEnable(false);
        context.apply(pDrawContext);
        sead::Viewport viewport(mRenderBuffer);
        viewport.apply(pDrawContext, mRenderBuffer);
        utl::DevTools::drawTexture(pDrawContext, sampler, scDrawCubeMapScale[flip],
                                   sead::Matrix44f::ident, sead::Color4f::cWhite);
        mRenderTargetColor.invalidateGPUCache(pDrawContext);
    }

    mCurrentSlice = -1;
    mCurrentFace = -1;
    mCurrentMipLevel = -1;
}

void CubeMap::clear(DrawContext* pDrawContext, u32 slice, const sead::Color4f& rColor)
{
    GPUMemVoidAddr zcull;

    for (u32 mipLevel = 0; mipLevel < mTextureData.getMipLevelNum(); mipLevel++)
    {
        for (u32 face = 0; face < 6; face++)
        {
            bindRenderBuffer(pDrawContext, &mTextureData, mipLevel, slice * 6 + face, nullptr,
                             zcull, false);
            if (face == 0)
            {
                sead::Viewport(mRenderBuffer).apply(pDrawContext, mRenderBuffer);
            }

            mRenderBuffer.fastClear(pDrawContext, 0, 1, rColor, 1.0f, 0,
                                    sead::Viewport(mRenderBuffer), true);
        }
    }

    mRenderTargetColor.invalidateGPUCache(pDrawContext);
}

/**
 * Finishes drawing and frees the temporary buffers.
 * @param pDrawContext draw context that receives the commands
 */
void CubeMap::end(DrawContext* pDrawContext)
{
    if (!mFlag.isOn(1 << 3))
    {
        return;
    }

    mRenderTargetColor.invalidateGPUCache(pDrawContext);

    if (mColorBuffer != nullptr)
    {
        utl::DynamicTextureAllocator::instance()->free(mColorBuffer);
    }

    if (mDepthBuffer != nullptr)
    {
        utl::DynamicTextureAllocator::instance()->free(mDepthBuffer);
    }

    mColorBuffer = nullptr;
    mDepthBuffer = nullptr;
    mZCullBuffer.invalidate();
    mFlag.reset(0x78);
}

/**
 * Filters one mip level through a work texture.
 * @param pDrawContext draw context that receives the commands
 * @param rSrc source sampler
 * @param rWork work sampler
 * @param srcSlice source slice
 * @param srcMipLevel source mip level
 * @param dstSlice destination slice
 * @param dstMipLevel destination mip level
 * @param sigma blur strength
 */
void CubeMap::renderToMipMapUnit(DrawContext* pDrawContext, const TextureSampler& rSrc,
                                 const TextureSampler& rWork, u32 srcSlice, u32 srcMipLevel,
                                 u32 dstSlice, u32 dstMipLevel, f32 sigma)
{
    for (u32 face = 0; face < 6; face++)
    {
        bindRenderBuffer(pDrawContext, &rWork.getTextureData(), 0, face, nullptr,
                         nullAddr(), false);
        if (face == 0)
        {
            sead::Viewport(mRenderBuffer).apply(pDrawContext, mRenderBuffer);
        }

        utl::ImageFilter2D::drawCubemapGaussian(pDrawContext, rSrc, srcSlice, srcMipLevel, face,
                                                utl::ImageFilter2D::BlurType(2), sigma);
    }

    mRenderTargetColor.invalidateGPUCache(pDrawContext);

    for (u32 face = 0; face < 6; face++)
    {
        bindRenderBuffer(pDrawContext, &mTextureData, dstMipLevel, dstSlice * 6 + face, nullptr,
                         nullAddr(), false);
        if (face == 0)
        {
            sead::Viewport(mRenderBuffer).apply(pDrawContext, mRenderBuffer);
        }

        utl::ImageFilter2D::drawCubemapGaussian(pDrawContext, rWork, 0, face,
                                                utl::ImageFilter2D::BlurType(3), sigma);
    }

    mRenderTargetColor.invalidateGPUCache(pDrawContext);
}

void CubeMap::renderToMipMapImpl(DrawContext* pDrawContext, const TextureSampler& rSrc,
                                 u32 srcSlice, u32 srcMipLevel, u32 dstSlice, u32 dstMipLevel,
                                 f32 sigma, u32 count)
{
    TextureSampler work;
    work.setSeamlessCubeMap(rSrc.isSeamlessCubeMap());
    work.setFilter(1, 1, 0);

    s32 width = rSrc.getTextureData().getWidth() >> srcMipLevel;
    TextureData* pTexture = utl::DynamicTextureAllocator::instance()->allocCube(
        pDrawContext, "cubemap_gaussian", TextureFormat(mTextureData.getTextureFormat()),
        width > 1 ? width : 1, 1, nullptr, utl::DynamicTextureAllocator::cAllocateType_0, true,
        false);
    work.applyTextureData(*pTexture);
    renderToMipMapUnit(pDrawContext, rSrc, work, srcSlice, srcMipLevel, dstSlice, dstMipLevel,
                       sigma);
    utl::DynamicTextureAllocator::instance()->free(pTexture);

    if (count >= 2)
    {
        width = getTextureData().getWidth() >> dstMipLevel;
        pTexture = utl::DynamicTextureAllocator::instance()->allocCube(
            pDrawContext, "cubemap_gaussian", TextureFormat(mTextureData.getTextureFormat()),
            width > 1 ? width : 1, 1, nullptr, utl::DynamicTextureAllocator::cAllocateType_0,
            true, false);
        work.applyTextureData(*pTexture);

        for (u32 i = 1; i < count; i++)
        {
            renderToMipMapUnit(pDrawContext, *this, work, dstSlice, dstMipLevel, dstSlice,
                               dstMipLevel, sigma);
        }

        utl::DynamicTextureAllocator::instance()->free(pTexture);
    }
}

/**
 * Generates a range of mip levels of one slice.
 * @param pDrawContext draw context that receives the commands
 * @param slice slice index
 * @param startMipLevel first mip level
 * @param endMipLevel last mip level
 * @param count filter pass count
 * @param sigma blur strength
 */
void CubeMap::generateMipMapImpl(DrawContext* pDrawContext, u32 slice, u32 startMipLevel,
                                 u32 endMipLevel, u32 count, f32 sigma)
{
    f32 s = count == 0 ? 0.0f : sigma;
    u32 n = count == 0 ? 1 : count;

    sead::GraphicsContext context;
    context.setDepthEnable(false, false);
    context.setCullingMode(0);
    context.setBlendEnable(false);
    context.apply(pDrawContext);

    for (u32 mipLevel = startMipLevel; mipLevel < endMipLevel; mipLevel++)
    {
        renderToMipMapImpl(pDrawContext, *this, slice, mipLevel - 1, slice, mipLevel, s, n);
    }
}

/**
 * Renders a filtered mip level of slice 0.
 * @param pDrawContext draw context that receives the commands
 * @param rSrc source sampler
 * @param srcMipLevel source mip level
 * @param dstMipLevel destination mip level
 * @param count filter pass count
 * @param sigma blur strength
 */
void CubeMap::renderToMipMap(DrawContext* pDrawContext, const TextureSampler& rSrc,
                             u32 srcMipLevel, u32 dstMipLevel, u32 count, f32 sigma)
{
    renderToMipMap(pDrawContext, rSrc, 0, srcMipLevel, 0, dstMipLevel, count, sigma);
}

void CubeMap::renderToMipMap(DrawContext* pDrawContext, const TextureSampler& rSrc,
                             u32 srcSlice, u32 srcMipLevel, u32 dstSlice, u32 dstMipLevel,
                             u32 count, f32 sigma)
{
    f32 minLod = getMinLod();
    f32 maxLod = getMaxLod();
    f32 lodBias = getLodBias();
    u8 minFilter = getMinFilter();
    u8 magFilter = getMagFilter();
    u8 mipFilter = getMipFilter();
    u8 mipLevelNum = mTextureData.getMipLevelNum();
    f32 s = count == 0 ? 0.0f : sigma;

    sead::GraphicsContext context;
    context.setDepthEnable(false, false);
    context.setCullingMode(0);
    context.setBlendEnable(false);
    context.apply(pDrawContext);

    setMinLod(0.0f);
    setMaxLod(mipLevelNum);
    setLodBias(0.0f);
    setFilter(1, 1, 2);
    renderToMipMapImpl(pDrawContext, rSrc, srcSlice, srcMipLevel, dstSlice, dstMipLevel, s,
                       count == 0 ? 1 : count);
    setMinLod(minLod);
    setMaxLod(maxLod);
    setLodBias(lodBias);
    setFilter(magFilter, minFilter, mipFilter);
}

/**
 * Generates a range of mip levels.
 * @param pDrawContext draw context that receives the commands
 * @param startMipLevel first mip level
 * @param endMipLevel last mip level
 * @param count filter pass count
 * @param sigma blur strength
 * @param seamless whether to sample seamlessly
 */
void CubeMap::generateMipMap(DrawContext* pDrawContext, u32 startMipLevel, u32 endMipLevel,
                             u32 count, f32 sigma, bool seamless)
{
    generateMipMapArray(pDrawContext, 0, startMipLevel, endMipLevel, count, sigma, seamless);
}

void CubeMap::generateMipMapArray(DrawContext* pDrawContext, u32 slice, u32 startMipLevel,
                                  u32 endMipLevel, u32 count, f32 sigma, bool seamless)
{
    u32 mipLevelNum = mTextureData.getMipLevelNum();

    if (startMipLevel >= mipLevelNum)
    {
        return;
    }

    f32 minLod = getMinLod();
    f32 maxLod = getMaxLod();
    f32 lodBias = getLodBias();
    u8 magFilter = getMagFilter();
    u8 minFilter = getMinFilter();
    u8 mipFilter = getMipFilter();
    bool isSeamless = isSeamlessCubeMap();
    u32 end = mipLevelNum < endMipLevel ? mipLevelNum : endMipLevel;

    setMinLod(0.0f);
    setMaxLod(mipLevelNum);
    setLodBias(0.0f);
    setFilter(1, 1, 1);
    setSeamlessCubeMap(seamless);

    generateMipMapImpl(pDrawContext, slice, startMipLevel, end, count, sigma);

    setMinLod(minLod);
    setMaxLod(maxLod);
    setLodBias(lodBias);
    setFilter(magFilter, minFilter, mipFilter);
    setSeamlessCubeMap(isSeamless);
}

void CubeMap::renderIrradiance(DrawContext* pDrawContext, u32 slice, u32 srcMipLevel,
                               u32 dstMipLevel)
{
    s32 width = mTextureData.getWidth() >> srcMipLevel;
    f32 angle = sead::Mathf::deg2rad(90.0f / (width > 1 ? width : 1));
    sead::Vector3f param(sead::Mathf::piHalf(), angle, angle);

    f32 minLod = getMinLod();
    setMinLod(0.0f);
    f32 maxLod = getMaxLod();
    f32 lodBias = getLodBias();
    u8 magFilter = getMagFilter();
    u8 minFilter = getMinFilter();
    u8 mipFilter = getMipFilter();

    setMaxLod(mTextureData.getMipLevelNum());
    setLodBias(0.0f);
    setFilter(1, 1, 2);
    setWrap(5, 5, 5);

    for (u32 face = 0; face < 6; face++)
    {
        bindRenderBuffer(pDrawContext, &mTextureData, dstMipLevel, slice * 6 + face, nullptr,
                         nullAddr(), false);
        sead::Viewport(mRenderBuffer).apply(pDrawContext, mRenderBuffer);

        if (mFlag.isOn(1 << 7))
        {
            utl::ImageFilter2D::drawCubemapIrradiance(pDrawContext, *this, srcMipLevel, slice,
                                                      face, param);
        }
        else
        {
            utl::ImageFilter2D::drawCubemapIrradiance(pDrawContext, *this, srcMipLevel, face,
                                                      param);
        }
    }

    setMinLod(minLod);
    setMaxLod(maxLod);
    setLodBias(lodBias);
    setFilter(magFilter, minFilter, mipFilter);
    mRenderTargetColor.invalidateGPUCache(pDrawContext);
}

}  // namespace agl::env
