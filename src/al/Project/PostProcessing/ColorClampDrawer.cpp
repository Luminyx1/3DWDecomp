#include "Project/PostProcessing/ColorClampDrawer.hpp"

#include <gfx/seadGraphicsContext.h>
#include <string>

#include "common/aglRenderBuffer.h"
#include "common/aglRenderTarget.h"
#include "common/aglTextureSampler.h"
#include "utility/aglDynamicTextureAllocator.h"

#include "Library/Debug/Render/RenderBufferAttacher.hpp"
#include "Library/Shader/DeferredRendering/FullScreenTriangle.hpp"
#include "Library/Shader/ForwardRendering/ShaderHolder.hpp"
#include "Project/PostProcessing/PostProcessingDrawerUtil.hpp"

namespace al {

/**
 * Constructs the color clamp drawing parameters.
 */
ColorClampDrawParam::ColorClampDrawParam() {
    mParamObj = new ParameterObj();
    mIsEnable = new ParameterBool(false, mParamObj, "IsEnable", "IsEnable", "", true);
    mClampColor = new ParameterC4f(sead::Color4f::cWhite, mParamObj, "ClampColor", "ClampColor", "Min=0, Max=1", true);
    mModifyStyle = new ParameterS32(0, mParamObj, "ModifyStyle", "Style", "Min=0, Max=5", true);
}

/**
 * Checks the Enable flag.
 * @return Whether Enable is set.
 */
bool ColorClampDrawParam::isEnable() const {
    return mIsEnable->getValue();
}

/**
 * Gets the ClampColor parameter.
 * @return ClampColor.
 */
const sead::Color4f& ColorClampDrawParam::getClampColor() const {
    return mClampColor->getValue();
}

/**
 * Gets the ModifyStyle parameter.
 * @return ModifyStyle.
 */
const s32& ColorClampDrawParam::getModifyStyle() const {
    return mModifyStyle->getValue();
}

/**
 * Gets the color modify shader and creates the parameter interpolation.
 * @param pShaderHolder Shader holder to get the shader from.
 */
ColorClampDrawer::ColorClampDrawer(ShaderHolder* pShaderHolder)
    : mShaderProgram(nullptr), mRequestInterp(nullptr) {
    mShaderProgram = pShaderHolder->getShaderProgram("alRenderColorModify");
    ParamRequestInterp* interp = new ParamRequestInterp();
    mRequestInterp = interp;
    interp->mCurrentParam = new ColorClampDrawParam();
    interp->mStartParam = new ColorClampDrawParam();
    interp->mEndParam = new ColorClampDrawParam();
    interp->mRequestParam = new ColorClampDrawParam();
}

/**
 * Finishes initialization of the parameter interpolation.
 */
void ColorClampDrawer::endInit() {
    mRequestInterp->endInit();
}

/**
 * Clears the parameter request.
 */
void ColorClampDrawer::clearRequest() {
    mRequestInterp->clearRequest();
}

/**
 * Updates the parameter interpolation.
 */
void ColorClampDrawer::update() {
    mRequestInterp->updateInterp();
}

void ColorClampDrawer::draw(agl::DrawContext* pContext, const agl::RenderBuffer& rBuffer) const {
    const ColorClampDrawParam* param = getCurrentParam();

    if (!param->isEnable()) {
        return;
    }

    const agl::TextureData* color = rBuffer.getRenderTargetColor();
    agl::utl::DynamicTextureAllocator* allocator = agl::utl::DynamicTextureAllocator::instance();
    agl::TextureData* copy = allocator->alloc(
        pContext, "color_clamp_draw_texture", agl::TextureFormat(color->getTextureFormat()),
        color->getWidth(0), color->getHeight(0), 1, nullptr,
        agl::utl::DynamicTextureAllocator::AllocateType(0), true, false);
    color->copyToAll(pContext, copy);

    std::string modifyStyle(1, '0' + param->getModifyStyle());
    const char* macros[] = {"COLOR_MODIFY_STYLE"};
    const char* values[] = {"COLOR_MODIFY_STYLE"};
    s32 index = ShaderSearchImpl::searchMacroIndex(macros, "COLOR_MODIFY_STYLE");

    if (index != -1) {
        values[index] = modifyStyle.c_str();
    }

    const agl::ShaderProgram* program = mShaderProgram->searchVariation(1, macros, values);
    program->activate(pContext, true);

    agl::SamplerLocation colorLocation("uOrgColor");
    colorLocation.search(*program);
    agl::TextureSampler colorSampler;
    colorSampler.applyTextureData(*copy);
    colorSampler.activate(pContext, colorLocation, -1, false);

    const sead::Color4f& clampColor = param->getClampColor();
    sead::Vector4f params(clampColor.r, clampColor.g, clampColor.b, 1.0f);
    setPostProcessingUniform(pContext, program, "uParams", params);

    sead::GraphicsContext graphicsContext;
    graphicsContext.setDepthEnable(false, false);
    graphicsContext.setBlendEnable(false);
    graphicsContext.apply(pContext);

    {
        agl::RenderBuffer renderBuffer;
        RenderBufferAttacher attacher(&renderBuffer, color, nullptr, nullptr, nullptr, nullptr);
        drawPostProcessingQuad(pContext);
    }

    allocator->free(copy);
}

/**
 * Gets the current interpolated parameter.
 * @return Current parameter.
 */
const ColorClampDrawParam* ColorClampDrawer::getCurrentParam() const {
    return static_cast<const ColorClampDrawParam*>(mRequestInterp->getCurrentParam());
}

/**
 * Requests a parameter.
 * @param priority Request priority.
 * @param step Interpolation steps.
 * @param rParam Requested parameter.
 */
void ColorClampDrawer::requestParam(s32 priority, s32 step, const ColorClampDrawParam& rParam) {
    mRequestInterp->requestParam(priority, step, rParam);
}

/**
 * Checks whether the current parameter is enabled.
 * @return Whether drawing is enabled.
 */
bool ColorClampDrawer::isEnable() const {
    return getCurrentParam()->isEnable();
}

}  // namespace al
