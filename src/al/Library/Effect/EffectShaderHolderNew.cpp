#include "Library/Effect/EffectShaderHolderNew.hpp"

#include <arm_neon.h>
#include <attributes.h>
#include <gfx/seadGraphicsContext.h>
#include <gfx/seadGraphicsContextMRT.h>
#include <math/seadMathCalcCommon.h>
#include <mc/seadCoreInfo.h>
#include <nerd/nerdMath.h>
#include <nn/vfx/Callback.h>
#include <nn/vfx/EmitterRes.h>
#include <nn/vfx/EmitterSet.h>
#include <nn/util/util_VectorApi.h>
#include <nn/vfx/System.h>

#include "common/aglShaderLocation.h"
#include "common/aglUniformBlock.h"
#include "utility/aglPrimitiveTexture.h"

#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Effect/EffectSystem.hpp"
#include "Library/Effect/PtclSystem.hpp"
#include "Library/Fog/FogDirector.hpp"
#include "Library/Light/PrePassLightKeeper.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Screen/ScreenFunction.hpp"
#include "Library/Shader/Block/UniformBlock.hpp"
#include "Library/Shader/Block/UniformBlockUtil.hpp"
#include "Library/Shader/DeferredRendering/CubeMapDirector.hpp"
#include "Library/Shader/DeferredRendering/GBufferArray.hpp"
#include "Project/Effect/EffectCameraHolder.hpp"
#include "Project/Effect/EffectEnvParam.hpp"
#include "Project/Effect/EffectUtil.hpp"

namespace {

using namespace al;

struct ScreenRect {
    f32 values[8];
};

EffectShaderHolderNew* getShaderHolder(const nn::vfx::Emitter* pEmitter) {
    return static_cast<EffectShaderHolderNew*>(
        static_cast<PtclSystem*>(pEmitter->m_EmitterSet->m_System)
            ->getEffectSystem()
            ->getShaderHolder());
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

u32 getTextureId(const agl::TextureData& rTexture) {
    rTexture.getTexture().setReference_();
    return rTexture.getTextureID();
}

nn::gfx::DescriptorSlot makeDescriptorSlot(u32 id) {
    nn::gfx::DescriptorSlot slot;
    slot.ToData()->value = id;
    return slot;
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

void bindCustomShaderTexture(nn::vfx::System* pSystem, agl::DrawContext* pDrawContext,
                             nn::vfx::CustomShaderTextureType type,
                             const agl::TextureSampler* pSampler) {
    pSampler->updateRegs();
    nn::gfx::DescriptorSlot samplerSlot = makeDescriptorSlot(pSampler->getSampler().getSamplerID());
    nn::gfx::DescriptorSlot textureSlot =
        makeDescriptorSlot(getTextureId(pSampler->getTextureData()));
    bindCustomShaderTexture(pSystem, sead::CoreInfo::getCurrentCoreId(),
                            pDrawContext->getCommandBuffer(), type, textureSlot, samplerSlot);
}

float32x4_t makeVector(f32 x, f32 y, f32 z) {
    f32 w = 0.0f;
    float32x2_t low = vcreate_f32(static_cast<u64>(*reinterpret_cast<u32*>(&x)) |
                                  static_cast<u64>(*reinterpret_cast<u32*>(&y)) << 32);
    float32x2_t high = vcreate_f32(static_cast<u64>(*reinterpret_cast<u32*>(&z)) |
                                   static_cast<u64>(*reinterpret_cast<u32*>(&w)) << 32);
    return vcombine_f32(low, high);
}

void calcParticleWorldPos(nn::util::Vector3fType* pPos, const nn::vfx::Emitter* pEmitter, s32 index,
                          const nn::util::Vector3fType& rLocalPos) {
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

    float32x4_t result = vmulq_laneq_f32(mtx.val[0], rLocalPos._v, 0);
    result = vfmaq_laneq_f32(result, mtx.val[1], rLocalPos._v, 1);
    result = vfmaq_laneq_f32(result, mtx.val[2], rLocalPos._v, 2);
    pPos->_v = vaddq_f32(mtx.val[3], result);
}

struct CustomFieldParam {
    u32 flags;
    f32 windScale;
    f32 range;
    f32 power;
};

void calcInverseTransform(sead::Vector3f* pOut, const float32x4x4_t& rMtx,
                          const sead::Vector3f& rPos) {
    f32 m[4][3];

    for (s32 i = 0; i < 4; i++) {
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
    f32 x = rPos.x - m[3][0];
    f32 y = rPos.y - m[3][1];
    f32 z = rPos.z - m[3][2];
    pOut->x = (x * c00 + y * c10 + z * c20) * invDet;
    pOut->y = (x * c01 + y * c11 + z * c21) * invDet;
    pOut->z = (x * c02 + y * c12 + z * c22) * invDet;
}

f32 calcEmitterScale(const nn::vfx::Emitter* pEmitter) {
    return pEmitter->m_EmitterAnimScale * pEmitter->m_EmitterSetScale;
}

}  // namespace

namespace al {

/**
 * Creates the dither uniform block and registers the custom action, shader and field callbacks.
 * @param pPtclSystem the particle system the callbacks are registered to
 * @param pDrawContext the draw context used by the render state callbacks
 * @param pEnvParam the effect environment parameter
 */
EffectShaderHolderNew::EffectShaderHolderNew(PtclSystem* pPtclSystem,
                                             agl::DrawContext* pDrawContext,
                                             EffectEnvParam* pEnvParam)
    : EffectShaderHolder(pPtclSystem, pDrawContext, pEnvParam), mEnvParam(pEnvParam) {
    mLightDirector = new EffectLightDirector();
    mLensFlareRect = reinterpret_cast<sead::Vector2f*>(new ScreenRect());

    mUbo->setData(4, &sead::Vector3f::zero, 0, 1);
    mUbo->setData(5, &sead::Vector3f::zero, 0, 1);
    mUbo->setValue(6, 0);

    s32 bayerMatrix[256];
    makeBayerMatrix(bayerMatrix, 4);
    mDitherUbo = createUniformBlock(cEffectDitherUboLayout, 1, nullptr, 2);
    mDitherUbo->setData(0, bayerMatrix, 0, 256);
    flushUbo(mDitherUbo);
    mDitherUbo->swap();
    mDitherUbo->setData(0, bayerMatrix, 0, 256);
    flushUbo(mDitherUbo);

    nn::vfx::CallbackSet emitterCallbackSet;
    emitterCallbackSet.emitterInitialize = customActionEmitterEmit;
    emitterCallbackSet.emitterPostCalculate = customActionEmitterPostCalc;
    emitterCallbackSet.emitterFinalize = customActionEmitterRemove;
    mPtclSystem->SetCallback(2, emitterCallbackSet);

    nn::vfx::CallbackSet particleCallbackSet;
    particleCallbackSet.particleEmit = customActionPerticleEmit;
    particleCallbackSet.particleRemove = customActionPerticleRemove;
    particleCallbackSet.particleCalculate = customActionPerticleCalc;
    mPtclSystem->SetCallback(3, particleCallbackSet);

    nn::vfx::CallbackSet rippleCallbackSet;
    rippleCallbackSet.particleCalculate = customActionRipplePerticle;
    mPtclSystem->SetCallback(4, rippleCallbackSet);

    nn::vfx::CallbackSet uvCallbackSet;
    uvCallbackSet.particleEmit = customActionGenerateUv;
    mPtclSystem->SetCallback(5, uvCallbackSet);

    nn::vfx::CallbackSet shaderCallbackSet;
    shaderCallbackSet.renderStateSet = customShaderCommon;
    mPtclSystem->SetCallback(13, shaderCallbackSet);
    mPtclSystem->SetCallback(15, shaderCallbackSet);

    mPtclSystem->SetCustomFieldCallback(customFieldCpu);
    mPtclSystem->SetDrawPathRenderStateSetCallback(nn::vfx::DrawPathCallbackId_0,
                                                   static_cast<nn::vfx::DrawPathFlag>(0x18e1),
                                                   DrawPathRenderStateSetCallback);
    mPtclSystem->SetDrawPathRenderStateSetCallback(nn::vfx::DrawPathCallbackId_1,
                                                   static_cast<nn::vfx::DrawPathFlag>(0x300),
                                                   renderStateReduceBuffer);
}

/**
 * Swaps the buffers of the scene and dither uniform blocks.
 */
void EffectShaderHolderNew::swapUbo() {
    mUbo->swap();
    mDitherUbo->swap();
}

/**
 * Updates the wind and repulsion parameters used by compute shader emitters.
 */
void EffectShaderHolderNew::updateShaderParamForCompute() {
    if (!EffectSystem::isEnableBatchCompute()) {
        return;
    }

    mUbo->setData(4, &mEnvParam->mWindDir, 0, 1);

    if (mEnvParam->mIsEnableRepulsion) {
        mUbo->setData(5, mEnvParam->mRepulsionPos, 0, 2);
        mUbo->setValue(6, mEnvParam->mRepulsionPosNum);
    } else {
        mUbo->setData(5, &sead::Vector3f::zero, 0, 1);
        mUbo->setValue(6, 0);
    }

    flushUbo(mUbo);
}

/**
 * Updates the scene uniform block from the environment, fog and view settings.
 * @param rViewMtx the view matrix
 * @param rScreenSize the screen size
 */
void EffectShaderHolderNew::updateShaderParam(const sead::Matrix34f& rViewMtx,
                                              const sead::Vector2f& rScreenSize) {
    mViewMtx = rViewMtx;
    mUbo->setData(4, &mEnvParam->mWindDir, 0, 1);

    if (mEnvParam->mIsEnableRepulsion) {
        mUbo->setData(5, mEnvParam->mRepulsionPos, 0, 2);
        mUbo->setValue(6, mEnvParam->mRepulsionPosNum);
    } else {
        mUbo->setData(5, &sead::Vector3f::zero, 0, 1);
        mUbo->setValue(6, 0);
    }

    FogDirector* fogDirector = mGraphicsSystemInfo->getFogDirector();
    const FogParam& fogParam = fogDirector->getFogParam();
    mUbo->setValue(7, static_cast<s32>(*fogParam.mIntensityMax > 0.0f));
    fogParam.getStart();
    mUbo->setValue(10, fogParam.getEnd());
    const YFogParam& yFogParam = fogDirector->getYFogParam();
    mUbo->setValue(11, static_cast<s32>(*yFogParam.mIntensityMax > 0.0f));
    yFogParam.getStart();
    mUbo->setValue(14, yFogParam.getEnd());
    setUboValue(mUbo, 15, sead::Vector4f(0.0f, 0.0f, 0.0f, 0.0f));
    setUboValue(mUbo, 16, sead::Vector4f(0.0f, 0.0f, 0.0f, 0.0f));
    EffectShaderHolder::updateShaderParam(rViewMtx, rScreenSize);
}

/**
 * Binds the scene uniform block to the custom compute shader slot.
 * @param pDrawContext the draw context
 */
void EffectShaderHolderNew::bindCustomShaderUboForCompute(agl::DrawContext* pDrawContext) {
    agl::ShaderLocation location;
    location.setLocation(agl::cShaderType_Compute, 12);
    mUbo->activate(pDrawContext, location);
}

/**
 * Binds the scene uniform block to the custom shader slot.
 * @param pDrawContext the draw context
 */
void EffectShaderHolderNew::bindCustomShaderUbo(agl::DrawContext* pDrawContext) {
    EffectShaderHolder::bindCustomShaderUbo(pDrawContext);
}

/**
 * Requests the point lights of all effect lights.
 */
void EffectShaderHolderNew::updateLight() {
    if (mGraphicsSystemInfo != nullptr) {
        mLightDirector->update(mGraphicsSystemInfo->getPrePassLightKeeper());
    }
}

/**
 * Sets the texture of the first custom texture slot.
 * @param pTexture the texture, or nullptr for a white texture
 */
void EffectShaderHolderNew::setupTextureProg0(const agl::TextureData* pTexture) {
    if (pTexture != nullptr) {
        mProg0Sampler.applyTextureData(*pTexture);
        return;
    }

    agl::TextureSampler sampler = *agl::utl::PrimitiveTexture::instance()->getTextureSampler(
        agl::utl::PrimitiveTexture::cType_White2D);
    mProg0Sampler.applyTextureData(sampler.getTextureData());
}

/**
 * Gets the irradiance cube map used as material light texture.
 * @param index the material light index (unused)
 * @return the irradiance sampler
 */
NOINLINE WEAK DISABLE_TAIL_CALLS const agl::TextureSampler*
EffectShaderHolderNew::getTextureMaterialLight(s32 index) const {
    return mGraphicsSystemInfo->getCubeMapDirector()->getIrradianceSampler(0);
}

/**
 * Gets a noise texture; there is none in this game.
 * @param index the noise texture index (unused)
 * @return nullptr
 */
NOINLINE WEAK const agl::TextureSampler*
EffectShaderHolderNew::getTextureNoise(s32 index) const {
    return nullptr;
}

/**
 * Sets the render state of deferred effects, masking the G-buffer color writes.
 * @param rArg the render state argument
 */
void EffectShaderHolderNew::renderDeferred(nn::vfx::RenderStateSetArg& rArg) const {
    const nn::vfx::detail::ResEmitter* resEmitter = rArg.pEmitter->m_pEmitterRes->m_pResEmitter;

    if ((resEmitter->drawPath | 2) != 7) {
        EffectShaderHolder::renderDeferred(rArg);
        return;
    }

    if (mIsDrawReduceBuffer) {
        sead::GraphicsContext context;
        context.setColorMask(0u);
        context.applyColorMask(mDrawContext);
        return;
    }

    sead::GraphicsContext context;
    setContextMRTAlphaMask(static_cast<sead::GraphicsContextMRT*>(&context));

    if (!rArg.pEmitter->m_pEmitterRes->m_pResEmitter->isAlphaMaskEnable) {
        context.setColorMask(2, false, false, false, false);
    }

    context.setColorMask(4, false, false, false, false);
    context.applyColorMask(mDrawContext);
}

/**
 * Sets the light parameters.
 * @param rPos the light position
 * @param radius the light radius
 * @param rColor the light color
 * @param isEnableSpecular whether the light has a specular term
 */
NOINLINE void EffectLight::set(const sead::Vector3f& rPos, f32 radius,
                                                const sead::Color4f& rColor,
                                                bool isEnableSpecular) {
    mPos = rPos;
    mRadius = radius;
    mColor = rColor;
    mIsEnableSpecular = isEnableSpecular;
}

/**
 * Allocates the pool of 64 effect lights.
 */
EffectLightDirector::EffectLightDirector() {
    mLightList.allocBuffer(64, nullptr);
}

/**
 * Takes a light from the pool.
 * @return the light, or nullptr if the pool is exhausted
 */
NOINLINE EffectLight* EffectLightDirector::tryCreateLight() {
    return mLightList.emplaceBack();
}

/**
 * Returns a light to the pool.
 * @param pLight the light
 */
NOINLINE void EffectLightDirector::removeLight(EffectLight* pLight) {
    mLightList.erase(pLight);
}

/**
 * Requests a point light for every light in use.
 * @param pKeeper the light pre-pass keeper
 */
NOINLINE void EffectLightDirector::update(PrePassLightKeeper* pKeeper) {
    for (EffectLight& light : mLightList) {
        pKeeper->requestPointLight(light.getPos(), light.getRadius(), light.getColor(), 0.95f, 0.0f,
                                   light.isEnableSpecular(), false, sead::Color4f::cWhite, 0);
    }
}

/**
 * Binds the scene uniform block, the lens flare parameters and the custom shader textures.
 * @param rArg the render state argument
 * @return true
 */
bool EffectShaderHolderNew::customShaderCommon(nn::vfx::RenderStateSetArg& rArg) {
    nn::vfx::BindReservedCustomShaderConstantBuffer(rArg);
    nn::vfx::Emitter* emitter = rArg.pEmitter;
    nn::vfx::System* system = emitter->m_EmitterSet->m_System;
    const nn::vfx::detail::ResEmitter* resEmitter = emitter->m_pEmitterRes->m_pResEmitter;
    EffectShaderHolderNew* holder = static_cast<EffectShaderHolderNew*>(
        static_cast<PtclSystem*>(system)->getEffectSystem()->getShaderHolder());
    u64 flag = resEmitter->customShaderFlag;

    if (flag & 0x1080000) {
        const agl::TextureSampler* texture;

        if ((resEmitter->customShaderSwitch >> 16) & 0xe) {
            texture = holder->getTextureNoise(1);
        } else {
            ScreenRect* rect = reinterpret_cast<ScreenRect*>(holder->mLensFlareRect);

            if (emitter->m_Frame <= 1.0f) {
                const EffectCameraHolder* cameraHolder =
                    static_cast<PtclSystem*>(system)->getEffectSystem()->getEffectCameraHolder();
                const nn::vfx::detail::ResEmitter* data = emitter->m_pEmitterData;
                const sead::Matrix34f& viewMtx = holder->mViewMtx;
                f32 scaleX = data->particleScaleX;
                f32 scaleY = data->particleScaleY;
                sead::Vector3f axisX(viewMtx.m[0][0], viewMtx.m[0][1], viewMtx.m[0][2]);
                sead::Vector3f axisY(viewMtx.m[1][0], viewMtx.m[1][1], viewMtx.m[1][2]);
                f32 sizeX = scaleX * 16.0f;
                f32 sizeY = scaleY * 16.0f;
                sead::Vector3f halfSize = axisX * scaleX * 0.5f + axisY * scaleY * 0.5f;
                sead::Vector3f right = axisX * (sizeX * 0.5f);
                sead::Vector3f up = axisY * (sizeY * 0.5f);
                sead::Vector3f pos(vgetq_lane_f32(emitter->m_MatrixSrt._m.val[3], 0),
                                   vgetq_lane_f32(emitter->m_MatrixSrt._m.val[3], 1),
                                   vgetq_lane_f32(emitter->m_MatrixSrt._m.val[3], 2));
                sead::Vector3f center = pos - halfSize;
                sead::Vector3f top = up + center;
                sead::Vector2f screenPosA;
                sead::Vector2f screenPosB;
                cameraHolder->calcScreenPosFromWorldPos(&screenPosA, top - right);
                cameraHolder->calcScreenPosFromWorldPos(&screenPosB, right + top);
                sead::Vector2f leftTop = screenPosA;
                sead::Vector2f rightTop = screenPosB;
                sead::Vector3f bottom = center - up;
                cameraHolder->calcScreenPosFromWorldPos(&screenPosA, bottom - right);
                cameraHolder->calcScreenPosFromWorldPos(&screenPosB, right + bottom);
                sead::Vector2f leftBottom = screenPosA;
                sead::Vector2f rightBottom = screenPosB;
                f32 leftTopX = leftTop.x / static_cast<u32>(getDisplayWidth());
                f32 leftTopY = leftTop.y / static_cast<u32>(getDisplayHeight());
                f32 rightTopX = rightTop.x / static_cast<u32>(getDisplayWidth());
                f32 rightTopY = rightTop.y / static_cast<u32>(getDisplayHeight());
                f32 leftBottomX = leftBottom.x / static_cast<u32>(getDisplayWidth());
                f32 leftBottomY = leftBottom.y / static_cast<u32>(getDisplayHeight());
                f32 rightBottomX = rightBottom.x / static_cast<u32>(getDisplayWidth());
                f32 rightBottomY = rightBottom.y / static_cast<u32>(getDisplayHeight());
                rect->values[0] = leftTopX;
                rect->values[1] = leftTopY;
                rect->values[2] = rightTopX;
                rect->values[3] = rightTopY;
                rect->values[4] = leftBottomX;
                rect->values[5] = leftBottomY;
                rect->values[6] = rightBottomX;
                rect->values[7] = rightBottomY;
            }

            texture = &holder->mProg0Sampler;
            holder->mUbo->setData(15, &rect->values[0], 0, 1);
            holder->mUbo->setData(16, &rect->values[4], 0, 1);
            flushUbo(holder->mUbo);
            agl::ShaderLocation location;
            location.setLocation(agl::cShaderType_Vertex, 12);
            location.setLocation(agl::cShaderType_Fragment, 12);
            holder->mUbo->activate(holder->mDrawContext, location);
        }

        if (texture != nullptr) {
            nn::gfx::DescriptorSlot samplerSlot;
            nn::gfx::DescriptorSlot textureSlot;
            texture->updateRegs();
            u32 samplerId = texture->getSampler().getSamplerID();
            samplerSlot.ToData()->value = samplerId;
            textureSlot.ToData()->value = getTextureId(texture->getTextureData());
            nn::vfx::detail::Shader* shader = emitter->m_pEmitterRes->m_Shader[0];
            s32 index = nn::vfx::CustomShaderTextureType_28 + ((flag >> 24) & 1);
            s32 pixelLocation = shader->GetCustomTexturePixelLocationByIndex(index);
            s32 vertexLocation = shader->GetCustomTextureVertexLocationByIndex(index);

            if (vertexLocation != -1) {
                rArg.pCommandBuffer->SetTextureAndSampler(
                    vertexLocation, nn::gfx::ShaderStage_Vertex, textureSlot, samplerSlot);
            }

            if (pixelLocation != -1) {
                rArg.pCommandBuffer->SetTextureAndSampler(pixelLocation, nn::gfx::ShaderStage_Pixel,
                                                          textureSlot, samplerSlot);
            }
        }
    }

    const s32* customData = emitter->m_pEmitterRes->m_CustomDataParam;
    const agl::TextureSampler* lightTexture =
        holder->getTextureMaterialLight(customData != nullptr ? *customData : 0);
    bindCustomShaderTexture(system, holder->mDrawContext, nn::vfx::CustomShaderTextureType_12,
                            lightTexture);
    bindCustomShaderTexture(system, holder->mDrawContext, nn::vfx::CustomShaderTextureType_0,
                            &holder->mExposureSampler);
    return true;
}

/**
 * Sets the blend state of effects drawn to the reduced buffer.
 * @param rArg the render state argument
 */
void EffectShaderHolderNew::renderStateReduceBuffer(nn::vfx::RenderStateSetArg& rArg) {
    EffectShaderHolderNew* holder = getShaderHolder(rArg.pEmitter);
    sead::GraphicsContext context;
    context.setBlendFactorSrcRGB(0, 5);
    context.setBlendFactorSrcA(0, 1);
    context.setBlendFactorDstRGB(0, 6);
    context.setBlendFactorDstA(0, 6);
    context.applyBlendAndFastZ(holder->mDrawContext);
}

/**
 * Updates the effect light of an emitter.
 * @param rArg the emitter post calculation argument
 */
void EffectShaderHolderNew::customActionEmitterPostCalc(nn::vfx::EmitterPostCalculateArg& rArg) {
    nn::vfx::Emitter* emitter = rArg.pEmitter;

    if (!emitter->m_IsCalculated) {
        return;
    }

    EffectLight* light = static_cast<EffectLight*>(emitter->m_UserData);

    if (light == nullptr) {
        return;
    }

    const CustomActionDataPointLight* data =
        static_cast<const CustomActionDataPointLight*>(emitter->m_pEmitterRes->m_CustomActionParam);
    sead::Vector3f pos;
    vst1_f32(&pos.x, vget_low_f32(emitter->m_EmitterLocalPos._v));
    vst1q_lane_f32(&pos.z, emitter->m_EmitterLocalPos._v, 2);
    f32 scale = calcEmitterScale(emitter);
    const nn::vfx::detail::ResEmitter* resEmitter = emitter->m_pEmitterData;
    f32 life = resEmitter->isLoop ? resEmitter->particleLife + resEmitter->emitEndFrame : 0.0f;
    setEffectLight(light, pos, data, scale, emitter->m_Frame, life);
}

/**
 * Creates the effect light of an emitter.
 * @param rArg the emitter initialization argument
 * @return true
 */
bool EffectShaderHolderNew::customActionEmitterEmit(nn::vfx::EmitterInitializeArg& rArg) {
    nn::vfx::Emitter* emitter = rArg.pEmitter;
    EffectShaderHolderNew* holder = getShaderHolder(emitter);
    emitter->m_UserData = holder != nullptr ? holder->mLightDirector->tryCreateLight() : nullptr;
    return true;
}

/**
 * Releases the effect light of an emitter.
 * @param rArg the emitter finalization argument
 */
void EffectShaderHolderNew::customActionEmitterRemove(nn::vfx::EmitterFinalizeArg& rArg) {
    nn::vfx::Emitter* emitter = rArg.pEmitter;
    EffectLight* light = static_cast<EffectLight*>(emitter->m_UserData);

    if (light != nullptr) {
        getShaderHolder(emitter)->mLightDirector->removeLight(light);
    }
}

/**
 * Updates the effect light of a particle.
 * @param rArg the particle calculation argument
 */
void EffectShaderHolderNew::customActionPerticleCalc(nn::vfx::ParticleCalculateArgImpl& rArg) {
    EffectLight* light = static_cast<EffectLight*>(rArg.pUserData);

    if (light == nullptr) {
        return;
    }

    nn::vfx::Emitter* emitter = rArg.pEmitter;
    s32 index = rArg.particleIndex;
    nn::util::Vector3fType localPos;
    nn::util::VectorLoad(&localPos,
                         reinterpret_cast<const nn::util::Float3&>(emitter->m_ParticlePos[index]));
    const CustomActionDataPointLight* data =
        static_cast<const CustomActionDataPointLight*>(emitter->m_pEmitterRes->m_CustomActionParam);
    nn::util::Vector3fType pos;
    calcParticleWorldPos(&pos, emitter, index, localPos);
    setEffectLight(light, reinterpret_cast<const sead::Vector3f&>(pos), data,
                   calcEmitterScale(emitter), rArg.time, rArg.life);
}

/**
 * Creates the effect light of a particle.
 * @param rArg the particle emission argument
 * @return true
 */
bool EffectShaderHolderNew::customActionPerticleEmit(nn::vfx::ParticleCalculateArgImpl& rArg) {
    EffectShaderHolderNew* holder = getShaderHolder(rArg.pEmitter);
    rArg.pUserData = holder != nullptr ? holder->mLightDirector->tryCreateLight() : nullptr;
    return true;
}

/**
 * Releases the effect light of a particle.
 * @param rArg the particle removal argument
 * @return true
 */
bool EffectShaderHolderNew::customActionPerticleRemove(nn::vfx::ParticleCalculateArgImpl& rArg) {
    EffectLight* light = static_cast<EffectLight*>(rArg.pUserData);

    if (light != nullptr) {
        getShaderHolder(rArg.pEmitter)->mLightDirector->removeLight(light);
    }

    return true;
}

/**
 * Does nothing.
 * @param rArg the particle calculation argument
 */
void EffectShaderHolderNew::customActionRipplePerticle(nn::vfx::ParticleCalculateArgImpl& rArg) {}

/**
 * Offsets the position of a new particle on the view plane to its cell of a 16x16 grid.
 * @param rArg the particle emission argument
 * @return true
 */
bool EffectShaderHolderNew::customActionGenerateUv(nn::vfx::ParticleCalculateArgImpl& rArg) {
    nn::vfx::Emitter* emitter = rArg.pEmitter;
    float32x2_t random = vld1_f32(emitter->m_ParticleRandom[rArg.particleIndex].v);
    u32 cell = emitter->m_ParticleNum;
    const sead::Matrix34f& viewMtx = getShaderHolder(emitter)->mViewMtx;
    f32 offsetX = ((s32)((cell >> 4) & 0xf) + -8.0f) * vget_lane_f32(random, 0);
    sead::Vector3f offset(viewMtx.m[0][0] * offsetX, offsetX * viewMtx.m[0][1],
                          offsetX * viewMtx.m[0][2]);
    f32 offsetY = ((s32)(cell & 0xf) + -8.0f) * vget_lane_f32(random, 1);
    f32 x = offset.x + offsetY * viewMtx.m[1][0];
    f32 y = offset.y + offsetY * viewMtx.m[1][1];
    f32 z = offset.z + offsetY * viewMtx.m[1][2];
    rArg.pEmitter->m_ParticlePos[rArg.particleIndex].x = x;
    rArg.pEmitter->m_ParticlePos[rArg.particleIndex].y = y;
    rArg.pEmitter->m_ParticlePos[rArg.particleIndex].z = z;
    return true;
}

/**
 * Applies the wind and the repulsion of the environment to a particle.
 * @param pPos the particle position
 * @param pVel the particle velocity
 * @param pTime the particle time
 * @param pLife the particle life
 * @param pEmitter the emitter
 * @param pProperty the particle property
 * @param pField the custom field resource
 * @param particleIndex the particle index
 * @return true
 */
bool EffectShaderHolderNew::customFieldCpu(nn::util::neon::Vector3fType* pPos,
                                           nn::util::neon::Vector3fType* pVel, f32* pTime,
                                           f32* pLife, nn::vfx::Emitter* pEmitter,
                                           const nn::vfx::detail::ParticleProperty* pProperty,
                                           const nn::vfx::detail::ResFieldCustom* pField,
                                           s32 particleIndex) {
    const EffectEnvParam* envParam =
        static_cast<PtclSystem*>(pEmitter->m_EmitterSet->m_System)->getEffectEnvParam();
    const CustomFieldParam* field = reinterpret_cast<const CustomFieldParam*>(pField);

    if (field->flags & 1) {
        f32 windScale = field->windScale;
        float32x4_t wind =
            makeVector(windScale * envParam->mWindDir.x, windScale * envParam->mWindDir.y,
                       windScale * envParam->mWindDir.z);
        pPos->_v = vaddq_f32(pPos->_v, wind);
    }

    if (!(field->flags & 6) || !envParam->mIsEnableRepulsion || envParam->mRepulsionPosNum < 1) {
        return true;
    }

    sead::Vector3f repulsion(0.0f, 0.0f, 0.0f);
    s32 repulsionNum = 0;
    float32x4x4_t mtx;

    for (s32 i = 0; i < envParam->mRepulsionPosNum; i++) {
        const sead::Vector3f& repulsionPos = envParam->mRepulsionPos[i];

        if (pEmitter->m_pEmitterData->followType == 1) {
            if (pEmitter->m_ParticleEmitterMatrixRow[0] != nullptr &&
                pEmitter->m_ParticleNum >= particleIndex) {
                const nn::util::Float4& row0 =
                    pEmitter->m_ParticleEmitterMatrixRow[0][particleIndex];
                const nn::util::Float4& row1 =
                    pEmitter->m_ParticleEmitterMatrixRow[1][particleIndex];
                const nn::util::Float4& row2 =
                    pEmitter->m_ParticleEmitterMatrixRow[2][particleIndex];
                mtx.val[0] = makeVector(row0.x, row1.x, row2.x);
                mtx.val[1] = makeVector(row0.y, row1.y, row2.y);
                mtx.val[2] = makeVector(row0.z, row1.z, row2.z);
                mtx.val[3] = makeVector(row0.w, row1.w, row2.w);
            }
        } else {
            mtx = pEmitter->m_MatrixSrt._m;
        }

        sead::Vector3f localPos;
        calcInverseTransform(&localPos, mtx, repulsionPos);
        f32 diffX = vgetq_lane_f32(pPos->_v, 0) - localPos.x;
        f32 diffY = vgetq_lane_f32(pPos->_v, 1) - localPos.y;
        f32 diffZ = vgetq_lane_f32(pPos->_v, 2) - localPos.z;
        f32 distanceSq = diffX * diffX + diffY * diffY + diffZ * diffZ;
        f32 repulsionY = field->flags & 4 ? 0.0f : diffY;

        if (!isNearZero(distanceSq, 0.001f) && distanceSq < field->range * field->range) {
            f32 distance = nerd::sqrt(distanceSq);
            f32 rate = field->power * (1.0f - distance / field->range) / distance;
            repulsion.x += diffX * rate;
            repulsion.y += repulsionY * rate;
            repulsion.z += diffZ * rate;
            repulsionNum++;
        }
    }

    if (repulsionNum > 0) {
        pPos->_v = vaddq_f32(makeVector(repulsion.x, repulsion.y, repulsion.z), pPos->_v);
    }

    return true;
}

}  // namespace al
