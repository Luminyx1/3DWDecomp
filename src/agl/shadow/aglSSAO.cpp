#include "shadow/aglSSAO.h"

#include <hostio/seadHostIOPropertyEvent.h>
#include <math/seadMathCalcCommon.h>

#include "common/aglDrawContext.h"
#include "common/aglGPUMemBlock.h"
#include "common/aglTextureDataInitializer.h"
#include "shadow/aglShadowUtil.h"
#include "utility/aglDynamicTextureAllocator.h"

namespace agl::sdw
{

namespace
{

sead::Matrix44f sTexToNdcMtx(2.0f, 0.0f, 0.0f, -1.0f, 0.0f, -2.0f, 0.0f, 1.0f, 0.0f, 0.0f, 2.0f,
                             -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
sead::Matrix44f sNdcToTexMtx(0.5f, 0.0f, 0.0f, 0.5f, 0.0f, -0.5f, 0.0f, 0.5f, 0.0f, 0.0f, 0.5f,
                             0.5f, 0.0f, 0.0f, 0.0f, 1.0f);

}  // namespace

const f32 SSAO::cSSAODefaultBaseRadius = utl::DevTools::calcScale(0.05f);
const f32 SSAO::cSSAODefaultDistanceIntensityZero = utl::DevTools::calcScale(1.0f);
const f32 SSAO::cSSAODefaultDensity = 1.0f;
const f32 SSAO::cSSAODefaultVariableDistMin = 8.0f;
const s32 SSAO::cSSAODefaultSamplePairNum = 3;
const f32 SSAO::cSSAODefaultDepthOffset = 0.0001f;
const f32 SSAO::cAlchemyAODefaultRadius = utl::DevTools::calcScale(3.0f);
const f32 SSAO::cAlchemyAODefaultMaxRadius = utl::DevTools::calcScale(0.05f);
const f32 SSAO::cAlchemyAODefaultBias = 0.12f;
const f32 SSAO::cAlchemyAODefaultDetectionIntensity = 1.0f;
const f32 SSAO::cAlchemyAODefaultDensity = 0.6f;
const s32 SSAO::cAlchemyAODefaultSamplePairNum = 5;

namespace
{

const u32 cRotateTable[16] = {11, 2, 13, 9, 4, 7, 5, 0, 10, 12, 6, 1, 15, 14, 8, 3};

}  // namespace

/**
 * Constructs a context and binds its render target.
 */
SSAO::Context::Context()
{
    mRenderBuffer.setRenderTargetColor(&mRenderTarget);
    mRenderBuffer.setRenderTargetDepth(nullptr);
    mDepthSampler.setFilterDirect(0, 0, 0);
}

/**
 * Releases the AO buffer.
 */
SSAO::Context::~Context()
{
    if (mAOBuffer)
    {
        utl::DynamicTextureAllocator::instance()->free(mAOBuffer);
        mAOBuffer = nullptr;
    }
}

/**
 * (Re)allocates the AO buffer and sizes the render buffer.
 * @param pDrawContext draw context
 * @param format texture format of the AO buffer
 * @param width buffer width
 * @param height buffer height
 * @param mipLevelNum number of mip levels
 */
void SSAO::Context::allocTexture(DrawContext* pDrawContext, TextureFormat format, s32 width,
                                 s32 height, s32 mipLevelNum)
{
    utl::DynamicTextureAllocator* allocator = utl::DynamicTextureAllocator::instance();
    mRenderBuffer.setVirtualSize(sead::Vector2f(width, height));
    mRenderBuffer.setPhysicalArea(0.0f, 0.0f, static_cast<f32>(width), static_cast<f32>(height));
    mAOSampler.setFilterDirect(1, 1, 0);

    if (mAOBuffer)
    {
        allocator->free(mAOBuffer);
    }

    mAOBuffer =
        allocator->alloc(pDrawContext, "ao_buffer", format, width, height, mipLevelNum, nullptr,
                         utl::DynamicTextureAllocator::cAllocateType_0, true, false);

    if (!mSSAOTexture)
    {
        mSSAOTexture = allocator->alloc(pDrawContext, "ssao_ao", format, width, height, 1, nullptr,
                                        utl::DynamicTextureAllocator::cAllocateType_1, true, false);
    }
}

/**
 * Constructs the SSAO with its default parameters.
 */
SSAO::SSAO()
    : utl::IParameterIO("aglssao", 0), mParameter(this),
      mIsEnable(false, "is_enable", "有効", &mParameter),
      mSSAOType(0, "ssao_type", "SSAOの種類", &mParameter),
      mAOFar(utl::DevTools::calcScale(200.0f), "ao_far", "AOの半径が0になる距離", &mParameter),
      mRadius(cSSAODefaultBaseRadius, "radius", "radius", &mParameter),
      mDistAttn(cSSAODefaultDistanceIntensityZero, "dist_attn", "Default Distance Intensity",
                &mParameter),
      mDensity(cSSAODefaultDensity, "density", "Default Density", &mParameter),
      mVariableDistMin(cSSAODefaultVariableDistMin, "variable_dist_min", "Variable Distance Min",
                       &mParameter),
      mDepthOffset(cSSAODefaultDepthOffset, "depth_offset", "Depth Offset", "Min=0", &mParameter),
      mSamplePairNum(cSSAODefaultSamplePairNum, "sample_pair_num", "Sample Pair Num", "Min=1",
                     &mParameter),
      mAlchemyRadius(cAlchemyAODefaultRadius, "alchemy_radius", "Radius", &mParameter),
      mAlchemyMaxRadius(cAlchemyAODefaultMaxRadius, "alchemy_max_radius", "MaxRadius", &mParameter),
      mAlchemyBias(cAlchemyAODefaultBias, "alchemy_bias", "Bias", &mParameter),
      mAlchemyDetectionIntensity(cAlchemyAODefaultDetectionIntensity, "alchemy_detection_intensity",
                                 "Detection Intensity", &mParameter),
      mAlchemyDensity(cAlchemyAODefaultDensity, "alchemy_density", "Density", &mParameter),
      mAlchemySamplePairNum(cAlchemyAODefaultSamplePairNum, "alchemy_sample_pair_num",
                            "Sample Pair Num", "Min=1", &mParameter),
      mBlurNum(1, "blur_num", "Blur Num", "Min=0", &mParameter),
      mMipBlurNum(0, "mip_blur_num", "Mip Blur Num", "Min=0", &mParameter)
{
    addObj(&mParameter, "ssao");
    const sead::Vector4f zero = sead::Vector4f::zero;
    for (s32 i = 0; i < 9; i++)
    {
        mSphereVolume[i] = zero;
    }
}

/**
 * Destroys the contexts, the rotation texture buffer and the debug page.
 */
SSAO::~SSAO()
{
    mContexts.freeBuffer();
    if (mRotateTextureBuffer.isValid())
    {
        mRotateTextureBuffer.deleteGPUMemBlock();
    }

    mDebugTexturePage.cleanUp();
}

/**
 * Allocates the contexts and the rotation texture.
 * @param contextNum number of contexts
 * @param pHeap heap to allocate from
 */
void SSAO::initialize(s32 contextNum, sead::Heap* pHeap)
{
    mContexts.tryAllocBuffer(contextNum, pHeap);

    mRotateTexture.initialize_(TextureType::cTextureType_2D,
                               TextureFormat::cTextureFormat_R8_G8_uNorm, 4, 4, 1, 1,
                               TextureAttribute(0), MultiSampleType(0), true);
    const detail::Surface& surface = mRotateTexture.getSurface();
    const u32 alignment = surface.mAlignment;
    const u32 size = surface.mStorageSize;
    auto* pBlock = new (pHeap, 8) GPUMemBlock<u8>;
    pBlock->allocBuffer_(size, pHeap, alignment, MemoryAttribute::CpuCached);
    mRotateTextureBuffer = GPUMemVoidAddr(*pBlock, 0);

    initRotateTexture_(false);
    initSphereVolume_(0x80, false);

    mDebugTexturePage.setUp(contextNum, "SSAO", pHeap);
}

/**
 * Fills the 4x4 rotation texture with directions from the rotation table.
 * @param force whether the rebuild was requested explicitly (unused)
 */
void SSAO::initRotateTexture_(bool force)
{
    u16* pImage = static_cast<u16*>(mRotateTextureBuffer.getPtr());
    for (s32 y = 0; y < 4; y++)
    {
        for (s32 x = 0; x < 4; x++)
        {
            s32 index = y * 4 + x + 1;
            if (index >= 16)
            {
                index = 0;
            }

            f32 sin;
            f32 cos;
            sead::Mathf::sinCosIdx(&sin, &cos, cRotateTable[index] << 27);
            const u8 r = static_cast<s32>((cos + 1.0f) * 0.5f * 255.0f);
            const s32 g = (sin + 1.0f) * 0.5f * 255.0f;
            pImage[y * 4 + x] = r | g << 8;
        }
    }

    mRotateTexture.initialize_(TextureType::cTextureType_2D,
                               TextureFormat::cTextureFormat_R8_G8_uNorm, 4, 4, 1, 1,
                               TextureAttribute(0), MultiSampleType(0), true);
    mRotateTexture.setDebugLabel("agl::sdw::SSAO");
    mRotateTexture.setImagePtr(mRotateTextureBuffer);
    TextureDataInitializerRAW::copyTileImage(&mRotateTexture,
                                             ConstGPUMemVoidAddr(mRotateTextureBuffer), 0);
    mRotateSampler.applyTextureData(mRotateTexture);
    mRotateSampler.setFilter(0, 0, 0);
    mRotateSampler.setWrap(1, 1, 1);
}

/**
 * Computes the sample pairs and their volume weights inside the unit sphere.
 * @param resolution grid resolution used to integrate the sphere volume
 * @param force whether the rebuild was requested explicitly (unused)
 */
void SSAO::initSphereVolume_(s32 resolution, bool force)
{
    constexpr s32 cSampleMax = 9 * 2 + 1;

    const s32 pairNum = sead::Mathi::min(*mSamplePairNum, 9);
    *mSamplePairNum = pairNum;

    sead::Vector2f samples[cSampleMax];
    f32 weights[cSampleMax];
    samples[0].set(0.0f, 0.0f);
    weights[0] = 0.0f;
    for (s32 i = 0; i < pairNum; i++)
    {
        const f32 radius = (1.0f / (pairNum + 1.0f)) * (i + 1.2f);
        const f32 angle = i * sead::Mathf::pi() / pairNum + 0.1f;
        weights[i * 2 + 1] = 0.0f;
        const f32 cos = std::cos(angle);
        const f32 x = radius * cos;
        const f32 sin = std::sin(angle);
        const f32 y = radius * sin;
        samples[i * 2 + 1] = sead::Vector2f(x, y);
        samples[i * 2 + 2] = sead::Vector2f(-(radius * cos), -(radius * sin));
        weights[i * 2 + 2] = 0.0f;
    }

    const s32 sampleNum = pairNum * 2 + 1;
    f32 total = 0.0f;
    for (s32 y = 0; y < resolution; y++)
    {
        const s32 half = resolution / 2;
        const f32 fy = static_cast<f32>(y - half) / half;
        for (s32 x = 0; x < resolution; x++)
        {
            const f32 fx = static_cast<f32>(x - half) / half;
            const f32 dist = fy * fy + fx * fx;
            if (dist > 1.0f)
            {
                continue;
            }

            f32 minDist = dist;
            s32 nearest = 0;
            for (s32 k = 1; k < sampleNum; k++)
            {
                const f32 dx = fx - samples[k].x;
                const f32 dy = fy - samples[k].y;
                const f32 d = dx * dx + dy * dy;
                if (minDist > d)
                {
                    minDist = d;
                    nearest = k;
                }
            }

            const f32 volume =
                2.0f / resolution * 2.0f / resolution * (sead::Mathf::sqrt(1.0f - dist) * 2.0f);
            weights[nearest] += volume;
            total += volume;
        }
    }

    for (s32 k = 0; k < sampleNum; k++)
    {
        weights[k] /= total;
    }

    mCenterWeight = weights[0];

    for (s32 i = 0; i < pairNum; i++)
    {
        const sead::Vector2f& sample = samples[i * 2 + 1];
        const f32 height =
            sead::Mathf::sqrt(1.0f - (sample.x * sample.x + sample.y * sample.y)) * 2.0f;
        mSphereVolume[i] = sead::Vector4f(sample.x, sample.y, height, weights[i * 2 + 1]);
        mSphereHeight[i] = height;
    }
}

/**
 * Draws the ambient occlusion of a context using a camera projection.
 * @param pDrawContext draw context
 * @param index context index
 * @param width buffer width
 * @param height buffer height
 * @param rDepth scene depth texture
 * @param rViewMtx camera view matrix (unused)
 * @param rProjMtx camera projection matrix
 */
void SSAO::drawToAOBuffer(DrawContext* pDrawContext, s32 index, s32 width, s32 height,
                          const TextureData& rDepth, const sead::Matrix34f& rViewMtx,
                          const sead::Matrix44f& rProjMtx) const
{
    if (!*mIsEnable)
    {
        return;
    }

    f32 near;
    f32 far;
    ShadowUtil::calcNearFar(&near, &far, rProjMtx);
    const f32 aspect = rProjMtx(1, 1) / rProjMtx(0, 0);
    const f32 fovy = std::atan(1.0f / rProjMtx(1, 1));
    drawToAOBuffer_(pDrawContext, index, width, height, rDepth, near, far, fovy, aspect);
}

/**
 * Draws the ambient occlusion of a context.
 * @param pDrawContext draw context
 * @param index context index
 * @param width buffer width
 * @param height buffer height
 * @param rDepth scene depth texture
 * @param near near clip distance
 * @param far far clip distance
 * @param fovy vertical field of view
 * @param aspect aspect ratio
 */
void SSAO::drawToAOBuffer(DrawContext* pDrawContext, s32 index, s32 width, s32 height,
                          const TextureData& rDepth, f32 near, f32 far, f32 fovy, f32 aspect) const
{
    if (!*mIsEnable)
    {
        return;
    }

    drawToAOBuffer_(pDrawContext, index, width, height, rDepth, near, far, fovy * 0.5f, aspect);
}

/**
 * Releases the AO buffer of a context.
 * @param index context index
 */
void SSAO::release(s32 index)
{
    Context& context = mContexts[index];
    if (context.mAOBuffer)
    {
        utl::DynamicTextureAllocator::instance()->free(context.mAOBuffer);
        context.mAOBuffer = nullptr;
    }
}

/**
 * Generates the host IO message.
 * @param pContext host IO context
 */
void SSAO::genMessage(sead::hostio::Context* pContext)
{
    genMessageIO(pContext, 0xf);
    genMessageParameter(pContext, this);
}

/**
 * Generates the parameter host IO messages.
 * @param pContext host IO context
 * @param pNode parent node
 */
void SSAO::genMessageParameter(sead::hostio::Context* pContext, sead::hostio::Node* pNode)
{
    mDebugTexturePage.genMessagePage(pContext, pNode);
    mIsEnable.genMessageParameter(pContext, mIsEnable.getMeta());
    mRadius.genMessageParameter(pContext, "Min=0, Max=1");
    mVariableDistMin.genMessageParameter(pContext, "Min=1, Max=100");
    mDistAttn.genMessageParameter(pContext, "Min=0");
    mDensity.genMessageParameter(pContext, "Min=0, Max=4");
    mSamplePairNum.genMessageParameter(pContext, mSamplePairNum.getMeta());
    mDepthOffset.genMessageParameter(pContext, mDepthOffset.getMeta());
    mAlchemyRadius.genMessageParameter(pContext, "Min=0, Max=10");
    mAlchemyMaxRadius.genMessageParameter(pContext, "Min=0, Max=1");
    mAlchemyDetectionIntensity.genMessageParameter(pContext, "Min=0, Max=10");
    mAlchemyDensity.genMessageParameter(pContext, "Min=0, Max=10");
    mAlchemyBias.genMessageParameter(pContext, "Min=0, Max=1");
    mAlchemySamplePairNum.genMessageParameter(pContext, "Min=1, Max=16");
    mAOFar.genMessageParameter(pContext, "Min=0");
    mBlurNum.genMessageParameter(pContext, mBlurNum.getMeta());
    mMipBlurNum.genMessageParameter(pContext, mMipBlurNum.getMeta());
}

/**
 * Generates the debug parameter host IO messages (none).
 * @param pContext host IO context (unused)
 * @param pNode parent node (unused)
 */
void SSAO::genMessageDebugParameter(sead::hostio::Context* pContext, sead::hostio::Node* pNode) {}

/**
 * Handles a host IO property event.
 * @param pEvent property event
 */
void SSAO::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent)
{
    listenPropertyEventIO(this, pEvent);
    listenPropertyEventDebugParameter(pEvent);
}

/**
 * Handles a parameter host IO property event (nothing to do).
 * @param pEvent property event (unused)
 */
void SSAO::listenPropertyEventParameter(const sead::hostio::PropertyEvent* pEvent) {}

/**
 * Rebuilds the sampling tables, forcing it when the rebuild button was pressed.
 * @param pEvent property event
 */
void SSAO::listenPropertyEventDebugParameter(const sead::hostio::PropertyEvent* pEvent)
{
    const bool force = reinterpret_cast<uintptr_t>(pEvent->getId()) == 1001;
    initRotateTexture_(force);
    initSphereVolume_(0x80, force);
}

/**
 * Rebuilds the sampling tables after the parameters were read.
 */
void SSAO::postRead_()
{
    initRotateTexture_(false);
    initSphereVolume_(0x80, false);
}

}  // namespace agl::sdw
