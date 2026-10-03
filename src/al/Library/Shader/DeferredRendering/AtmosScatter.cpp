#include "Library/Shader/DeferredRendering/AtmosScatter.hpp"

#include <arm_neon.h>
#include <common/aglDrawContext.h>
#include <common/aglRenderBuffer.h>
#include <common/aglShaderLocation.h>
#include <common/aglShaderProgram.h>
#include <common/aglTextureSampler.h>
#include <gfx/seadGraphicsContext.h>
#include <gfx/seadGraphicsContextMRT.h>
#include <gfx/seadViewport.h>
#include <math/seadMathCalcCommon.h>
#include <math/seadQuat.h>
#include <utility/aglDynamicTextureAllocator.h>
#include <utility/aglPrimitiveTexture.h>

#include "Library/Debug/Render/RenderBufferAttacher.hpp"
#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorSceneInfo.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Shader/Block/UniformBlock.hpp"
#include "Library/Shader/Block/UniformBlockUtil.hpp"
#include "Library/Shader/DeferredRendering/FullScreenQuadModel.hpp"
#include "Library/Shader/DeferredRendering/GBufferArray.hpp"
#include "Library/Shader/DeferredRendering/SamplerLocation.hpp"
#include "Library/Shader/ForwardRendering/ShaderHolder.hpp"
#include "Library/Shader/ForwardRendering/ShaderSamplerSetter.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Draw/GraphicsParamKeeper.hpp"

namespace {
const al::UniformBlockLayout cAtmosScatterUboLayout[] = {
    {0, agl::UniformBlock::cType_Vec3, 1},  // Camera position
    {1, agl::UniformBlock::cType_Vec4, 1},  // Sun direction and intensity
    {2, agl::UniformBlock::cType_Vec4, 1},  // Light direction and scaled intensity
    {3, agl::UniformBlock::cType_Vec4, 3},  // Inverse view projection matrix
    {4, agl::UniformBlock::cType_Vec2, 1},  // Tangent of the half field of view
    {5, agl::UniformBlock::cType_Vec2, 1},  // Cosines of the sun size
};

/// Radius of the ground in kilometers.
constexpr f32 cRgKm = 6360.0f;
/// Radius of the top of the atmosphere in kilometers.
constexpr f32 cRtKm = 6420.0f;
/// Minimum distance of the camera from the center of the earth in kilometers.
constexpr f32 cCameraRadiusMinKm = cRgKm + 0.1f;
/// Number of layers of the 3D textures.
constexpr s32 cLayerNum = 32;
/// Number of scattering orders that are precomputed.
constexpr s32 cScatteringOrderNum = 4;

/**
 * Sets a scalar value to a uniform if it was found.
 * @param pDrawContext Draw context.
 * @param rLocation Location of the uniform.
 * @param value Value to set.
 */
template <typename T>
inline void setUniformValue(agl::DrawContext* pDrawContext, const agl::UniformLocation& rLocation,
                            T value) {
    if (rLocation.isValid()) {
        rLocation.setUniformNVN(pDrawContext, 1, &value);
    }
}

/**
 * Searches a uniform by name and sets a scalar value to it.
 * @param pDrawContext Draw context.
 * @param pProgram Shader program to search the uniform in.
 * @param pName Uniform name.
 * @param rValue Value to set.
 */
template <typename T>
inline void setUniformValue(agl::DrawContext* pDrawContext, const agl::ShaderProgram* pProgram,
                            const char* pName, const T& rValue) {
    agl::UniformLocation location(pName);
    location.search(*pProgram);
    setUniformValue(pDrawContext, location, rValue);
}

/**
 * Searches a uniform by name and sets an array of floats to it.
 * @param pDrawContext Draw context.
 * @param pProgram Shader program to search the uniform in.
 * @param pName Uniform name.
 * @param num Number of floats.
 * @param pData Data to set.
 */
inline void setUniformArray(agl::DrawContext* pDrawContext, const agl::ShaderProgram* pProgram,
                            const char* pName, u32 num, const void* pData) {
    agl::UniformLocation location(pName);
    location.search(*pProgram);
    location.setUniform(pDrawContext, num, pData);
}

/**
 * Copies a value and sets it to a member of a uniform block.
 * @param pBlock Uniform block.
 * @param memberIndex Index of the member.
 * @param rValue Value to set.
 */
template <typename T>
inline void setUniformBlockValue(al::UniformBlock* pBlock, s32 memberIndex, const T& rValue) {
    T value = rValue;
    pBlock->setData(memberIndex, &value, 0, 1);
}

/**
 * Multiplies a 3x4 matrix with a 4x4 matrix.
 * @param pOut Resulting 3x4 matrix.
 * @param rA Left matrix.
 * @param rB Right matrix.
 */
inline void multiplyMtx34Mtx44(sead::Matrix34f* pOut, const sead::Matrix34f& rA,
                               const sead::Matrix44f& rB) {
    float32x4_t a0 = vld1q_f32(rA.m[0]);
    float32x4_t a1 = vld1q_f32(rA.m[1]);
    float32x4_t a2 = vld1q_f32(rA.m[2]);

    float32x4_t b0 = vld1q_f32(rB.m[0]);
    float32x4_t b1 = vld1q_f32(rB.m[1]);
    float32x4_t b2 = vld1q_f32(rB.m[2]);
    float32x4_t b3 = vld1q_f32(rB.m[3]);

    float32x4_t c0 = vmulq_laneq_f32(b0, a0, 0);
    c0 = vfmaq_laneq_f32(c0, b1, a0, 1);
    c0 = vfmaq_laneq_f32(c0, b2, a0, 2);
    c0 = vfmaq_laneq_f32(c0, b3, a0, 3);

    float32x4_t c1 = vmulq_laneq_f32(b0, a1, 0);
    c1 = vfmaq_laneq_f32(c1, b1, a1, 1);
    c1 = vfmaq_laneq_f32(c1, b2, a1, 2);
    c1 = vfmaq_laneq_f32(c1, b3, a1, 3);

    float32x4_t c2 = vmulq_laneq_f32(b0, a2, 0);
    c2 = vfmaq_laneq_f32(c2, b1, a2, 1);
    c2 = vfmaq_laneq_f32(c2, b2, a2, 2);
    c2 = vfmaq_laneq_f32(c2, b3, a2, 3);

    vst1q_f32(pOut->m[0], c0);
    vst1q_f32(pOut->m[1], c1);
    vst1q_f32(pOut->m[2], c2);
}

/**
 * Sets the uniforms describing the altitude of a layer of the 3D scattering textures.
 * @param pProgram Shader program to set the uniforms to.
 * @param layer Layer index.
 */
void setLayer(const agl::ShaderProgram* pProgram, s32 layer) {
    f32 r = layer / (cLayerNum - 1.0f);
    r = sead::Mathf::sqrt(cRgKm * cRgKm + r * r * (cRtKm * cRtKm - cRgKm * cRgKm));
    r += layer == 0 ? 0.01 : (layer == cLayerNum - 1 ? -0.001 : 0.0);

    f32 dMin = cRtKm - r;
    f32 dMax = sead::Mathf::sqrt(r * r - cRgKm * cRgKm) +
               sead::Mathf::sqrt(cRtKm * cRtKm - cRgKm * cRgKm);
    f32 dMinP = r - cRgKm;
    f32 dMaxP = sead::Mathf::sqrt(r * r - cRgKm * cRgKm);
    sead::Vector4f dhdH(dMin, dMax, dMinP, dMaxP);

    setUniformValue(al::GameFrameworkNx::getAglDrawContext(), pProgram, "r", r);
    setUniformArray(al::GameFrameworkNx::getAglDrawContext(), pProgram, "dhdH", 4, &dhdH);
    setUniformValue(al::GameFrameworkNx::getAglDrawContext(), pProgram, "layer", layer);
}
}  // namespace

namespace al {

/**
 * Gets the shaders, creates the precomputed textures and the per view uniform blocks, and
 * initializes the parameters.
 * @param pInfo Graphics system info.
 * @param viewNum Number of views to draw.
 * @param sunIntensity Intensity of the sun.
 */
AtmosScatter::AtmosScatter(GraphicsSystemInfo* pInfo, s32 viewNum, f32 sunIntensity)
    : mGraphicsSystemInfo(pInfo),
      mRenderEarthProgram(ShaderHolder::sInstance->getShaderProgram("PasRenderEarth")),
      mRenderDirLightColorProgram(
          ShaderHolder::sInstance->getShaderProgram("PasRenderDirLightColor")),
      mTransmittanceProgram(ShaderHolder::sInstance->getShaderProgram("PasTransmittance")),
      mIrradiance1Program(ShaderHolder::sInstance->getShaderProgram("PasIrradiance1")),
      mInscatter1Program(ShaderHolder::sInstance->getShaderProgram("PasInscatter1")),
      mCopyInscatter1Program(ShaderHolder::sInstance->getShaderProgram("PasCopyInscatter1")),
      mCopyInscatterNProgram(ShaderHolder::sInstance->getShaderProgram("PasCopyInscatterN")),
      mCopyIrradianceProgram(ShaderHolder::sInstance->getShaderProgram("PasCopyIrradiance")),
      mInscatterSProgram(ShaderHolder::sInstance->getShaderProgram("PasInscatterS")),
      mIrradianceNProgram(ShaderHolder::sInstance->getShaderProgram("PasIrradianceN")),
      mInscatterNProgram(ShaderHolder::sInstance->getShaderProgram("PasInscatterN")),
      mQuadModel(new FullScreenQuadModel()), mRenderBuffer(new agl::RenderBuffer()),
      mSunIntensity(sunIntensity),
      mParamIo(new GraphicsParamIo("AtmosScatter", "aglatmos", nullptr)) {
    createTextures();

    mViewUbos.allocBuffer(viewNum, nullptr);

    for (s32 i = 0; i < viewNum; i++) {
        ViewUbo* viewUbo = new ViewUbo();
        viewUbo->mUniformBlock = createUniformBlock(cAtmosScatterUboLayout, 6, nullptr, 2);
        mViewUbos.pushBack(viewUbo);
    }

    mSunYRotDegree.init(0.0f, "SunYRotDegree",
                        "太陽の軌道の方向（Ｙ軸角度で指定）", "Min=0, Max=360",
                        &mParamObj);
    mSunZRotDegree.init(0.0f, "SunZRotDegree",
                        "太陽の軌道の傾き（Ｚ軸角度で指定）", "Min=0, Max=360",
                        &mParamObj);
    mSunXRotDegreeInit.init(0.0f, "SunXRotDegreeInit",
                            "太陽の初期角度（Ｘ軸角度で指定）", "Min=0, Max=360",
                            &mParamObj);
    mSunXRotSpeedDegree.init(0.001f, "SunXRotSpeedDegree", "太陽の動く角速度",
                             "Min=0, Max=360", &mParamObj);
    mCameraOffsetParam.init(sead::Vector3f::zero, "CameraOffset",
                            "カメラの地球からのオフセット位置初期値(Km)",
                            "Min=-100.0f, Max=100.0f", &mParamObj);

    mParamIo->getParamIo()->addObj(&mParamObj,
                                   StringTmp<128>("CommonParam%s", "AtmosScatter").cstr());
}

/**
 * Allocates the precomputed textures and creates their samplers.
 */
void AtmosScatter::createTextures() {
    DynamicTexAlloc alloc(true);

    mTransmittanceTex = alloc.getAlloc()->alloc(
        GameFrameworkNx::getAglDrawContext(), "PasTransmittance",
        agl::TextureFormat::cTextureFormat_R11_G11_B10_float, 256, 64, 1, nullptr,
        agl::utl::DynamicTextureAllocator::cAllocateType_1, true, false);
    mIrradianceTex = alloc.getAlloc()->alloc(
        GameFrameworkNx::getAglDrawContext(), "PasIrradiance",
        agl::TextureFormat::cTextureFormat_R11_G11_B10_float, 64, 16, 1, nullptr,
        agl::utl::DynamicTextureAllocator::cAllocateType_1, true, false);
    mInscatterTex = alloc.getAlloc()->alloc3D(
        GameFrameworkNx::getAglDrawContext(), "PasDeltaSM3D",
        agl::TextureFormat::cTextureFormat_R16_G16_B16_A16_float, 256, 128, cLayerNum, 1, nullptr,
        agl::utl::DynamicTextureAllocator::cAllocateType_1, true, false);

    mTransmittanceSampler = new agl::TextureSampler(*mTransmittanceTex);
    mIrradianceSampler = new agl::TextureSampler(*mIrradianceTex);
    mInscatterSampler = new agl::TextureSampler(*mInscatterTex);
    mDeltaESampler = new agl::TextureSampler();
    mDeltaSRSampler = new agl::TextureSampler();
    mDeltaSMSampler = new agl::TextureSampler();
    mDeltaJSampler = new agl::TextureSampler();
}

/**
 * Frees the precomputed textures.
 */
AtmosScatter::~AtmosScatter() {
    DynamicTexAlloc alloc(false);

    if (mTransmittanceTex != nullptr) {
        alloc.freeTex(mTransmittanceTex);
    }

    if (mIrradianceTex != nullptr) {
        alloc.freeTex(mIrradianceTex);
    }

    if (mInscatterTex != nullptr) {
        alloc.freeTex(mInscatterTex);
    }
}

/**
 * Checks whether the scattering textures have to be precomputed this frame.
 * @return Whether the precomputation is drawn.
 */
bool AtmosScatter::isPrecompute() const {
    if (mIsPreDrawn && mIsPrecompute) {
        return true;
    }

    return false;
}

/**
 * Gets the radius of the ground.
 * @return Radius in kilometers.
 */
f32 AtmosScatter::getRgKm() const {
    return cRgKm;
}

/**
 * Gets the radius of the top of the atmosphere.
 * @return Radius in kilometers.
 */
f32 AtmosScatter::getRtKm() const {
    return cRtKm;
}

/**
 * Applies the stage parameter file and resets the sun and the camera.
 * @param pResource Stage resource.
 * @param pStageName Name of the stage.
 */
void AtmosScatter::initStageResource(const Resource* pResource, const char* pStageName) {
    mParamIo->initStageResource(pResource, pStageName);
    mSunXRot = sead::Mathf::deg2rad(*mSunXRotDegreeInit);
    mCameraOffset.set(*mCameraOffsetParam);
}

/**
 * Allocates the intermediate textures of the precomputation.
 */
void AtmosScatter::TemporaryTextures::createTextures() {
    mDeltaE = mAllocator->alloc(GameFrameworkNx::getAglDrawContext(), "PasDeltaE",
                                agl::TextureFormat::cTextureFormat_R11_G11_B10_float, 64, 16, 1,
                                nullptr, agl::utl::DynamicTextureAllocator::cAllocateType_1,
                                true, false);
    mDeltaSR = mAllocator->alloc3D(
        GameFrameworkNx::getAglDrawContext(), "PasDeltaSR3D",
        agl::TextureFormat::cTextureFormat_R11_G11_B10_float, 256, 128, cLayerNum, 1, nullptr,
        agl::utl::DynamicTextureAllocator::cAllocateType_1, true, false);
    mDeltaSM = mAllocator->alloc3D(
        GameFrameworkNx::getAglDrawContext(), "PasDeltaSM3D",
        agl::TextureFormat::cTextureFormat_R11_G11_B10_float, 256, 128, cLayerNum, 1, nullptr,
        agl::utl::DynamicTextureAllocator::cAllocateType_1, true, false);
    mDeltaJ = mAllocator->alloc3D(
        GameFrameworkNx::getAglDrawContext(), "PasmDeltaJ3D",
        agl::TextureFormat::cTextureFormat_R11_G11_B10_float, 256, 128, cLayerNum, 1, nullptr,
        agl::utl::DynamicTextureAllocator::cAllocateType_1, true, false);
}

/**
 * Rotates the sun along its orbit.
 */
void AtmosScatter::updateAtmosScatter() {
    sead::Quatf rotate;
    rotate.setAxisAngle(sead::Vector3f::ey, *mSunYRotDegree);
    mSunAxis.setRotated(rotate, sead::Vector3f::ex);
    sead::Vector3f orbitAxis;
    orbitAxis.setRotated(rotate, sead::Vector3f::ez);

    mSunXRot = wrapValue(mSunXRot + sead::Mathf::deg2rad(*mSunXRotSpeedDegree),
                         sead::Mathf::pi2());

    rotate.setAxisAngle(orbitAxis, *mSunZRotDegree);
    mSunAxis.setRotated(rotate, mSunAxis);
    sead::Vector3f up;
    up.setRotated(rotate, sead::Vector3f::ey);

    rotate.setAxisRadian(mSunAxis, mSunXRot);
    mSunDir.setRotated(rotate, up);
    normalizeOrZero(&mSunDir);
}

/**
 * Updates the precomputation flags and swaps the uniform blocks.
 */
void AtmosScatter::preDrawGraphics() {
    mIsPreDrawnPrev = mIsPreDrawn;
    mIsPreDrawn = true;
    mIsPrecompute = mIsRequestPrecompute;
    mIsRequestPrecompute = false;

    for (s32 i = 0; i < mViewUbos.size(); i++) {
        mViewUbos[i]->mUniformBlock->swap();
    }
}

/**
 * Searches the variation of an atmosphere shader matching the current settings.
 * @param pProgram Base shader program.
 * @param renderType Render type of the variation.
 * @return The shader program variation.
 */
const agl::ShaderProgram* AtmosScatter::searchVariation(const agl::ShaderProgram* pProgram,
                                                        s32 renderType) const {
    const char* macros[] = {"IS_TRANSMITTANCE_NON_LINEAR", "IS_INSCATTER_NON_LINEAR",
                            "IS_SUN_SMOOTH_STEP", "RENDER_TYPE"};
    const char* values[] = {"1", "1", "1", "0"};

    if (!mIsTransmittanceNonLinear) {
        values[0] = "0";
    }

    if (!mIsInscatterNonLinear) {
        values[1] = "0";
    }

    switch (renderType) {
    case RenderType::cForward:
        values[3] = "0";
        break;
    case RenderType::cDeferred:
        values[3] = "1";
        break;
    case RenderType::cCubeMap:
        values[3] = "2";
        break;
    }

    return pProgram->searchVariation(4, macros, values);
}

/**
 * Precomputes the transmittance, irradiance and inscatter textures.
 * @param shaderMode Current shader mode.
 * @return The shader mode after drawing.
 */
agl::ShaderMode AtmosScatter::drawPrecompute(agl::ShaderMode shaderMode) const {
    if (!isPrecompute()) {
        return shaderMode;
    }

    TemporaryTextures textures(agl::utl::DynamicTextureAllocator::instance());
    textures.createTextures();
    mDeltaSRSampler->applyTextureData(*textures.mDeltaSR);
    mDeltaSMSampler->applyTextureData(*textures.mDeltaSM);
    mDeltaESampler->applyTextureData(*textures.mDeltaE);
    mDeltaJSampler->applyTextureData(*textures.mDeltaJ);

    sead::GraphicsContext context;
    context.setBlendEnable(false);
    context.setDepthEnable(false, false);
    context.apply(GameFrameworkNx::getDrawContext());

    // Transmittance
    {
        RenderBufferAttacher attacher(mRenderBuffer, mTransmittanceTex, nullptr, nullptr, nullptr,
                                      nullptr);
        sead::Viewport viewport(*mRenderBuffer);
        viewport.apply(GameFrameworkNx::getDrawContext(), *mRenderBuffer);

        const agl::ShaderProgram* program = searchVariation(mTransmittanceProgram, 0);
        program->activate(GameFrameworkNx::getAglDrawContext(), true);
        mQuadModel->drawQuad();
    }

    // Irradiance of the single scattering
    {
        RenderBufferAttacher attacher(mRenderBuffer, textures.mDeltaE, nullptr, nullptr, nullptr,
                                      nullptr);
        sead::Viewport viewport(*mRenderBuffer);
        viewport.apply(GameFrameworkNx::getDrawContext(), *mRenderBuffer);

        const agl::ShaderProgram* program = searchVariation(mIrradiance1Program, 0);
        program->activate(GameFrameworkNx::getAglDrawContext(), true);
        ShaderSamplerSetter transmittanceSetter(GameFrameworkNx::getAglDrawContext(), program,
                                                mTransmittanceSampler, "uTransmittance", false);
        mQuadModel->drawQuad();
    }

    // Inscatter of the single scattering
    {
        sead::GraphicsContext passContext;
        passContext.setBlendEnableMask(0);
        passContext.setDepthEnable(false, false);
        passContext.apply(GameFrameworkNx::getDrawContext());

        const agl::ShaderProgram* program = searchVariation(mInscatter1Program, 0);
        program->activate(GameFrameworkNx::getAglDrawContext(), true);
        ShaderSamplerSetter transmittanceSetter(GameFrameworkNx::getAglDrawContext(), program,
                                                mTransmittanceSampler, "uTransmittance", false);

        for (s32 layer = 0; layer < cLayerNum; layer++) {
            RenderBufferAttacher attacher(mRenderBuffer, layer, textures.mDeltaSR,
                                          textures.mDeltaSM, nullptr, nullptr, nullptr);
            sead::Viewport viewport(*mRenderBuffer);
            viewport.apply(GameFrameworkNx::getDrawContext(), *mRenderBuffer);
            setLayer(program, layer);
            mQuadModel->drawQuad();
        }
    }

    // Clear the irradiance
    {
        RenderBufferAttacher attacher(mRenderBuffer, mIrradianceTex, nullptr, nullptr, nullptr,
                                      nullptr);
        sead::Viewport viewport(*mRenderBuffer);
        viewport.apply(GameFrameworkNx::getDrawContext(), *mRenderBuffer);
        mRenderBuffer->clear(GameFrameworkNx::getDrawContext(), sead::FrameBuffer::cColor,
                             sead::Color4f::cBlack, 0.0f, 0);
    }

    // Copy the single scattering to the inscatter
    {
        sead::GraphicsContext passContext;
        passContext.setBlendEnableMask(0);
        passContext.setDepthEnable(false, false);
        passContext.apply(GameFrameworkNx::getDrawContext());

        const agl::ShaderProgram* program = mCopyInscatter1Program;
        program->activate(GameFrameworkNx::getAglDrawContext(), true);
        ShaderSamplerSetter deltaSRSetter(GameFrameworkNx::getAglDrawContext(), program,
                                          mDeltaSRSampler, "uDeltaSR", false);
        ShaderSamplerSetter deltaSMSetter(GameFrameworkNx::getAglDrawContext(), program,
                                          mDeltaSMSampler, "uDeltaSM", false);

        for (s32 layer = 0; layer < cLayerNum; layer++) {
            RenderBufferAttacher attacher(mRenderBuffer, layer, mInscatterTex, nullptr, nullptr,
                                          nullptr, nullptr);
            sead::Viewport viewport(*mRenderBuffer);
            viewport.apply(GameFrameworkNx::getDrawContext(), *mRenderBuffer);
            f32 copyDepth = (layer + 0.5f) / cLayerNum;
            agl::UniformLocation location("uCopyDepth");
            location.search(*program);
            location.setUniform(GameFrameworkNx::getAglDrawContext(), copyDepth);
            mQuadModel->drawQuad();
        }
    }

    // Multiple scattering
    for (s32 order = 2; order < cScatteringOrderNum + 1; order++) {
        {
            sead::GraphicsContext passContext;
            passContext.setBlendEnableMask(0);
            passContext.setDepthEnable(false, false);
            passContext.apply(GameFrameworkNx::getDrawContext());

            const agl::ShaderProgram* program = searchVariation(mInscatterSProgram, 0);
            program->activate(GameFrameworkNx::getAglDrawContext(), true);
            ShaderSamplerSetter deltaESetter(GameFrameworkNx::getAglDrawContext(), program,
                                             mDeltaESampler, "uDeltaE", false);
            ShaderSamplerSetter transmittanceSetter(GameFrameworkNx::getAglDrawContext(), program,
                                                    mTransmittanceSampler, "uTransmittance",
                                                    false);
            ShaderSamplerSetter deltaSRSetter(GameFrameworkNx::getAglDrawContext(), program,
                                              mDeltaSRSampler, "uDeltaSR", false);
            ShaderSamplerSetter deltaSMSetter(GameFrameworkNx::getAglDrawContext(), program,
                                              mDeltaSMSampler, "uDeltaSM", false);
            agl::UniformLocation location("first");
            location.search(*program);
            location.setUniform(GameFrameworkNx::getAglDrawContext(), order == 2 ? 1.0f : 0.0f);

            for (s32 layer = 0; layer < cLayerNum; layer++) {
                RenderBufferAttacher attacher(mRenderBuffer, layer, textures.mDeltaJ, nullptr,
                                              nullptr, nullptr, nullptr);
                sead::Viewport viewport(*mRenderBuffer);
                viewport.apply(GameFrameworkNx::getDrawContext(), *mRenderBuffer);
                setLayer(program, layer);
                mQuadModel->drawQuad();
            }
        }

        {
            sead::GraphicsContext passContext;
            passContext.setBlendEnable(false);
            passContext.setDepthEnable(false, false);
            passContext.apply(GameFrameworkNx::getDrawContext());

            RenderBufferAttacher attacher(mRenderBuffer, textures.mDeltaE, nullptr, nullptr,
                                          nullptr, nullptr);
            sead::Viewport viewport(*mRenderBuffer);
            viewport.apply(GameFrameworkNx::getDrawContext(), *mRenderBuffer);

            const agl::ShaderProgram* program = searchVariation(mIrradianceNProgram, 0);
            program->activate(GameFrameworkNx::getAglDrawContext(), true);
            ShaderSamplerSetter deltaSRSetter(GameFrameworkNx::getAglDrawContext(), program,
                                              mDeltaSRSampler, "uDeltaSR", false);
            ShaderSamplerSetter deltaSMSetter(GameFrameworkNx::getAglDrawContext(), program,
                                              mDeltaSMSampler, "uDeltaSM", false);
            agl::UniformLocation location("first");
            location.search(*program);
            location.setUniform(GameFrameworkNx::getAglDrawContext(), order == 2 ? 1.0f : 0.0f);
            mQuadModel->drawQuad();
        }

        {
            sead::GraphicsContext passContext;
            passContext.setBlendEnable(false);
            passContext.setDepthEnable(false, false);
            passContext.apply(GameFrameworkNx::getDrawContext());

            const agl::ShaderProgram* program = searchVariation(mInscatterNProgram, 0);
            program->activate(GameFrameworkNx::getAglDrawContext(), true);
            ShaderSamplerSetter transmittanceSetter(GameFrameworkNx::getAglDrawContext(), program,
                                                    mTransmittanceSampler, "uTransmittance",
                                                    false);
            ShaderSamplerSetter deltaJSetter(GameFrameworkNx::getAglDrawContext(), program,
                                             mDeltaJSampler, "uDeltaJ", false);

            for (s32 layer = 0; layer < cLayerNum; layer++) {
                RenderBufferAttacher attacher(mRenderBuffer, layer, textures.mDeltaSR, nullptr,
                                              nullptr, nullptr, nullptr);
                sead::Viewport viewport(*mRenderBuffer);
                viewport.apply(GameFrameworkNx::getDrawContext(), *mRenderBuffer);
                setLayer(program, layer);
                mQuadModel->drawQuad();
            }
        }

        {
            sead::GraphicsContext passContext;
            passContext.setColorMask(true, true, true, true);
            // Additive blending (one, one, add) to accumulate the scattering orders.
            passContext.setBlendFactor(0, 2, 2);
            passContext.setBlendEquation(0, 1);
            passContext.setBlendEnable(true);
            passContext.setDepthEnable(false, false);
            passContext.apply(GameFrameworkNx::getDrawContext());

            {
                RenderBufferAttacher attacher(mRenderBuffer, mIrradianceTex, nullptr, nullptr,
                                              nullptr, nullptr);
                sead::Viewport viewport(*mRenderBuffer);
                viewport.apply(GameFrameworkNx::getDrawContext(), *mRenderBuffer);

                const agl::ShaderProgram* program = searchVariation(mCopyIrradianceProgram, 0);
                program->activate(GameFrameworkNx::getAglDrawContext(), true);
                ShaderSamplerSetter deltaESetter(GameFrameworkNx::getAglDrawContext(), program,
                                                 mDeltaESampler, "uDeltaE", false);
                mQuadModel->drawQuad();
            }

            const agl::ShaderProgram* program = searchVariation(mCopyInscatterNProgram, 0);
            program->activate(GameFrameworkNx::getAglDrawContext(), true);
            ShaderSamplerSetter deltaSSetter(GameFrameworkNx::getAglDrawContext(), program,
                                             mDeltaSRSampler, "uDeltaS", false);

            for (s32 layer = 0; layer < cLayerNum; layer++) {
                RenderBufferAttacher attacher(mRenderBuffer, layer, mInscatterTex, nullptr,
                                              nullptr, nullptr, nullptr);
                sead::Viewport viewport(*mRenderBuffer);
                viewport.apply(GameFrameworkNx::getDrawContext(), *mRenderBuffer);
                setLayer(program, layer);
                mQuadModel->drawQuad();
            }
        }
    }

    return shaderMode;
}

/**
 * Calculates the position of the camera relative to the center of the earth.
 * @param pCameraPos Output camera position in kilometers.
 */
inline void AtmosScatter::calcCameraPos(sead::Vector3f* pCameraPos) const {
    sead::Vector3f cameraPos(0.0f, 0.0f, 0.0f);
    cameraPos += mCameraOffset;
    cameraPos.y += getRgKm();

    if (cameraPos.length() < cCameraRadiusMinKm) {
        setLength(&cameraPos, cCameraRadiusMinKm);
    }

    pCameraPos->set(cameraPos);
}

/**
 * Calculates the cosines of the apparent sun size.
 * @param pSunSizeCos Output cosines of the inner and outer sun size.
 */
inline void AtmosScatter::calcSunSizeCos(sead::Vector2f* pSunSizeCos) const {
    f32 innerRadian = sead::Mathf::deg2rad(mSunSizeDegree);
    f32 outerRadian = sead::Mathf::deg2rad(mSunSizeDegreeOuter);
    pSunSizeCos->x = sead::Mathf::cos(innerRadian);
    pSunSizeCos->y = sead::Mathf::cos(outerRadian);
}

/**
 * Draws the far atmosphere to a forward rendered buffer.
 * @param viewIndex Index of the view.
 * @param rViewMtx View matrix.
 * @param rProjMtx Projection matrix.
 * @param rProjOffset Projection offset.
 * @param fovy Vertical field of view.
 * @param aspect Aspect ratio.
 * @param shaderMode Current shader mode.
 * @return The shader mode after drawing.
 */
agl::ShaderMode AtmosScatter::drawFarForward(s32 viewIndex, const sead::Matrix34f& rViewMtx,
                                             const sead::Matrix44f& rProjMtx,
                                             const sead::Vector2f& rProjOffset, f32 fovy,
                                             f32 aspect, agl::ShaderMode shaderMode) const {
    sead::GraphicsContext context;
    context.setColorMask(true, true, true, true);
    context.setDepthEnable(true, false);
    context.setDepthFunc(3);
    context.setBlendEnable(false);
    context.apply(GameFrameworkNx::getDrawContext());

    return drawFar(viewIndex, rViewMtx, rProjMtx, rProjOffset, fovy, aspect,
                   RenderType::cForward, shaderMode);
}

/**
 * Draws the far atmosphere.
 * @param viewIndex Index of the view.
 * @param rViewMtx View matrix.
 * @param rProjMtx Projection matrix.
 * @param rProjOffset Projection offset.
 * @param fovy Vertical field of view.
 * @param aspect Aspect ratio.
 * @param renderType Render target type.
 * @param shaderMode Current shader mode.
 * @return The shader mode after drawing.
 */
agl::ShaderMode AtmosScatter::drawFar(s32 viewIndex, const sead::Matrix34f& rViewMtx,
                                      const sead::Matrix44f& rProjMtx,
                                      const sead::Vector2f& rProjOffset, f32 fovy, f32 aspect,
                                      RenderType renderType, agl::ShaderMode shaderMode) const {
    ViewUbo* viewUbo = mViewUbos[viewIndex];

    const agl::ShaderProgram* program = searchVariation(mRenderEarthProgram, renderType);
    program->activate(GameFrameworkNx::getAglDrawContext(), true);
    ShaderSamplerSetter transmittanceSetter(GameFrameworkNx::getAglDrawContext(), program,
                                            mTransmittanceSampler, "uTransmittance", false);
    ShaderSamplerSetter earthSetter(
        GameFrameworkNx::getAglDrawContext(), program,
        agl::utl::PrimitiveTexture::instance()->getTextureSampler(
            agl::utl::PrimitiveTexture::cType_Gray2D),
        "uEarthTex", false);
    ShaderSamplerSetter irradianceSetter(GameFrameworkNx::getAglDrawContext(), program,
                                         mIrradianceSampler, "uIrradiance", false);
    ShaderSamplerSetter inscatterSetter(GameFrameworkNx::getAglDrawContext(), program,
                                        mInscatterSampler, "uInscatter", false);

    sead::Matrix34f viewRotMtx(sead::Matrix33f(rViewMtx), sead::Vector3f::zero);
    sead::Matrix34f invViewRotMtx;
    invViewRotMtx.setInverse(viewRotMtx);
    sead::Matrix44f invProjMtx;
    invProjMtx.setInverse(rProjMtx);
    sead::Matrix34f invViewProjMtx;
    multiplyMtx34Mtx44(&invViewProjMtx, invViewRotMtx, invProjMtx);

    sead::Vector3f cameraPos;
    calcCameraPos(&cameraPos);
    const sead::Vector3f sunDir = mSunDir;

    viewUbo->mUniformBlock->setData(0, &cameraPos, 0, 1);
    setUniformBlockValue(viewUbo->mUniformBlock, 1,
                         sead::Vector4f(sunDir.x, sunDir.y, sunDir.z, mSunIntensity));
    setUniformBlockValue(viewUbo->mUniformBlock, 2,
                         sead::Vector4f(-sunDir.x, -sunDir.y, -sunDir.z, mSunIntensity * 0.001f));
    viewUbo->mUniformBlock->setData(3, &invViewProjMtx, 0, 3);

    sead::Vector2f sunSizeCos;
    calcSunSizeCos(&sunSizeCos);
    viewUbo->mUniformBlock->setData(5, &sunSizeCos, 0, 1);

    sead::Vector2f tanFovyHalf;
    calcTanFovyHalf(nullptr, &tanFovyHalf, fovy, aspect, rProjOffset);
    viewUbo->mUniformBlock->setData(4, &tanFovyHalf, 0, 1);

    agl::UniformBlockLocation location("PasRenderEarth");
    location.search(*program);
    viewUbo->mUniformBlock->activate(GameFrameworkNx::getAglDrawContext(), location);
    viewUbo->mUniformBlock->flushCurrentBuffer();

    mQuadModel->drawQuad();

    return shaderMode;
}

/**
 * Draws the far atmosphere to a cube map.
 * @param viewIndex Index of the view.
 * @param rViewMtx View matrix.
 * @param rProjMtx Projection matrix.
 * @param rProjOffset Projection offset.
 * @param fovy Vertical field of view.
 * @param aspect Aspect ratio.
 * @param shaderMode Current shader mode.
 * @return The shader mode after drawing.
 */
agl::ShaderMode AtmosScatter::drawFarToCubeMap(s32 viewIndex, const sead::Matrix34f& rViewMtx,
                                               const sead::Matrix44f& rProjMtx,
                                               const sead::Vector2f& rProjOffset, f32 fovy,
                                               f32 aspect, agl::ShaderMode shaderMode) const {
    sead::GraphicsContext context;
    context.setColorMask(true, true, true, true);
    context.setDepthEnable(true, false);
    context.setDepthFunc(3);
    context.setBlendEnable(false);
    context.apply(GameFrameworkNx::getDrawContext());

    return drawFar(viewIndex, rViewMtx, rProjMtx, rProjOffset, fovy, aspect,
                   RenderType::cCubeMap, shaderMode);
}

/**
 * Draws the far atmosphere to the G-buffer.
 * @param viewIndex Index of the view.
 * @param pGBufferArray G-buffer to draw to.
 * @param rViewMtx View matrix.
 * @param rProjMtx Projection matrix.
 * @param rProjOffset Projection offset.
 * @param fovy Vertical field of view.
 * @param aspect Aspect ratio.
 * @param shaderMode Current shader mode.
 * @return The shader mode after drawing.
 */
agl::ShaderMode AtmosScatter::drawFarDeferred(s32 viewIndex, GBufferArray* pGBufferArray,
                                              const sead::Matrix34f& rViewMtx,
                                              const sead::Matrix44f& rProjMtx,
                                              const sead::Vector2f& rProjOffset, f32 fovy,
                                              f32 aspect, agl::ShaderMode shaderMode) const {
    RenderBufferAttacher attacher(mRenderBuffer, pGBufferArray->getGBufAlbedoTex(),
                                  pGBufferArray->getGBufNrmViewTex(),
                                  pGBufferArray->getGBufDepthViewTex(),
                                  pGBufferArray->getGBufLightBufferTex(), nullptr);

    sead::GraphicsContextMRT context;
    GBufferArray::setContextMRT(&context);
    context.setDepthEnable(true, false);
    context.setDepthFunc(3);
    context.apply(GameFrameworkNx::getDrawContext());

    return drawFar(viewIndex, rViewMtx, rProjMtx, rProjOffset, fovy, aspect,
                   RenderType::cDeferred, shaderMode);
}

/**
 * Calculates the camera position, the sun direction and the direction of the sun light that
 * reaches the camera.
 * @param pCameraPos Output camera position in kilometers, can be nullptr.
 * @param pSunDir Output sun direction, can be nullptr.
 * @param pLightSunDir Output sun light direction clamped above the horizon, can be nullptr.
 */
void AtmosScatter::calcInfo(sead::Vector3f* pCameraPos, sead::Vector3f* pSunDir,
                            sead::Vector3f* pLightSunDir) const {
    sead::Vector3f cameraPos(0.0f, 0.0f, 0.0f);
    cameraPos += mCameraOffset;
    cameraPos.y += getRgKm();

    if (cameraPos.length() < cCameraRadiusMinKm) {
        setLength(&cameraPos, cCameraRadiusMinKm);
    }

    if (pCameraPos != nullptr) {
        pCameraPos->set(cameraPos);
    }

    if (pSunDir != nullptr) {
        pSunDir->set(mSunDir);
    }

    if (pLightSunDir == nullptr) {
        return;
    }

    sead::Vector3f down = -cameraPos;

    if (!isParallelDirection(down, mSunDir, 0.01f)) {
        f32 horizonAngle = sead::Mathf::asin(
            sead::Mathf::clamp(cCameraRadiusMinKm / down.length(), 0.0f, 1.0f));
        sead::Vector3f downDir = down;
        normalizeOrZero(&downDir);

        sead::Vector3f axis;
        f32 sunAngle;

        {
            sead::Quatf sunRotate;
            sunRotate.makeVectorRotation(downDir, mSunDir);
            calcQuatRotateAxisAndDegree(&axis, &sunAngle, sunRotate);
        }

        sunAngle = sead::Mathf::deg2rad(sunAngle);

        f32 halfHorizonAngle = horizonAngle * 0.5f;
        f32 halfCos = sead::Mathf::cos(halfHorizonAngle);
        f32 halfSin = sead::Mathf::sin(halfHorizonAngle);
        sead::Quatf horizonRotate(halfCos, halfSin * axis.x, halfSin * axis.y, halfSin * axis.z);
        sead::Vector3f horizonDir = downDir;
        rotateVectorQuat(&horizonDir, horizonRotate);
        normalizeOrZero(&horizonDir);

        if (!(horizonAngle <= sunAngle)) {
            pLightSunDir->set(horizonDir);
            return;
        }
    }

    pLightSunDir->set(mSunDir);
}

/**
 * Draws the color of the directional sun light to a texture.
 * @param rColor Color the light is multiplied with.
 * @param pTexture Texture to draw to.
 * @param shaderMode Current shader mode.
 * @return The shader mode after drawing.
 */
agl::ShaderMode AtmosScatter::drawDirLightColor(const sead::Color4f& rColor,
                                                const agl::TextureData* pTexture,
                                                agl::ShaderMode shaderMode) const {
    sead::GraphicsContext context;
    context.setColorMask(true, true, true, true);
    context.setDepthEnable(false, false);
    context.setBlendEnable(false);
    context.apply(GameFrameworkNx::getDrawContext());

    RenderBufferAttacher attacher(mRenderBuffer, pTexture, nullptr, nullptr, nullptr, nullptr);
    sead::Viewport viewport(*mRenderBuffer);
    viewport.apply(GameFrameworkNx::getDrawContext(), *mRenderBuffer);

    const agl::ShaderProgram* program = searchVariation(mRenderDirLightColorProgram, 0);
    program->activate(GameFrameworkNx::getAglDrawContext(), true);
    ShaderSamplerSetter transmittanceSetter(GameFrameworkNx::getAglDrawContext(), program,
                                            mTransmittanceSampler, "uTransmittance", false);
    ShaderSamplerSetter inscatterSetter(GameFrameworkNx::getAglDrawContext(), program,
                                        mInscatterSampler, "uInscatter", false);

    sead::Vector3f cameraPos;
    sead::Vector3f sunDir;
    sead::Vector3f lightSunDir;
    calcInfo(&cameraPos, &sunDir, &lightSunDir);

    setUniformArray(GameFrameworkNx::getAglDrawContext(), program, "uCamPos", 3, &cameraPos);
    setUniformArray(GameFrameworkNx::getAglDrawContext(), program, "uSunDir", 3, &sunDir);
    setUniformArray(GameFrameworkNx::getAglDrawContext(), program, "uLightSunDir", 3,
                    &lightSunDir);
    setUniformValue(GameFrameworkNx::getAglDrawContext(), program, "uSunIntensity",
                    mSunIntensity);
    setUniformArray(GameFrameworkNx::getAglDrawContext(), program, "uLightMulColor", 4, &rColor);

    sead::Vector2f sunSizeCos;
    calcSunSizeCos(&sunSizeCos);
    setUniformArray(GameFrameworkNx::getAglDrawContext(), program, "uSunSizeCos", 2,
                    &sunSizeCos);

    mQuadModel->drawQuad();

    return shaderMode;
}

}  // namespace al

namespace AtmosScatterFunction {

/**
 * Gets the atmospheric scattering of the scene.
 * @param pActor Actor of the scene.
 * @return The atmospheric scattering.
 */
al::AtmosScatter* getAtmosScatter(const al::LiveActor* pActor) {
    return pActor->getSceneInfo()->graphicsSystemInfo->getAtmosScatter();
}

}  // namespace AtmosScatterFunction
