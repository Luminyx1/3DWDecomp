#include "Library/Effect/EffectShaderHolder.hpp"

#include <gfx/seadGraphicsContextMRT.h>
#include <arm_neon.h>
#include <math/seadMathCalcCommon.h>
#include <mc/seadCoreInfo.h>
#include <nn/os.h>
#include <nn/util/util_Matrix.h>
#include <nn/util/util_MatrixApi.h>
#include <nn/vfx/Callback.h>
#include <nn/vfx/EmitterRes.h>
#include <nn/vfx/EmitterSet.h>
#include <nn/vfx/System.h>

#include "common/aglShaderLocation.h"
#include "common/aglUniformBlock.h"
#include "driver/aglGraphicsDriverMgr.h"
#include "utility/aglPrimitiveTexture.h"

#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Effect/EffectSystem.hpp"
#include "Library/Effect/PtclSystem.hpp"
#include "Library/Fog/FogDirector.hpp"
#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/Light/DirectionalLightKeeper.hpp"
#include "Library/Light/LightIntensityDirector.hpp"
#include "Library/Light/PrePassLightKeeper.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Shader/Block/UniformBlock.hpp"
#include "Library/Shader/Block/UniformBlockUtil.hpp"
#include "Library/Shader/DeferredRendering/CubeMapDirector.hpp"
#include "Library/Shader/DeferredRendering/GBufferArray.hpp"

namespace {

using namespace al;

float32x4_t makeVector(f32 x, f32 y, f32 z) {
    f32 w = 0.0f;
    float32x2_t low = vcreate_f32(static_cast<u64>(*reinterpret_cast<u32*>(&x)) |
                                  static_cast<u64>(*reinterpret_cast<u32*>(&y)) << 32);
    float32x2_t high = vcreate_f32(static_cast<u64>(*reinterpret_cast<u32*>(&z)) |
                                   static_cast<u64>(*reinterpret_cast<u32*>(&w)) << 32);
    return vcombine_f32(low, high);
}

u32 getTextureId(const agl::TextureData& rTexture) {
    rTexture.getTexture().setReference_();
    return rTexture.getTextureID();
}

void setTextureSlot(PtclSystem* pPtclSystem, nn::vfx::TextureSlotId id, u32 textureId) {
    sead::CoreId coreId = sead::CoreInfo::getCurrentCoreId();
    nn::gfx::DescriptorSlot slot;
    slot.ToData()->value = textureId;
    pPtclSystem->SetTextureSlot(coreId, id, slot);
}

const agl::TextureSampler* getBlack2DSampler() {
    return agl::utl::PrimitiveTexture::instance()->getTextureSampler(
        agl::utl::PrimitiveTexture::cType_Black2D);
}

void bindCustomShaderTexture(nn::vfx::System* pSystem, s32 coreId,
                             nn::gfx::CommandBuffer* pCommandBuffer,
                             nn::vfx::CustomShaderTextureType type,
                             nn::gfx::DescriptorSlot textureSlot,
                             nn::gfx::DescriptorSlot samplerSlot) {
    nn::vfx::detail::Shader* shader = pSystem->GetCurrentShader(coreId);

    if (shader == nullptr) {
        return;
    }

    s32 pixelLocation = shader->GetCustomTexturePixelLocation(type);
    s32 vertexLocation = shader->GetCustomTextureVertexLocation(type);

    if (vertexLocation != -1) {
        pCommandBuffer->SetTextureAndSampler(vertexLocation, nn::gfx::ShaderStage_Vertex,
                                             textureSlot, samplerSlot);
    }

    if (pixelLocation != -1) {
        pCommandBuffer->SetTextureAndSampler(pixelLocation, nn::gfx::ShaderStage_Pixel, textureSlot,
                                             samplerSlot);
    }
}

nn::gfx::DescriptorSlot makeDescriptorSlot(u32 id) {
    nn::gfx::DescriptorSlot slot;
    slot.ToData()->value = id;
    return slot;
}

void bindCustomShaderTexture(nn::vfx::RenderStateSetArg& rArg,
                             nn::vfx::CustomShaderTextureType type,
                             const agl::TextureSampler* pSampler) {
    rArg.GetShader();
    pSampler->updateRegs();
    nn::gfx::DescriptorSlot samplerSlot = makeDescriptorSlot(pSampler->getSampler().getSamplerID());
    nn::gfx::DescriptorSlot textureSlot =
        makeDescriptorSlot(getTextureId(pSampler->getTextureData()));
    nn::vfx::System* system = rArg.GetSystem();
    bindCustomShaderTexture(system, sead::CoreInfo::getCurrentCoreId(), rArg.pCommandBuffer, type,
                            textureSlot, samplerSlot);
}

void calcParticleWorldPos(sead::Vector3f* pPos, const nn::vfx::Emitter* pEmitter, s32 index,
                          const nn::util::Float4& rLocalPos) {
    float32x4_t localPos = vdupq_n_f32(0.0f);
    localPos = vld1q_lane_f32(&rLocalPos.v[0], localPos, 0);
    localPos = vld1q_lane_f32(&rLocalPos.v[1], localPos, 1);
    localPos = vld1q_lane_f32(&rLocalPos.v[2], localPos, 2);
    const nn::util::Float4& row0 = pEmitter->m_ParticleEmitterMatrixRow[0][index];
    const nn::util::Float4& row1 = pEmitter->m_ParticleEmitterMatrixRow[1][index];
    const nn::util::Float4& row2 = pEmitter->m_ParticleEmitterMatrixRow[2][index];
    float32x4x4_t mtx;

    switch (pEmitter->m_pEmitterData->followType) {
    case 0:
        mtx = pEmitter->m_MatrixSrt._m;
        break;
    case 2:
        mtx.val[0] = makeVector(row0.x, row1.x, row2.x);
        mtx.val[1] = makeVector(row0.y, row1.y, row2.y);
        mtx.val[2] = makeVector(row0.z, row1.z, row2.z);
        mtx.val[3] = makeVector(vgetq_lane_f32(pEmitter->m_MatrixSrt._m.val[3], 0),
                                vgetq_lane_f32(pEmitter->m_MatrixSrt._m.val[3], 1),
                                vgetq_lane_f32(pEmitter->m_MatrixSrt._m.val[3], 2));
        break;
    default:
        mtx.val[0] = makeVector(row0.x, row1.x, row2.x);
        mtx.val[1] = makeVector(row0.y, row1.y, row2.y);
        mtx.val[2] = makeVector(row0.z, row1.z, row2.z);
        mtx.val[3] = makeVector(row0.w, row1.w, row2.w);
        break;
    }

    float32x4_t result = vmulq_laneq_f32(mtx.val[0], localPos, 0);
    result = vfmaq_laneq_f32(result, mtx.val[1], localPos, 1);
    result = vfmaq_laneq_f32(result, mtx.val[2], localPos, 2);
    result = vaddq_f32(mtx.val[3], result);
    vst1q_f32(&pPos->x, result);
}

void applyLightFade(sead::Color4f* pColor, f32 frame, f32 life, u8 fadeInFrame, u8 fadeOutFrame) {
    if (fadeInFrame != 0) {
        f32 rate = sead::Mathf::clamp(frame / fadeInFrame, 0.0f, 1.0f);

        if (rate < 1.0f) {
            pColor->setLerp(sead::Color4f::cBlack, *pColor, rate);
        }
    }

    if (fadeOutFrame != 0) {
        f32 rate =
            1.0f - sead::Mathf::clamp((frame - (life - fadeOutFrame)) / fadeOutFrame, 0.0f, 1.0f);

        if (rate < 1.0f) {
            pColor->setLerp(sead::Color4f::cBlack, *pColor, rate);
        }
    }
}

void requestParticlePointLight(PrePassLightKeeper* pKeeper, const nn::vfx::Emitter* pEmitter,
                               const sead::Color4f& rColor, u8 fadeInFrame, u8 fadeOutFrame,
                               f32 radius) {
    if (pEmitter == nullptr) {
        return;
    }

    s32 particleNum = pEmitter->m_ParticleNum;

    for (s32 i = 0; i < particleNum; i++) {
        const nn::vfx::detail::ParticleAttribute* attr = &pEmitter->m_ParticleAttr[i];

        if (attr == nullptr) {
            continue;
        }

        f32 createTime = attr->createTime;
        f32 life = attr->life;
        sead::Color4f color = rColor;
        applyLightFade(&color, pEmitter->m_Frame - createTime, life, fadeInFrame, fadeOutFrame);

        if (i > pEmitter->m_ParticleNum) {
            continue;
        }

        const nn::util::Float4* localPos = &pEmitter->m_ParticlePos[i];

        if (localPos == nullptr) {
            continue;
        }

        sead::Vector3f pos;
        calcParticleWorldPos(&pos, pEmitter, i, *localPos);
        pKeeper->requestPointLight(pos, radius, color, 0.95f, 0.0f, false, false,
                                   sead::Color4f::cWhite, 0);
    }
}

template <typename T>
void setUboValue(UniformBlock* pUbo, s32 index, const T& rValue) {
    T value = rValue;
    pUbo->setData(index, &value, 0, 1);
}

void flushUbo(const UniformBlock* pUbo) {
    u32 offset = pUbo->getCurrentBlockOffset(0);
    u32 size = pUbo->getBlockSize();
    agl::GPUMemVoidAddr(pUbo->getBuffer(), offset).flushCPUCache(size);
}

struct AreaLoopParam {
    float32x4_t pos;
    float32x4_t size;
    float32x4_t sizeInv;
    float32x4_t offset;
    float32x4_t color;
    float32x4_t rotate[3];
    float32x4_t rotateInv[3];
};

static_assert(sizeof(AreaLoopParam) == 0xb0);

void calcInverse3x3(float32x4x4_t* pOut, const float32x4x4_t& rMtx) {
    f32 m[3][3];

    for (s32 i = 0; i < 3; i++) {
        for (s32 j = 0; j < 3; j++) {
            m[i][j] = rMtx.val[i][j];
        }
    }

    f32 c00 = m[1][1] * m[2][2] - m[1][2] * m[2][1];
    f32 c01 = m[0][2] * m[2][1] - m[0][1] * m[2][2];
    f32 c02 = m[0][1] * m[1][2] - m[0][2] * m[1][1];
    f32 c10 = m[1][2] * m[2][0] - m[1][0] * m[2][2];
    f32 c11 = m[0][0] * m[2][2] - m[0][2] * m[2][0];
    f32 c12 = m[0][2] * m[1][0] - m[0][0] * m[1][2];
    f32 c20 = m[1][0] * m[2][1] - m[1][1] * m[2][0];
    f32 c21 = m[0][1] * m[2][0] - m[0][0] * m[2][1];
    f32 c22 = m[0][0] * m[1][1] - m[0][1] * m[1][0];
    f32 det = m[0][0] * c00 + m[0][1] * c10 + m[0][2] * c20;
    f32 invDet = det == 0.0f ? 0.0f : 1.0f / det;
    pOut->val[0] = makeVector(c00 * invDet, c01 * invDet, c02 * invDet);
    pOut->val[1] = makeVector(c10 * invDet, c11 * invDet, c12 * invDet);
    pOut->val[2] = makeVector(c20 * invDet, c21 * invDet, c22 * invDet);
    pOut->val[3] = vdupq_n_f32(0.0f);
}

struct FogColor {
    sead::Vector3f color;
    f32 intensity;
};

f32 calcInvRange(f32 start, f32 end) {
    return 1.0f / ((end == start ? end + 1.0f : end) - start);
}

}  // namespace

namespace al {

UniformBlockLayout cEffectSceneEffectUboLayout[23] = {
    {0, agl::UniformBlock::cType_Float, 1},  {1, agl::UniformBlock::cType_Vec3, 1},
    {2, agl::UniformBlock::cType_Vec3, 1},   {3, agl::UniformBlock::cType_Vec4, 3},
    {4, agl::UniformBlock::cType_Vec3, 1},   {5, agl::UniformBlock::cType_Vec3, 2},
    {6, agl::UniformBlock::cType_Int, 1},    {7, agl::UniformBlock::cType_Int, 1},
    {8, agl::UniformBlock::cType_Vec4, 1},   {9, agl::UniformBlock::cType_Float, 1},
    {10, agl::UniformBlock::cType_Float, 1}, {11, agl::UniformBlock::cType_Int, 1},
    {12, agl::UniformBlock::cType_Vec4, 1},  {13, agl::UniformBlock::cType_Float, 1},
    {14, agl::UniformBlock::cType_Float, 1}, {15, agl::UniformBlock::cType_Vec4, 1},
    {16, agl::UniformBlock::cType_Vec4, 1},  {17, agl::UniformBlock::cType_Vec4, 1},
    {18, agl::UniformBlock::cType_Float, 1}, {19, agl::UniformBlock::cType_Vec4, 1},
    {20, agl::UniformBlock::cType_Float, 1}, {21, agl::UniformBlock::cType_Vec2, 1},
    {22, agl::UniformBlock::cType_Vec2, 1},
};

UniformBlockLayout cEffectDitherUboLayout[1] = {
    {0, agl::UniformBlock::cType_Int, 256},
};

s32 gAreaLoopRepNum[4];

/**
 * Creates the effect scene uniform block and registers the custom shader and action callbacks.
 * @param pPtclSystem the particle system the callbacks are registered to
 * @param pDrawContext the draw context used by the render state callbacks
 * @param pEnvParam the effect environment parameter (unused)
 */
EffectShaderHolder::EffectShaderHolder(PtclSystem* pPtclSystem, agl::DrawContext* pDrawContext,
                                       EffectEnvParam* pEnvParam)
    : mPtclSystem(pPtclSystem), mDrawContext(pDrawContext) {
    mDitherScale.set(0.0f, 0.0f);
    mUbo = createUniformBlock(cEffectSceneEffectUboLayout, 23, nullptr, 2);

    mUbo->setData(8, &sead::Color4f::cWhite, 0, 1);
    mUbo->setData(17, &sead::Color4f::cWhite, 0, 1);
    mUbo->setValue(9, 1000.0f);
    mUbo->setValue(18, 0.001f);
    mUbo->setData(12, &sead::Color4f::cWhite, 0, 1);
    mUbo->setData(19, &sead::Color4f::cWhite, 0, 1);
    mUbo->setValue(13, 1000.0f);
    mUbo->setValue(20, 0.001f);
    mUbo->setValue(0, 1.0f);
    mUbo->setData(1, &sead::Color4f::cWhite, 0, 1);
    setUboValue(mUbo, 2, -sead::Vector3f::ey);
    mUbo->setData(21, &sead::Vector2f::zero, 0, 1);
    mUbo->setData(22, &sead::Vector2f::zero, 0, 1);
    nn::util::Matrix4x3fType identity;
    nn::util::MatrixIdentity(&identity);
    nn::util::FloatColumnMajor4x3 viewInvMtx;
    nn::util::MatrixStore(&viewInvMtx, identity);
    mUbo->setData(3, &viewInvMtx, 0, 3);

    nn::vfx::CallbackSet actionCallbackSet[5];
    nn::vfx::CallbackSet shaderCallbackSet[5];

    actionCallbackSet[0].emitterDraw = CustomActionEmitterDrawOverrideCallback;
    actionCallbackSet[0].emitterPostCalculate =
        CustomActionEmitterPostCalcCallbackPointLightEmitter;
    mPtclSystem->SetCallback(0, actionCallbackSet[0]);

    actionCallbackSet[1].emitterPostCalculate =
        CustomActionEmitterPostCalcCallbackPointLightPerticle;
    actionCallbackSet[1].emitterDraw = CustomActionEmitterDrawOverrideCallback;
    mPtclSystem->SetCallback(1, actionCallbackSet[1]);

    shaderCallbackSet[1].renderStateSet = StandardRenderStateCallback;
    mPtclSystem->SetCallback(9, shaderCallbackSet[1]);
    nn::vfx::CallbackSet standardCallbackSet;
    standardCallbackSet.renderStateSet = StandardRenderStateCallback;
    mPtclSystem->SetCallback(8, standardCallbackSet);
    mPtclSystem->SetCallback(14, standardCallbackSet);

    shaderCallbackSet[2].renderStateSet = CustomShaderRenderStateSetCallbackLensFlare;
    mPtclSystem->SetCallback(10, shaderCallbackSet[2]);
    shaderCallbackSet[3].renderStateSet = StandardRenderStateCallback;
    mPtclSystem->SetCallback(11, shaderCallbackSet[3]);
    shaderCallbackSet[4].renderStateSet = CustomShaderRenderStateSetCallbackNormalMap;
    mPtclSystem->SetCallback(12, shaderCallbackSet[4]);

    shaderCallbackSet[3].renderStateSet = AreaLoopRenderStateSetCallback;
    shaderCallbackSet[3].emitterDraw = AreaLoopDrawOverrideCallback;
    mPtclSystem->SetCallback(11, shaderCallbackSet[3]);

    mPtclSystem->SetDrawPathRenderStateSetCallback(nn::vfx::DrawPathCallbackId_0,
                                                   static_cast<nn::vfx::DrawPathFlag>(0xc1),
                                                   DrawPathRenderStateSetCallback);
    mIsDrawPathMRT = true;
}

/**
 * Sets the graphics system info and reserves point lights for effects.
 * @param pInfo the graphics system info
 */
void EffectShaderHolder::setGraphicsSystemInfo(const GraphicsSystemInfo* pInfo) {
    if (pInfo == nullptr) {
        mGraphicsSystemInfo = nullptr;
        return;
    }

    mGraphicsSystemInfo = pInfo;
    pInfo->getPrePassLightKeeper()->mPointLightNum += 64;
}

/**
 * Updates the scene uniform block from the exposure, light and fog settings.
 * @param rViewMtx the view matrix
 * @param rScreenSize the screen size
 */
void EffectShaderHolder::updateShaderParam(const sead::Matrix34f& rViewMtx,
                                           const sead::Vector2f& rScreenSize) {
    mUbo->setValue(
        0, 1.0f / sead::Mathf::clampMin(
                      mGraphicsSystemInfo->getLightIntensityDirector()->getExposure(), 0.0001f));
    mUbo->setData(1, &mGraphicsSystemInfo->getDirectionalLightKeeper()->getCurrentColor(), 0, 1);

    sead::Vector3f lightDir;
    mGraphicsSystemInfo->tryDirectionalLightInfo(&lightDir, nullptr, nullptr);
    lightDir.setRotated(rViewMtx, lightDir);
    normalizeOrZero(&lightDir);
    setUboValue(mUbo, 2, -lightDir);

    sead::Matrix34f viewInvMtx;
    viewInvMtx.setInverse(rViewMtx);
    mUbo->setData(3, &viewInvMtx, 0, 3);

    FogDirector* fogDirector = mGraphicsSystemInfo->getFogDirector();
    mUbo->setData(21, &mDitherScale, 0, 1);
    mUbo->setData(22, &rScreenSize, 0, 1);

    if (fogDirector != nullptr) {
        const FogParam& fogParam = fogDirector->getFogParam();
        FogColor fogColor = {reinterpret_cast<const sead::Vector3f&>(*fogParam.mColor),
                             sead::Mathf::max(*fogParam.mIntensityMax, 0.0f)};
        sead::Color4f fogMulColor =
            *fogParam.mIntensityMax > 0.0f ? *fogParam.mMulColor : sead::Color4f::cWhite;
        f32 fogNear = -fogParam.getStart();
        f32 fogInvRange = calcInvRange(fogNear, -fogParam.getEnd());
        mUbo->setData(8, &fogColor, 0, 1);
        mUbo->setData(17, &fogMulColor, 0, 1);
        mUbo->setValue(9, fogNear);
        mUbo->setValue(18, fogInvRange);

        const YFogParam& yFogParam = fogDirector->getYFogParam();
        FogColor yFogColor = {reinterpret_cast<const sead::Vector3f&>(*yFogParam.mColor),
                              sead::Mathf::max(*yFogParam.mIntensityMax, 0.0f)};
        sead::Color4f yFogMulColor =
            *yFogParam.mIntensityMax > 0.0f ? *yFogParam.mMulColor : sead::Color4f::cWhite;
        f32 yFogStart = yFogParam.getStart();
        f32 yFogEnd = yFogParam.getEnd();
        sead::Vector3f viewUp;
        viewUp.setRotated(rViewMtx, sead::Vector3f::ey);
        sead::Vector3f viewStartPos;
        viewStartPos.setMul(rViewMtx, sead::Vector3f(0.0f, yFogStart, 0.0f));
        sead::Vector3f viewEndPos;
        viewEndPos.setMul(rViewMtx, sead::Vector3f(0.0f, yFogEnd, 0.0f));
        f32 yFogNear = viewUp.dot(viewStartPos);
        f32 yFogInvRange = calcInvRange(yFogNear, viewUp.dot(viewEndPos));
        mUbo->setData(12, &yFogColor, 0, 1);
        mUbo->setData(19, &yFogMulColor, 0, 1);
        mUbo->setValue(13, yFogNear);
        mUbo->setValue(20, yFogInvRange);
    }

    flushUbo(mUbo);
}

/**
 * Swaps the buffer of the scene uniform block.
 */
void EffectShaderHolder::swapUbo() {
    mUbo->swap();
}

/**
 * Binds the scene uniform block to the custom shader slot.
 * @param pDrawContext the draw context
 */
void EffectShaderHolder::bindCustomShaderUbo(agl::DrawContext* pDrawContext) {
    agl::ShaderLocation location;
    location.setLocation(agl::cShaderType_Vertex, 12);
    location.setLocation(agl::cShaderType_Fragment, 12);
    location.setLocation(agl::cShaderType_Compute, 12);
    mUbo->activate(pDrawContext, location);
}

/**
 * Sets the color buffer texture.
 * @param pTexture the texture, or nullptr for a black texture
 */
void EffectShaderHolder::setupTextureColor(const agl::TextureData* pTexture) {
    if (pTexture != nullptr) {
        mColorSampler.applyTextureData(*pTexture);
        u32 textureId = getTextureId(*pTexture);
        setTextureSlot(mPtclSystem, nn::vfx::TextureSlotId_FrameBuffer, textureId);
        return;
    }

    const agl::TextureData& black = getBlack2DSampler()->getTextureData();
    mColorSampler.applyTextureData(black);
    u32 textureId = getTextureId(black);
    setTextureSlot(mPtclSystem, nn::vfx::TextureSlotId_FrameBuffer, textureId);
}

/**
 * Sets the depth buffer texture.
 * @param pTexture the texture, or nullptr for a black texture
 */
void EffectShaderHolder::setupTextureDepth(const agl::TextureData* pTexture) {
    if (pTexture != nullptr) {
        mDepthSampler.applyTextureData(*pTexture);
        u32 textureId = getTextureId(*pTexture);
        setTextureSlot(mPtclSystem, nn::vfx::TextureSlotId_DepthBuffer, textureId);
        return;
    }

    const agl::TextureData& black = getBlack2DSampler()->getTextureData();
    mDepthSampler.applyTextureData(black);
    u32 textureId = getTextureId(black);
    setTextureSlot(mPtclSystem, nn::vfx::TextureSlotId_DepthBuffer, textureId);
}

/**
 * Sets the water depth buffer texture.
 * @param pTexture the texture, or nullptr for a black texture
 */
void EffectShaderHolder::setupTextureWaterDepth(const agl::TextureData* pTexture) {
    if (pTexture != nullptr) {
        mWaterDepthSampler.applyTextureData(*pTexture);
        u32 textureId = getTextureId(*pTexture);
        setTextureSlot(mPtclSystem, nn::vfx::TextureSlotId_DepthBuffer, textureId);
        return;
    }

    const agl::TextureData& black = getBlack2DSampler()->getTextureData();
    mWaterDepthSampler.applyTextureData(black);
    u32 textureId = getTextureId(black);
    setTextureSlot(mPtclSystem, nn::vfx::TextureSlotId_DepthBuffer, textureId);
}

/**
 * Sets the light buffer texture.
 * @param pTexture the texture
 */
void EffectShaderHolder::setupTextureLight(const agl::TextureData* pTexture) {
    if (pTexture != nullptr) {
        mLightSampler.applyTextureData(*pTexture);
    }
}

/**
 * Sets the dither texture and its scale.
 * @param pTexture the texture
 * @param rScale the dither scale
 */
void EffectShaderHolder::setupTextureDither(const agl::TextureData* pTexture,
                                            const sead::Vector2f& rScale) {
    if (pTexture == nullptr) {
        return;
    }

    mDitherSampler.applyTextureData(*pTexture);
    mDitherSampler.setWrap(1, 1, 1);
    mDitherSampler.setFilter(0, 0, 0);
    mDitherScale.x = rScale.x;
    mDitherScale.y = rScale.y;
}

/**
 * Sets the exposure texture to a white texture.
 * @param pTexture the texture (unused)
 */
void EffectShaderHolder::setupTextureExposure(const agl::TextureData* pTexture) {
    agl::TextureSampler sampler = *agl::utl::PrimitiveTexture::instance()->getTextureSampler(
        agl::utl::PrimitiveTexture::cType_White2D);
    mExposureSampler.applyTextureData(sampler.getTextureData());
}

/**
 * Sets the irradiance and mirror cube map textures.
 * @param pDirector the cube map director
 */
void EffectShaderHolder::setupTextureCubeMap(const CubeMapDirector* pDirector) {
    const agl::TextureSampler* sampler = pDirector->getIrradianceSampler(0);
    mIrradianceSampler[0].applyTextureData(sampler->getTextureData());
    sampler = pDirector->getIrradianceSampler(1);
    mIrradianceSampler[1].applyTextureData(sampler->getTextureData());
    sampler = pDirector->getCubeMapMirrorSampler(0);
    mCubeMapMirrorSampler[0].applyTextureData(sampler->getTextureData());
    sampler = pDirector->getCubeMapMirrorSampler(1);
    mCubeMapMirrorSampler[1].applyTextureData(sampler->getTextureData());
}

/**
 * Sets the render state of deferred effects.
 * @param rArg the render state argument
 */
void EffectShaderHolder::renderDeferred(nn::vfx::RenderStateSetArg& rArg) const {
    if (!mIsDrawPathMRT) {
        if (mIsDrawPathDepthShadow) {
            agl::driver::GraphicsDriverMgr::instance()->setDepthClamp(
                GameFrameworkNx::getAglDrawContext(), true);
        }

        return;
    }

    const nn::vfx::Emitter* emitter = rArg.pEmitter;
    sead::GraphicsContext context;
    GBufferArray::setContextMRTAlphaMask(static_cast<sead::GraphicsContextMRT*>(&context));

    switch (emitter->m_pEmitterData->maskType) {
    case 0:
        context.setCullingMode(0);
        break;
    case 1:
        context.setCullingMode(2);
        break;
    case 2:
        context.setCullingMode(1);
        break;
    }

    context.setPolygonOffsetBackEnable(true);
    context.apply(GameFrameworkNx::getDrawContext());
}

/**
 * Binds the shared custom shader parameters selected by the given flag.
 * @param rArg the render state argument
 * @param flag the custom shader flag of the emitter
 */
void EffectShaderHolder::CustomShaderRenderStateSetCallbackShared(nn::vfx::RenderStateSetArg& rArg,
                                                                  u64 flag) {
    nn::vfx::detail::Shader* shader = rArg.GetShader();
    nn::vfx::Emitter* emitter = rArg.pEmitter;
    nn::vfx::TemporaryBuffer* tempBuffer = rArg.pDrawParameterArg->m_pTemporaryBuffer;
    const EffectShaderHolder* holder = static_cast<const EffectShaderHolder*>(rArg.pUserParam);

    if (flag & 4) {
        nn::gfx::GpuAddress address;
        address.ToData()->value = 0;
        address.ToData()->impl = 0;
        f32* buffer = static_cast<f32*>(tempBuffer->Map(&address, sizeof(f32)));

        if (buffer != nullptr) {
            const f32* param = static_cast<const f32*>(emitter->m_pEmitterRes->m_CustomShaderParam);
            buffer[0] = param != nullptr ? param[12] : 0.0f;
        }

        shader->BindCustomShaderUniformBlock(
            rArg.pCommandBuffer, nn::vfx::CustomShaderConstantBufferIndex_2, &address, sizeof(f32));
    }

    if (flag & 8) {
        bindCustomShaderTexture(rArg, nn::vfx::CustomShaderTextureType_3, &holder->mDitherSampler);
        nn::gfx::GpuAddress address;
        address.ToData()->value = 0;
        address.ToData()->impl = 0;
        f32* buffer = static_cast<f32*>(tempBuffer->Map(&address, sizeof(f32) * 2));

        if (buffer != nullptr) {
            const f32* param = static_cast<const f32*>(emitter->m_pEmitterRes->m_CustomShaderParam);
            buffer[0] = param != nullptr ? param[10] : 0.0f;
            param = static_cast<const f32*>(emitter->m_pEmitterRes->m_CustomShaderParam);
            buffer[1] = param != nullptr ? param[11] : 0.0f;
        }

        shader->BindCustomShaderUniformBlock(rArg.pCommandBuffer,
                                             nn::vfx::CustomShaderConstantBufferIndex_0, &address,
                                             sizeof(f32) * 2);
    }

    if (flag & 0x10) {
        bindCustomShaderTexture(rArg, nn::vfx::CustomShaderTextureType_2,
                                &holder->mWaterDepthSampler);
    }
}

/**
 * Binds the scene uniform block and the common custom shader textures.
 * @param rArg the render state argument
 * @return true
 */
bool EffectShaderHolder::CustomShaderRenderStateSetCallbackCommon(
    nn::vfx::RenderStateSetArg& rArg) {
    nn::vfx::Emitter* emitter = rArg.pEmitter;
    const EffectShaderHolder* holder = static_cast<const EffectShaderHolder*>(rArg.pUserParam);
    agl::ShaderLocation location;
    location.setLocation(agl::cShaderType_Vertex, 12);
    location.setLocation(agl::cShaderType_Fragment, 12);
    location.setLocation(agl::cShaderType_Compute, 12);
    holder->mUbo->activate(GameFrameworkNx::getAglDrawContext(), location);

    const nn::vfx::detail::ResEmitter* resEmitter = emitter->m_pEmitterRes->m_pResEmitter;
    u64 switchFlag = resEmitter->customShaderSwitch;
    CustomShaderRenderStateSetCallbackShared(rArg, resEmitter->customShaderFlag);

    const agl::TextureSampler* cubeMap;

    if (switchFlag & 4) {
        cubeMap = &holder->mIrradianceSampler[0];
    } else if (switchFlag & 8) {
        cubeMap = &holder->mIrradianceSampler[1];
    } else if (switchFlag & 0x10) {
        cubeMap = &holder->mCubeMapMirrorSampler[1];
    } else if (switchFlag & 0x20) {
        cubeMap = &holder->mCubeMapMirrorSampler[1];
    } else {
        cubeMap = &holder->mIrradianceSampler[1];
    }

    bindCustomShaderTexture(rArg, nn::vfx::CustomShaderTextureType_12, cubeMap);
    bindCustomShaderTexture(rArg, nn::vfx::CustomShaderTextureType_0, &holder->mExposureSampler);
    return true;
}

/**
 * Binds the common custom shader parameters and the reserved constant buffer.
 * @param rArg the render state argument
 * @return true
 */
bool EffectShaderHolder::StandardRenderStateCallback(nn::vfx::RenderStateSetArg& rArg) {
    CustomShaderRenderStateSetCallbackCommon(rArg);
    nn::vfx::BindReservedCustomShaderConstantBuffer(rArg);
    return true;
}

/**
 * Binds the scene uniform block and the custom shader textures of normal mapped effects.
 * @param rArg the render state argument
 * @return true
 */
bool EffectShaderHolder::CustomShaderRenderStateSetCallbackNormalMap(
    nn::vfx::RenderStateSetArg& rArg) {
    rArg.GetShader();
    nn::vfx::Emitter* emitter = rArg.pEmitter;
    const EffectShaderHolder* holder = static_cast<const EffectShaderHolder*>(rArg.pUserParam);
    agl::ShaderLocation location;
    location.setLocation(agl::cShaderType_Vertex, 12);
    location.setLocation(agl::cShaderType_Fragment, 12);
    location.setLocation(agl::cShaderType_Compute, 12);
    holder->mUbo->activate(GameFrameworkNx::getAglDrawContext(), location);

    const nn::vfx::detail::ResEmitter* resEmitter = emitter->m_pEmitterRes->m_pResEmitter;
    u64 switchFlag = resEmitter->customShaderSwitch;
    CustomShaderRenderStateSetCallbackShared(rArg, resEmitter->customShaderFlag);

    const agl::TextureSampler* cubeMap;

    if (switchFlag & 1) {
        cubeMap = &holder->mIrradianceSampler[0];
    } else if (switchFlag & 2) {
        cubeMap = &holder->mIrradianceSampler[1];
    } else if (switchFlag & 4) {
        cubeMap = &holder->mCubeMapMirrorSampler[1];
    } else if (switchFlag & 8) {
        cubeMap = &holder->mCubeMapMirrorSampler[1];
    } else {
        cubeMap = &holder->mIrradianceSampler[1];
    }

    bindCustomShaderTexture(rArg, nn::vfx::CustomShaderTextureType_12, cubeMap);
    bindCustomShaderTexture(rArg, nn::vfx::CustomShaderTextureType_0, &holder->mExposureSampler);
    return true;
}

/**
 * Binds the common custom shader parameters and the lens flare parameters.
 * @param rArg the render state argument
 * @return true
 */
bool EffectShaderHolder::CustomShaderRenderStateSetCallbackLensFlare(
    nn::vfx::RenderStateSetArg& rArg) {
    CustomShaderRenderStateSetCallbackCommon(rArg);
    nn::vfx::EmitterResource* emitterRes = rArg.pEmitter->m_pEmitterRes;
    const void* param = emitterRes->m_CustomShaderParam;

    if (param != nullptr) {
        nn::gfx::GpuAddress address;
        address.ToData()->value = 0;
        address.ToData()->impl = 0;
        size_t size = emitterRes->m_CustomShaderParamSize;
        void* buffer = rArg.pDrawParameterArg->m_pTemporaryBuffer->Map(&address, size);

        if (buffer != nullptr) {
            memcpy(buffer, param, size);
            rArg.GetShader()->BindCustomShaderUniformBlock(
                rArg.pCommandBuffer, nn::vfx::CustomShaderConstantBufferIndex_0, &address, size);
        }
    }

    nn::vfx::BindReservedCustomShaderConstantBuffer(rArg);
    return true;
}

/**
 * Forwards the draw path render state to the holder.
 * @param rArg the render state argument
 */
void EffectShaderHolder::DrawPathRenderStateSetCallback(nn::vfx::RenderStateSetArg& rArg) {
    static_cast<const EffectShaderHolder*>(rArg.pUserParam)->renderDeferred(rArg);
}

/**
 * Stores the holder passed as draw user parameter in the emitter.
 * @param rArg the emitter draw argument
 * @return false
 */
bool EffectShaderHolder::CustomActionEmitterDrawOverrideCallback(nn::vfx::EmitterDrawArg& rArg) {
    rArg.pEmitter->m_UserData2 = rArg.pUserParam;
    return false;
}

/**
 * Requests a point light at the position of the emitter.
 * @param rArg the emitter post calculation argument
 */
void EffectShaderHolder::CustomActionEmitterPostCalcCallbackPointLightEmitter(
    nn::vfx::EmitterPostCalculateArg& rArg) {
    nn::vfx::Emitter* emitter = rArg.pEmitter;
    const EffectShaderHolder* holder = static_cast<const EffectShaderHolder*>(emitter->m_UserData2);

    if (holder == nullptr) {
        holder = static_cast<PtclSystem*>(emitter->m_EmitterSet->m_System)
                     ->getEffectSystem()
                     ->getShaderHolder();
    }

    const CustomActionDataPointLight* data =
        static_cast<const CustomActionDataPointLight*>(emitter->m_pEmitterRes->m_CustomActionParam);

    if (data == nullptr) {
        return;
    }

    PrePassLightKeeper* keeper = holder->mGraphicsSystemInfo->getPrePassLightKeeper();
    f32 radius = data->radius;
    sead::Vector3f pos;
    pos.x = emitter->m_EmitterLocalPos._v[0];
    pos.y = emitter->m_EmitterLocalPos._v[1];
    pos.z = emitter->m_EmitterLocalPos._v[2];
    sead::Color4f color = data->color;
    const nn::vfx::detail::ResEmitter* resEmitter = emitter->m_pEmitterData;
    u8 fadeInFrame = data->fadeInFrame;

    if (fadeInFrame != 0) {
        f32 rate = (emitter->m_Frame - resEmitter->emitEndFrame) / fadeInFrame;
        color.setLerp(sead::Color4f::cBlack, color, sead::Mathf::clamp(rate, 0.0f, 1.0f));
    }

    u8 fadeOutFrame = data->fadeOutFrame;

    if (fadeOutFrame != 0 && resEmitter->isLoop) {
        f32 rate = ((f32)(resEmitter->particleLife + resEmitter->emitEndFrame) - emitter->m_Frame) /
                   fadeOutFrame;
        color.setLerp(sead::Color4f::cBlack, color, sead::Mathf::clamp(rate, 0.0f, 1.0f));
    }

    keeper->requestPointLight(pos, radius, color, 0.95f, 0.0f, false, false, sead::Color4f::cWhite,
                              0);
}

/**
 * Requests a point light for every particle of the emitter and its children.
 * @param rArg the emitter post calculation argument
 */
void EffectShaderHolder::CustomActionEmitterPostCalcCallbackPointLightPerticle(
    nn::vfx::EmitterPostCalculateArg& rArg) {
    nn::vfx::Emitter* emitter = rArg.pEmitter;
    const EffectShaderHolder* holder = static_cast<const EffectShaderHolder*>(emitter->m_UserData2);

    if (holder == nullptr) {
        return;
    }

    const CustomActionDataPointLight* data =
        static_cast<const CustomActionDataPointLight*>(emitter->m_pEmitterRes->m_CustomActionParam);

    if (data == nullptr) {
        return;
    }

    PrePassLightKeeper* keeper = holder->mGraphicsSystemInfo->getPrePassLightKeeper();
    f32 radius = data->radius;
    sead::Color4f color = data->color;
    s32 fadeInFrame = data->fadeInFrame;
    s32 fadeOutFrame = data->fadeOutFrame;

    for (s32 i = 0; i < 16; i++) {
        requestParticlePointLight(keeper, emitter->m_ChildEmitter[i], color, fadeInFrame,
                                  fadeOutFrame, radius);
    }

    requestParticlePointLight(keeper, emitter, color, fadeInFrame, fadeOutFrame, radius);
}

/**
 * Advances a linear congruential random number and returns it as a rate.
 * @param pSeed the random seed, updated in place
 * @return the random rate in [0, 1)
 */
f32 GetSeadRand(u32* pSeed) {
    *pSeed = *pSeed * 0x526797f1 + 0xa6481f9b;
    return *pSeed * (1.0f / 4294967296.0f);
}

/**
 * Sets the current repeat count of the area loop drawing of a core.
 * @param index the core index
 * @param num the repeat count
 */
void SetAreaLoopRepeatNum(s32 index, s32 num) {
    gAreaLoopRepNum[index] = num;
}

/**
 * Gets the current repeat count of the area loop drawing of a core.
 * @param index the core index
 * @return the repeat count
 */
s32 GetAreaLoopRepeatNum(s32 index) {
    return gAreaLoopRepNum[index];
}

/**
 * Draws an area loop emitter several times.
 * @param rArg the emitter draw argument
 * @return whether the emitter was drawn
 */
bool EffectShaderHolder::AreaLoopDrawOverrideCallback(nn::vfx::EmitterDrawArg& rArg) {
    if (rArg.shaderType == nn::vfx::ShaderType_Compute) {
        return false;
    }

    nn::vfx::Emitter* emitter = rArg.pEmitter;
    nn::vfx::detail::Shader* shader = emitter->m_pEmitterRes->m_Shader[rArg.shaderType];

    if (shader == nullptr) {
        return false;
    }

    s32 coreId = nn::os::GetCurrentCoreNumber();
    const f32* param = static_cast<const f32*>(emitter->m_pEmitterRes->m_CustomShaderParam);
    f32 repeat = param != nullptr ? param[0] : 0.0f;
    u32 repeatNum = repeat > 0.0f ? repeat : -repeat;

    if (repeatNum == 0) {
        return true;
    }

    rArg.pCommandBuffer->SetShader(shader->GetGfxShader(), nn::gfx::ShaderStageBit_All);

    for (u32 i = 0; i != repeatNum; i++) {
        gAreaLoopRepNum[coreId] = i;
        emitter->m_EmitterCalculator->DrawEmitterUsingBoundShader(
            rArg.pCommandBuffer, emitter, shader, rArg.pUserParam, rArg.pDrawParameterArg);
    }

    return true;
}

/**
 * Binds the area loop parameters and the common custom shader parameters.
 * @param rArg the render state argument
 * @return true
 */
bool EffectShaderHolder::AreaLoopRenderStateSetCallback(nn::vfx::RenderStateSetArg& rArg) {
    s32 coreId = nn::os::GetCurrentCoreNumber();
    nn::vfx::Emitter* emitter = rArg.pEmitter;
    nn::vfx::detail::Shader* shader = rArg.GetShader();

    if (shader == nullptr) {
        return true;
    }

    nn::gfx::GpuAddress address;
    address.ToData()->value = 0;
    address.ToData()->impl = 0;
    AreaLoopParam* buffer = static_cast<AreaLoopParam*>(
        rArg.pDrawParameterArg->m_pTemporaryBuffer->Map(&address, sizeof(AreaLoopParam)));

    if (buffer != nullptr) {
        const f32* param = static_cast<const f32*>(emitter->m_pEmitterRes->m_CustomShaderParam);
        f32 sizeX = 0.0f;
        f32 sizeY = 0.0f;
        f32 sizeZ = 0.0f;
        f32 colorR = 0.0f;
        f32 colorG = 0.0f;
        f32 colorB = 0.0f;

        if (param != nullptr) {
            sizeX = param[1];
            sizeY = param[2];
            sizeZ = param[3];
            colorR = param[4];
            colorG = param[5];
            colorB = param[6];
        }

        float32x4_t volumeScale = emitter->m_EmitterSet->m_EmitterVolumeScale._v;
        float32x4_t translation = emitter->m_MatrixSrt._m.val[3];
        buffer->pos = vsetq_lane_f32(vgetq_lane_f32(translation, 0), buffer->pos, 0);
        buffer->pos = vsetq_lane_f32(vgetq_lane_f32(translation, 1), buffer->pos, 1);
        buffer->pos = vsetq_lane_f32(vgetq_lane_f32(translation, 2), buffer->pos, 2);
        buffer->pos = vsetq_lane_f32(0.0f, buffer->pos, 3);
        sizeX *= vgetq_lane_f32(volumeScale, 0);
        sizeY *= vgetq_lane_f32(volumeScale, 1);
        sizeZ *= vgetq_lane_f32(volumeScale, 2);

        const float32x4x4_t& rotate = nn::util::MatrixRowMajor4x3f::ConstantIdentity._m;
        float32x4x4_t inverse;
        calcInverse3x3(&inverse, rotate);

        for (s32 i = 0; i < 3; i++) {
            buffer->rotate[i] =
                makeVector(vgetq_lane_f32(rotate.val[i], 0), vgetq_lane_f32(rotate.val[i], 1),
                           vgetq_lane_f32(rotate.val[i], 2));
            buffer->rotateInv[i] = inverse.val[i];
        }

        buffer->size = makeVector(sizeX, sizeY, sizeZ);
        buffer->sizeInv =
            vsetq_lane_f32(isNearZero(sizeX) ? 1000000.0f : 1.0f / sizeX, buffer->sizeInv, 0);
        buffer->sizeInv =
            vsetq_lane_f32(isNearZero(sizeY) ? 1000000.0f : 1.0f / sizeY, buffer->sizeInv, 1);
        buffer->sizeInv =
            vsetq_lane_f32(isNearZero(sizeZ) ? 1000000.0f : 1.0f / sizeZ, buffer->sizeInv, 2);
        buffer->sizeInv = vsetq_lane_f32(0.0f, buffer->sizeInv, 3);

        u32 seed = gAreaLoopRepNum[coreId] * 0x068847d7 + 0x2ecc9b50;
        f32 randomX = seed * (1.0f / 4294967296.0f) + 0.2f;
        f32 randomY = GetSeadRand(&seed) + 0.2f;
        f32 randomZ = GetSeadRand(&seed) + 0.2f;
        buffer->offset = makeVector(sizeX * 1333.0f * randomX, sizeY * 1333.0f * randomY,
                                    sizeZ * 1333.0f * randomZ);
        buffer->color = makeVector(colorR, colorG, colorB);
        shader->BindCustomShaderUniformBlock(rArg.pCommandBuffer,
                                             nn::vfx::CustomShaderConstantBufferIndex_0, &address,
                                             sizeof(AreaLoopParam));
    }

    CustomShaderRenderStateSetCallbackCommon(rArg);
    nn::vfx::BindReservedCustomShaderConstantBuffer(rArg);
    return true;
}

}  // namespace al
