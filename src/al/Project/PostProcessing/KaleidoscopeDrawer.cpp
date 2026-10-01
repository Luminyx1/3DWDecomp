#include "Project/PostProcessing/KaleidoscopeDrawer.hpp"

#include <gfx/seadGraphicsContext.h>

#include "common/aglRenderBuffer.h"
#include "common/aglRenderTarget.h"
#include "common/aglTextureSampler.h"
#include "utility/aglDynamicTextureAllocator.h"

#include "Library/Debug/Render/RenderBufferAttacher.hpp"
#include "Library/Shader/Block/UniformBlock.hpp"
#include "Library/Shader/Block/UniformBlockUtil.hpp"
#include "Library/Shader/ForwardRendering/ShaderHolder.hpp"
#include "Project/PostProcessing/PostProcessingDrawerUtil.hpp"

namespace {

const al::UniformBlockLayout cKaleidoscopeLayout[] = {{0, agl::UniformBlock::cType_Int, 1}};

const char* const cNumberText[] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};

}  // namespace

namespace al {

/**
 * Constructs the kaleidoscope parameters.
 */
KaleidoscopeParam::KaleidoscopeParam() {
    mParamObj = new ParameterObj();
    mIsEnable = new ParameterBool(false, mParamObj, "IsEnable", "有効", "", true);
    mMirrorType = new ParameterS32(3, mParamObj, "MirrorType", "タイプ", "Min=0, Max=1", true);
    mDivideNum = new ParameterS32(4, mParamObj, "RadialDivNum", "分割数", "Min=1, Max=32", true);
    mIsDistort = new ParameterBool(true, mParamObj, "IsDistort", "中心圧縮する", "", true);
}

/**
 * Checks the Enable flag.
 * @return Whether Enable is set.
 */
bool KaleidoscopeParam::isEnable() const {
    return mIsEnable->getValue();
}

/**
 * Gets the MirrorType parameter.
 * @return MirrorType.
 */
s32 KaleidoscopeParam::getMirrorType() const {
    return mMirrorType->getValue();
}

/**
 * Gets the DivideNum parameter.
 * @return DivideNum.
 */
s32 KaleidoscopeParam::getDivideNum() const {
    return mDivideNum->getValue();
}

/**
 * Checks the Distort flag.
 * @return Whether Distort is set.
 */
bool KaleidoscopeParam::isDistort() const {
    return mIsDistort->getValue();
}

/**
 * Gets the kaleidoscope shader and creates the parameter interpolation and uniform block.
 * @param pShaderHolder Shader holder to get the shader from.
 */
KaleidoscopeDrawer::KaleidoscopeDrawer(ShaderHolder* pShaderHolder)
    : mShaderHolder(pShaderHolder), mRequestInterp(nullptr), mUniformBlock(nullptr) {
    mShaderProgram = pShaderHolder->getShaderProgram("alRenderKaleidoscope");
    ParamRequestInterp* interp = new ParamRequestInterp();
    mRequestInterp = interp;
    interp->mCurrentParam = new KaleidoscopeParam();
    interp->mStartParam = new KaleidoscopeParam();
    interp->mEndParam = new KaleidoscopeParam();
    interp->mRequestParam = new KaleidoscopeParam();
    mUniformBlock = createUniformBlock(cKaleidoscopeLayout, 1, nullptr, 2);
}

/**
 * Destroys the uniform block.
 */
KaleidoscopeDrawer::~KaleidoscopeDrawer() {
    if (mUniformBlock != nullptr) {
        delete mUniformBlock;
        mUniformBlock = nullptr;
    }
}

/**
 * Finishes initialization of the parameter interpolation.
 */
void KaleidoscopeDrawer::endInit() {
    mRequestInterp->endInit();
}

/**
 * Clears the parameter request.
 */
void KaleidoscopeDrawer::clearRequest() {
    mRequestInterp->clearRequest();
}

/**
 * Updates the parameter interpolation and writes the division count to the uniform block.
 */
void KaleidoscopeDrawer::update() {
    mRequestInterp->updateInterp();
    UniformBlockSetter setter(mUniformBlock, 0);
    mUniformBlock->setValue(0, getCurrentParam()->getDivideNum());
}

/**
 * Gets the current interpolated parameter.
 * @return Current parameter.
 */
const KaleidoscopeParam* KaleidoscopeDrawer::getCurrentParam() const {
    return static_cast<const KaleidoscopeParam*>(mRequestInterp->getCurrentParam());
}

/**
 * Draws the kaleidoscope effect onto the color target of a render buffer.
 * @param pContext Draw context.
 * @param rBuffer Render buffer to draw onto.
 */
void KaleidoscopeDrawer::draw(agl::DrawContext* pContext, const agl::RenderBuffer& rBuffer) const {
    const KaleidoscopeParam* param = getCurrentParam();

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

    const char* macros[] = {"MIRROR_TYPE", "IS_DISTORT"};
    const char* values[2];
    values[0] = cNumberText[param->getMirrorType()];
    values[1] = param->isDistort() ? "1" : "0";

    const agl::ShaderProgram* program = mShaderHolder->getShaderProgram("alRenderKaleidoscope");
    program = program->searchVariation(2, macros, values);
    program->activate(pContext, true);

    agl::SamplerLocation colorLocation("uOrgColor");
    colorLocation.search(*program);
    agl::TextureSampler colorSampler;
    colorSampler.applyTextureData(*copy);
    colorSampler.activate(pContext, colorLocation, -1, false);

    s32 mirrorType = param->getMirrorType();

    if (mirrorType == 3 || mirrorType == 4) {
        setUniformBlockToShader(mUniformBlock, pContext, *program, "KaleidoscopeParam", 0);
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

    allocator->free(copy);
}

/**
 * Requests a parameter.
 * @param priority Request priority.
 * @param step Interpolation steps.
 * @param rParam Requested parameter.
 */
void KaleidoscopeDrawer::requestParam(s32 priority, s32 step, const KaleidoscopeParam& rParam) {
    mRequestInterp->requestParam(priority, step, rParam);
}

/**
 * Checks whether the current parameter is enabled.
 * @return Whether drawing is enabled.
 */
bool KaleidoscopeDrawer::isEnable() const {
    return getCurrentParam()->isEnable();
}

}  // namespace al
