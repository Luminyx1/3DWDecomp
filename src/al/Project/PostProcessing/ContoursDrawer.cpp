#include "Project/PostProcessing/ContoursDrawer.hpp"

#include <gfx/seadGraphicsContext.h>
#include <gfx/seadViewport.h>

#include "common/aglDrawContext.h"
#include "common/aglRenderBuffer.h"
#include "common/aglRenderTarget.h"
#include "common/aglShaderLocation.h"
#include "common/aglShaderProgram.h"
#include "common/aglTextureSampler.h"
#include "utility/aglDynamicTextureAllocator.h"
#include "utility/aglImageFilter2D.h"

#include "Library/Debug/Render/RenderBufferAttacher.hpp"
#include "Library/Shader/ForwardRendering/ShaderHolder.hpp"
#include "Project/PostProcessing/PostProcessingDrawerUtil.hpp"

namespace {

typedef agl::utl::ImageFilter2D::GaussianKernel GaussianKernel;

inline void drawGaussianBlur(agl::DrawContext* pContext,
                             agl::utl::DynamicTextureAllocator* pAllocator,
                             const agl::TextureData* pSrc, const agl::TextureData* pDst,
                             s32 kernel) {
    auto drawGaussian = [pContext](const agl::TextureData* pFrom, const agl::TextureData* pTo,
                                   s32 kernelType, bool isVertical) {
        agl::TextureSampler sampler;
        sampler.applyTextureData(*pFrom);
        sampler.setWrapX(7);
        sampler.setWrapY(7);

        agl::RenderTargetColor target;
        target.applyTextureData(*pTo);

        agl::RenderBuffer renderBuffer;
        renderBuffer.setVirtualSize(sead::Vector2f(pFrom->getWidth(0), pFrom->getHeight(0)));
        renderBuffer.setPhysicalArea(
            sead::BoundBox2f(0.0f, 0.0f, pFrom->getWidth(0), pFrom->getHeight(0)));
        renderBuffer.setRenderTargetColorNullAll();
        renderBuffer.setRenderTargetColor(&target);
        renderBuffer.bind(pContext);

        sead::Viewport viewport(renderBuffer);
        viewport.apply(pContext, renderBuffer);
        agl::utl::ImageFilter2D::drawGaussian(pContext, sampler, viewport,
                                              GaussianKernel(kernelType), isVertical, true,
                                              sead::Vector2f::zero);
        target.invalidateGPUCache(pContext);
    };

    agl::TextureData* work = pAllocator->alloc(
        pContext, "gaussian_x", agl::TextureFormat(pSrc->getTextureFormat()), pSrc->getWidth(0),
        pSrc->getHeight(0), 1, nullptr, agl::utl::DynamicTextureAllocator::AllocateType(0), true,
        false);
    drawGaussian(pSrc, work, kernel, true);
    drawGaussian(work, pDst, kernel, false);
    pAllocator->free(work);
}

}  // namespace

namespace al {

/**
 * Constructs the contours drawing parameters.
 */
ContoursDrawParam::ContoursDrawParam() {
    mParamObj = new ParameterObj();
    mIsEnable = new ParameterBool(false, mParamObj, "IsEnable", "有効", "", true);
    mIsNeon = new ParameterBool(false, mParamObj, "IsNeon", "ネオン", "", true);
    mIsUseDepth = new ParameterBool(false, mParamObj, "IsUseDepth", "デプスも使う", "", true);
    mIsUseLinearDepth =
        new ParameterBool(false, mParamObj, "IsUseLinearDepth", "リニアデプスで判定", "", true);
    mKernelSize =
        new ParameterS32(0, mParamObj, "KernelSize", "カーネルサイズ", "Min=0, Max=2", true);
    mGaussianType =
        new ParameterS32(1, mParamObj, "GaussianType", "ガウシアンタイプ", "Min=-1, Max=5", true);
    mGaussianTypeForEdge = new ParameterS32(1, mParamObj, "GaussianTypeForEdge",
                                            "ガウシアンタイプ(エッジ用)", "Min=-1, Max=5", true);
    mThreshold = new ParameterF32(0.3f, mParamObj, "Threshold", "閾値", "Min=0, Max=1", true);
    mThresholdDepth =
        new ParameterF32(5.0f, mParamObj, "ThresholdDepth", "デプス閾値", "Min=0, Max=1", true);
    mBrightnessOffset = new ParameterF32(5.0f, mParamObj, "BrightnessOffset", "明度オフセット",
                                         "Min=0, Max=1", true);
}

/**
 * Checks the Enable flag.
 * @return Whether Enable is set.
 */
bool ContoursDrawParam::isEnable() const {
    return mIsEnable->getValue();
}

/**
 * Checks the Neon flag.
 * @return Whether Neon is set.
 */
bool ContoursDrawParam::isNeon() const {
    return mIsNeon->getValue();
}

/**
 * Checks the UseDepth flag.
 * @return Whether UseDepth is set.
 */
bool ContoursDrawParam::isUseDepth() const {
    return mIsUseDepth->getValue();
}

/**
 * Checks the UseLinearDepth flag.
 * @return Whether UseLinearDepth is set.
 */
bool ContoursDrawParam::isUseLinearDepth() const {
    return mIsUseLinearDepth->getValue();
}

/**
 * Gets the KernelSize parameter.
 * @return KernelSize.
 */
s32 ContoursDrawParam::getKernelSize() const {
    return mKernelSize->getValue();
}

/**
 * Gets the GaussianType parameter.
 * @return GaussianType.
 */
s32 ContoursDrawParam::getGaussianType() const {
    return mGaussianType->getValue();
}

/**
 * Gets the GaussianTypeForEdge parameter.
 * @return GaussianTypeForEdge.
 */
s32 ContoursDrawParam::getGaussianTypeForEdge() const {
    return mGaussianTypeForEdge->getValue();
}

/**
 * Gets the Threshold parameter.
 * @return Threshold.
 */
f32 ContoursDrawParam::getThreshold() const {
    return mThreshold->getValue();
}

/**
 * Gets the ThresholdDepth parameter.
 * @return ThresholdDepth.
 */
f32 ContoursDrawParam::getThresholdDepth() const {
    return mThresholdDepth->getValue();
}

/**
 * Gets the BrightnessOffset parameter.
 * @return BrightnessOffset.
 */
f32 ContoursDrawParam::getBrightnessOffset() const {
    return mBrightnessOffset->getValue();
}

/**
 * Gets the contours shader and creates the parameter interpolation.
 * @param pShaderHolder Shader holder to get the shader from.
 */
ContoursDrawer::ContoursDrawer(ShaderHolder* pShaderHolder)
    : mShaderHolder(pShaderHolder), mRequestInterp(nullptr) {
    mShaderProgram = pShaderHolder->getShaderProgram("alRenderContours");
    ParamRequestInterp* interp = new ParamRequestInterp();
    mRequestInterp = interp;
    interp->mCurrentParam = new ContoursDrawParam();
    interp->mStartParam = new ContoursDrawParam();
    interp->mEndParam = new ContoursDrawParam();
    interp->mRequestParam = new ContoursDrawParam();
}

/**
 * Destroys the drawer.
 */
ContoursDrawer::~ContoursDrawer() {}

/**
 * Finishes initialization of the parameter interpolation.
 */
void ContoursDrawer::endInit() {
    mRequestInterp->endInit();
}

/**
 * Clears the parameter request.
 */
void ContoursDrawer::clearRequest() {
    mRequestInterp->clearRequest();
}

/**
 * Updates the parameter interpolation.
 */
void ContoursDrawer::update() {
    mRequestInterp->updateInterp();
}

/**
 * Draws contour lines onto the color target of a render buffer.
 * @param pContext Draw context.
 * @param rBuffer Render buffer to draw onto.
 * @param pLinearDepth Linear depth texture.
 * @param pDepth Depth texture.
 */
void ContoursDrawer::draw(agl::DrawContext* pContext, const agl::RenderBuffer& rBuffer,
                          const agl::TextureData* pLinearDepth,
                          const agl::TextureData* pDepth) const {
    const ContoursDrawParam* param = getCurrentParam();

    if (!param->isEnable()) {
        return;
    }

    const agl::TextureData* color = rBuffer.getRenderTargetColor();
    agl::utl::DynamicTextureAllocator* allocator = agl::utl::DynamicTextureAllocator::instance();
    agl::TextureData* blurred = allocator->alloc(
        pContext, "color_clamp_draw_texture", agl::TextureFormat(color->getTextureFormat()),
        color->getWidth(0), color->getHeight(0), 1, nullptr,
        agl::utl::DynamicTextureAllocator::AllocateType(0), true, false);
    s32 gaussianType = param->getGaussianType();

    if (gaussianType != -1) {
        drawGaussianBlur(pContext, allocator, color, blurred, gaussianType);
    } else {
        color->copyToAll(pContext, blurred);
    }

    const char* macros[] = {"KERNEL_SIZE", "IS_NEON", "IS_USE_DEPTH"};
    const char* values[] = {"0", "0", "0"};

    switch (param->getKernelSize()) {
    case 0:
        values[0] = "0";
        break;
    case 1:
        values[0] = "1";
        break;
    case 2:
        values[0] = "2";
        break;
    default:
        break;
    }

    if (param->isNeon()) {
        values[1] = "1";
    }

    if (param->isUseDepth()) {
        values[2] = "1";
    }

    const agl::ShaderProgram* program = mShaderHolder->getShaderProgram("alRenderContours");
    program = program->searchVariation(3, macros, values);
    program->activate(pContext, true);

    agl::SamplerLocation colorLocation("uOrgColor");
    colorLocation.search(*program);
    agl::TextureSampler colorSampler;
    colorSampler.applyTextureData(*blurred);
    colorSampler.activate(pContext, colorLocation, -1, false);

    agl::SamplerLocation depthLocation("uOrgDepth");
    depthLocation.search(*program);
    agl::TextureSampler depthSampler;

    if (param->isUseLinearDepth()) {
        depthSampler.applyTextureData(*pLinearDepth);
    } else {
        depthSampler.applyTextureData(*pDepth);
    }

    depthSampler.activate(pContext, depthLocation, -1, false);

    sead::Vector2f texcel(1.0f / color->getWidth(0), 1.0f / color->getHeight(0));

    {
        f32 threshold = param->getThreshold();
        agl::UniformLocation location("uThreshold");
        location.search(*program);
        location.setUniform(pContext, threshold);
    }

    {
        f32 thresholdDepth = param->getThresholdDepth();
        agl::UniformLocation location("uThresholdDepth");
        location.search(*program);
        location.setUniform(pContext, thresholdDepth);
    }

    {
        f32 brightnessOffset = param->getBrightnessOffset();
        agl::UniformLocation location("uBrightnessOffset");
        location.search(*program);
        location.setUniform(pContext, brightnessOffset);
    }

    {
        agl::UniformLocation location("uTexcel");
        location.search(*program);
        location.setUniform(pContext, 2, &texcel);
    }

    sead::GraphicsContext graphicsContext;
    graphicsContext.setDepthEnable(false, false);
    graphicsContext.setBlendEnable(false);
    graphicsContext.apply(pContext);

    {
        agl::RenderBuffer renderBuffer;
        RenderBufferAttacher attacher(&renderBuffer, color, nullptr, nullptr, nullptr, nullptr);
        drawPostProcessingQuad(pContext);
    }

    allocator->free(blurred);
    s32 gaussianTypeForEdge = param->getGaussianTypeForEdge();

    if (gaussianTypeForEdge != -1) {
        drawGaussianBlur(pContext, allocator, color, color, gaussianTypeForEdge);
    }
}

/**
 * Gets the current interpolated parameter.
 * @return Current parameter.
 */
const ContoursDrawParam* ContoursDrawer::getCurrentParam() const {
    return static_cast<const ContoursDrawParam*>(mRequestInterp->getCurrentParam());
}

/**
 * Requests a parameter.
 * @param priority Request priority.
 * @param step Interpolation steps.
 * @param rParam Requested parameter.
 */
void ContoursDrawer::requestParam(s32 priority, s32 step, const ContoursDrawParam& rParam) {
    mRequestInterp->requestParam(priority, step, rParam);
}

/**
 * Checks whether the current parameter is enabled.
 * @return Whether drawing is enabled.
 */
bool ContoursDrawer::isEnable() const {
    return getCurrentParam()->isEnable();
}

}  // namespace al
