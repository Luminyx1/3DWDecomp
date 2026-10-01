#include "Library/PostProcessing/LightStreakDirector.hpp"

#include <cmath>
#include <math/seadMathCalcCommon.h>

#include <common/aglRenderBuffer.h>
#include <common/aglShaderLocation.h>
#include <common/aglTextureData.h>
#include <utility/aglDynamicTextureAllocator.h>

#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Shader/Block/UniformBlock.hpp"
#include "Library/Shader/Block/UniformBlockUtil.hpp"
#include "Library/Shader/DeferredRendering/FullScreenQuadModel.hpp"
#include "Library/Shader/ForwardRendering/ShaderHolder.hpp"

namespace {
const al::UniformBlockLayout cLightStreakUboLayout[] = {
    {0, agl::UniformBlock::cType_Float, 1}, {1, agl::UniformBlock::cType_Float, 1},
    {2, agl::UniformBlock::cType_Float, 1}, {3, agl::UniformBlock::cType_Vec2, 6},
    {4, agl::UniformBlock::cType_Vec3, 3},
};

agl::DrawContext* getDrawContext() {
    return al::GameFrameworkNx::getAglDrawContext();
}

template <typename T>
inline void setUniformData(const al::UniformBlock* pBlock, s32 memberIndex, T value,
                           s32 arrayIndex) {
    pBlock->setData(memberIndex, &value, arrayIndex, 1);
}
}  // namespace

namespace al {

/**
 * Checks whether the light streak is visible at all.
 * @return Whether the intensity is non-zero.
 */
bool LightStreakParam::isEnable() const {
    return *mIntensity != 0.0f;
}

/**
 * Compares all parameter values with another light streak parameter.
 * @param rOther Parameter to compare with.
 * @return Whether all values are equal.
 */
bool LightStreakParam::operator==(const LightStreakParam& rOther) const {
    if (*mIntensity != *rOther.mIntensity)
        return false;
    if (*mStreakScale != *rOther.mStreakScale)
        return false;
    if (*mAttn != *rOther.mAttn)
        return false;
    if (*mThreshold != *rOther.mThreshold)
        return false;
    if (*mRotateDegree != *rOther.mRotateDegree)
        return false;
    if (*mStreakType != *rOther.mStreakType)
        return false;
    if (*mPassNum != *rOther.mPassNum)
        return false;
    if (!(*mStreakColor1 == *rOther.mStreakColor1))
        return false;
    if (!(*mStreakColor2 == *rOther.mStreakColor2))
        return false;
    return *mStreakColor3 == *rOther.mStreakColor3;
}

/**
 * Copies all parameter values from another light streak parameter.
 * @param rOther Parameter to copy from.
 * @return This parameter.
 */
LightStreakParam& LightStreakParam::operator=(const LightStreakParam& rOther) {
    *mIntensity = *rOther.mIntensity;
    *mStreakScale = *rOther.mStreakScale;
    *mAttn = *rOther.mAttn;
    *mThreshold = *rOther.mThreshold;
    *mRotateDegree = *rOther.mRotateDegree;
    *mStreakType = *rOther.mStreakType;
    *mPassNum = *rOther.mPassNum;
    *mStreakColor1 = *rOther.mStreakColor1;
    *mStreakColor2 = *rOther.mStreakColor2;
    *mStreakColor3 = *rOther.mStreakColor3;
    return *this;
}

/**
 * Interpolates between two light streak parameters. Integer values switch at the halfway point.
 * @param rA Parameter at rate 0.
 * @param rB Parameter at rate 1.
 * @param rate Interpolation rate.
 */
void LightStreakParam::interp(const LightStreakParam& rA, const LightStreakParam& rB, f32 rate) {
    mIntensity.copyLerp(rA.mIntensity, rB.mIntensity, rate);
    mStreakScale.copyLerp(rA.mStreakScale, rB.mStreakScale, rate);
    mAttn.copyLerp(rA.mAttn, rB.mAttn, rate);
    mThreshold.copyLerp(rA.mThreshold, rB.mThreshold, rate);
    mRotateDegree.copyLerp(rA.mRotateDegree, rB.mRotateDegree, rate);
    mStreakColor1.copyLerp(rA.mStreakColor1, rB.mStreakColor1, rate);
    mStreakColor2.copyLerp(rA.mStreakColor2, rB.mStreakColor2, rate);
    mStreakColor3.copyLerp(rA.mStreakColor3, rB.mStreakColor3, rate);
    mStreakType.copy(rate < 0.5f ? rA.mStreakType : rB.mStreakType);
    mPassNum.copy(rate < 0.5f ? rA.mPassNum : rB.mPassNum);
}

/**
 * Creates the light streak director, its shaders and uniform blocks.
 * @param pInfo Graphics system info.
 */
LightStreakDirector::LightStreakDirector(GraphicsSystemInfo* pInfo)
    : GraphicsParamRequestInterpKeeper<LightStreakParam>(pInfo, 14, "LightStreak", "aglgodray",
                                                         nullptr) {
    mMaskShader = ShaderHolder::sInstance->getShaderProgram("MakeLightStreakMask");
    mBlurShader = ShaderHolder::sInstance->getShaderProgram("MakeLightStreakBlur");
    mComposeShader = ShaderHolder::sInstance->getShaderProgram("ComposeLightStreak");
    mFullScreenQuadModel = new FullScreenQuadModel();

    for (s32 i = 0; i < mUniformBlocks.capacity(); i++)
        mUniformBlocks.pushBack(createUniformBlock(cLightStreakUboLayout, 5, nullptr, 2));
}

/**
 * Destroys the full screen quad and the uniform blocks.
 */
LightStreakDirector::~LightStreakDirector() {
    if (mFullScreenQuadModel != nullptr) {
        delete mFullScreenQuadModel;
        mFullScreenQuadModel = nullptr;
    }

    while (!mUniformBlocks.isEmpty()) {
        UniformBlock* uniformBlock = mUniformBlocks.popBack();

        if (uniformBlock != nullptr)
            delete uniformBlock;
    }
}

/**
 * Loads the stage specific light streak parameters.
 * @param pResource Stage resource.
 * @param pStageName Stage name.
 */
void LightStreakDirector::initStageResource(const Resource* pResource, const char* pStageName) {
    GraphicsParamRequestInterpKeeperImpl::initStageResource(pResource, pStageName);
}

/**
 * Checks whether the current light streak is visible.
 * @return Whether the light streak is enabled.
 */
bool LightStreakDirector::isEnable() const {
    return getCurrentParam().isEnable();
}

/**
 * Overrides the intensity of the current light streak parameter.
 * @param intensity New intensity.
 */
void LightStreakDirector::setIntensity(f32 intensity) {
    getCurrentParam().setIntensity(intensity);
}

/**
 * Updates the uniform blocks of every blur pass.
 * @param width Width of the blur buffer.
 * @param height Height of the blur buffer.
 */
void LightStreakDirector::updateUbo(s32 width, s32 height) const {
    const LightStreakParam& param = getCurrentParam();
    f32 angleStep;

    switch (param.getStreakType()) {
    case 1:
        angleStep = 72.0f;
        break;
    case 2:
        angleStep = 60.0f;
        break;
    default:
        angleStep = 90.0f;
        break;
    }

    f32 invWidth = 1.0f / width;
    f32 invHeight = 1.0f / height;
    f32 attn = param.getAttn();

    for (s32 i = 0; i < mUniformBlocks.size(); i++) {
        f32 passScale = exp2f(2.0f * i);
        f32 passAttn = powf(attn, passScale);
        UniformBlockSetter setter(mUniformBlocks[i], 0);
        setUniformData(mUniformBlocks(i), 1, passAttn, 0);
        setUniformData(mUniformBlocks(i), 0, param.getIntensity(), 0);
        setUniformData(mUniformBlocks(i), 2, param.getThreshold(), 0);

        f32 angle = param.getRotateDegree();

        for (s32 j = 0; j < 6; j++) {
            f32 radian = angle * (sead::Mathf::pi() / 180.0f);
            f32 cos = cosf(radian);
            f32 sin = sinf(radian);
            sead::Vector2f direction(invWidth * cos * (passScale * param.getStreakScale()),
                                     invHeight * sin * (passScale * param.getStreakScale()));
            mUniformBlocks(i)->setData(3, &direction, j, 1);
            angle = wrapAngle(angle + angleStep);
        }

        mUniformBlocks(i)->setData(4, &param.getStreakColor1(), 0, 1);
        mUniformBlocks(i)->setData(4, &param.getStreakColor2(), 1, 1);
        mUniformBlocks(i)->setData(4, &param.getStreakColor3(), 2, 1);
    }
}

/**
 * Draws the light streak of the given texture and composes it into the render buffer.
 * @param index View index.
 * @param rRenderBuffer Render buffer to compose into.
 * @param rViewport Viewport of the render buffer.
 * @param rTexture Source texture.
 * @param shaderMode Current shader mode.
 * @return Shader mode after drawing.
 */
agl::ShaderMode LightStreakDirector::drawToRenderBuffer(s32 index,
                                                        const agl::RenderBuffer& rRenderBuffer,
                                                        const sead::Viewport& rViewport,
                                                        const agl::TextureData& rTexture,
                                                        agl::ShaderMode shaderMode) const {
    s32 width = rTexture.getWidth(0) / 4;
    s32 height = rTexture.getHeight(0) / 4;
    updateUbo(width, height);

    agl::utl::DynamicTextureAllocator* allocator = agl::utl::DynamicTextureAllocator::instance();
    agl::TextureData* maskTexture = allocator->alloc(
        getDrawContext(), "Light Streak Mask", agl::TextureFormat::cTextureFormat_R11_G11_B10_float,
        width, height, 1, nullptr, agl::utl::DynamicTextureAllocator::cAllocateType_0, true, false);
    shaderMode = drawMask(index, width, height, rTexture, *maskTexture, shaderMode);
    agl::TextureData* streakTexture =
        allocator->allocArray(getDrawContext(), "Light Streak Texture Array",
                              agl::TextureFormat::cTextureFormat_R11_G11_B10_float, width, height,
                              6, 1, nullptr, agl::utl::DynamicTextureAllocator::cAllocateType_0,
                              true, false);
    agl::TextureData* tmpTexture =
        allocator->allocArray(getDrawContext(), "Tmp Texture Array",
                              agl::TextureFormat::cTextureFormat_R11_G11_B10_float, width, height,
                              6, 1, nullptr, agl::utl::DynamicTextureAllocator::cAllocateType_0,
                              true, false);

    switch (getCurrentParam().getPassNum()) {
    case 1:
        shaderMode =
            drawBlurMrt(index, width, height, *maskTexture, *streakTexture, 0, shaderMode);
        break;
    case 2:
        shaderMode = drawBlurMrt(index, width, height, *maskTexture, *tmpTexture, 0, shaderMode);
        shaderMode = drawBlurMrt(index, width, height, *tmpTexture, *streakTexture, 1, shaderMode);
        break;
    case 3:
        shaderMode =
            drawBlurMrt(index, width, height, *maskTexture, *streakTexture, 0, shaderMode);
        shaderMode = drawBlurMrt(index, width, height, *streakTexture, *tmpTexture, 1, shaderMode);
        shaderMode = drawBlurMrt(index, width, height, *tmpTexture, *streakTexture, 2, shaderMode);
        break;
    default:
        shaderMode = drawBlurMrt(index, width, height, *maskTexture, *tmpTexture, 0, shaderMode);
        shaderMode = drawBlurMrt(index, width, height, *tmpTexture, *streakTexture, 1, shaderMode);
        shaderMode = drawBlurMrt(index, width, height, *streakTexture, *tmpTexture, 2, shaderMode);
        shaderMode = drawBlurMrt(index, width, height, *tmpTexture, *streakTexture, 3, shaderMode);
        break;
    }

    allocator->free(tmpTexture);
    shaderMode = composeBlurToBuffer(index, width, height, rRenderBuffer, rViewport,
                                     *streakTexture, shaderMode);
    allocator->free(streakTexture);
    allocator->free(maskTexture);

    agl::UniformBlockLocation location;
    location.setName("LightStreakInfo");
    location.search(*mBlurShader);
    return shaderMode;
}

}  // namespace al
