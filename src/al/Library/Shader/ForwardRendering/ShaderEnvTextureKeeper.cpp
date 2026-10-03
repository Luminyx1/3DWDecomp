#include "Library/Shader/ForwardRendering/ShaderEnvTextureKeeper.hpp"

#include <attributes.h>
#include <common/aglTextureSampler.h>
#include <gfx/seadCamera.h>
#include <gfx/seadGraphicsContextMRT.h>
#include <nn/g3d/g3d_ResMaterial.h>
#include <utility/aglParameterIO.h>
#include <utility/aglPrimitiveTexture.h>
#include <utility/aglResParameter.h>

#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Model/SimpleModelEnv.hpp"
#include "Library/Resource/ResourceFunction.hpp"
#include "Library/Shader/Block/UniformBlock.hpp"
#include "Library/Shader/Block/UniformBlockUtil.hpp"
#include "Library/Shader/DeferredRendering/CubeMapDirector.hpp"
#include "Library/Shader/DeferredRendering/FullScreenQuadModel.hpp"
#include "Library/Shader/DeferredRendering/GBufferArray.hpp"
#include "Library/Shader/DeferredRendering/SamplerLocation.hpp"
#include "Library/Shader/ForwardRendering/EnvTextureKeeper.hpp"
#include "Library/Shader/ForwardRendering/ShaderFresnelTextureKeeper.hpp"
#include "Library/Shader/ForwardRendering/ShaderHolder.hpp"
#include "Library/Shader/ForwardRendering/ShaderMirrorDirector.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Draw/GraphicsAreaDirector.hpp"
#include "Project/Model/ModelAdditionalInfo.hpp"

namespace nn::g3d {
class ResShaderProgram;
}  // namespace nn::g3d

namespace al {
const nn::g3d::ResShaderProgram* searchVariation(const nn::g3d::ResShadingModel* pShadingModel,
                                                 s32 macroNum, const char* const* pMacros,
                                                 const char* const* pValues);
agl::ShaderMode activateShader(const nn::g3d::ResShaderProgram* pProgram,
                               agl::ShaderMode shaderMode);
}  // namespace al

namespace {
const al::UniformBlockLayout cRenderCubeMapLayout[] = {
    {0, agl::UniformBlock::cType_Float, 1},
};

constexpr const char* cUnsetParamName = "※投影パラメータ名を入力してください";

/**
 * @brief Picks the override id when it is valid, else the base id.
 */
ALWAYS_INLINE s32 selectTexId(s32 overrideId, s32 baseId) {
    return overrideId != -1 && overrideId != -100 ? overrideId : baseId;
}

/**
 * @brief Gets the fresnel id of an info, preferring the override.
 */
ALWAYS_INLINE s32 getFresnel(const al::EnvTexInfo& rInfo) {
    return selectTexId(rInfo.getOverrideId().getFresnel(), rInfo.getBaseId().getFresnel());
}

/**
 * @brief Gets the thickness id of an info, preferring the override.
 */
ALWAYS_INLINE s32 getThickness(const al::EnvTexInfo& rInfo) {
    return selectTexId(rInfo.getOverrideId().getThickness(), rInfo.getBaseId().getThickness());
}

/**
 * @brief Gets the mirror texture id of an info, preferring the override.
 */
ALWAYS_INLINE s32 getMirrorTexId(const al::EnvTexInfo& rInfo) {
    return selectTexId(rInfo.getOverrideId().getMirrorTexId(),
                       rInfo.getBaseId().getMirrorTexId());
}

/**
 * @brief Gets the reflection cube map id of an info, preferring the override.
 */
ALWAYS_INLINE s32 getCubeMapId(const al::EnvTexInfo& rInfo) {
    return selectTexId(rInfo.getOverrideId().getCubeMapId(), rInfo.getBaseId().getCubeMapId());
}

/**
 * @brief Gets the roughness id of an info, preferring the override.
 */
ALWAYS_INLINE s32 getRoughness(const al::EnvTexInfo& rInfo) {
    return selectTexId(rInfo.getOverrideId().getRoughness(), rInfo.getBaseId().getRoughness());
}

/**
 * @brief Gets the refraction cube map id of an info, preferring the override.
 */
ALWAYS_INLINE s32 getRefractCubeMapId(const al::EnvTexInfo& rInfo) {
    return selectTexId(rInfo.getOverrideId().getRefractCubeMapId(),
                       rInfo.getBaseId().getRefractCubeMapId());
}

/**
 * @brief Gets the refraction type of an info, preferring the override.
 */
ALWAYS_INLINE s32 getRefract(const al::EnvTexInfo& rInfo) {
    return selectTexId(rInfo.getOverrideId().getRefract(), rInfo.getBaseId().getRefract());
}

/**
 * @brief Gets the irradiance id of an info, preferring the override.
 */
ALWAYS_INLINE s32 getIrradiance(const al::EnvTexInfo& rInfo) {
    return selectTexId(rInfo.getOverrideId().getIrradiance(), rInfo.getBaseId().getIrradiance());
}

/**
 * @brief Gets the light category of an info: the override, then the material, then the default.
 */
ALWAYS_INLINE s32 getLightCategory(const al::EnvTexInfo& rInfo, s32 defaultCategory) {
    if (rInfo.getOverrideId().getLightCategory() != -1) {
        return rInfo.getOverrideId().getLightCategory();
    }

    if (rInfo.getBaseId().getLightCategory() != -1) {
        return rInfo.getBaseId().getLightCategory();
    }

    return defaultCategory;
}

/**
 * @brief Checks whether an id is neither invalid nor uncached.
 */
ALWAYS_INLINE bool isValidTexId(s32 id) {
    return id != -1 && id != -100;
}

/**
 * @brief Takes the fresnel id of an info and reports whether it changed.
 */
ALWAYS_INLINE bool checkAndSetFresnelChange(al::EnvTexId* pId, const al::EnvTexInfo& rInfo) {
    bool isChanged = pId->getFresnel() != getFresnel(rInfo);
    pId->setFresnel(getFresnel(rInfo));
    return isChanged;
}

/**
 * @brief Takes the thickness id of an info and reports whether it changed.
 */
ALWAYS_INLINE bool checkAndSetThicknessChange(al::EnvTexId* pId, const al::EnvTexInfo& rInfo) {
    bool isChanged = pId->getThickness() != getThickness(rInfo);
    pId->setThickness(getThickness(rInfo));
    return isChanged;
}

/**
 * @brief Takes the mirror texture id of an info and reports whether it changed.
 */
ALWAYS_INLINE bool checkAndSetMirrorTexChange(al::EnvTexId* pId, const al::EnvTexInfo& rInfo) {
    bool isChanged = pId->getMirrorTexId() != getMirrorTexId(rInfo);
    pId->setMirrorTexId(getMirrorTexId(rInfo));
    return isChanged;
}

/**
 * @brief Takes the reflection settings and light category of an info and reports whether any
 * changed.
 */
ALWAYS_INLINE bool checkAndSetReflectCubeMapChange(al::EnvTexId* pId, const al::EnvTexInfo& rInfo,
                                                   s32 defaultCategory) {
    s32 category = getLightCategory(rInfo, defaultCategory);
    bool isChanged = (pId->getCubeMapId() != getCubeMapId(rInfo)) |
                     (pId->getRoughness() != getRoughness(rInfo)) |
                     (pId->getLightCategory() != category);
    pId->setCubeMapId(getCubeMapId(rInfo));
    pId->setRoughness(getRoughness(rInfo));
    pId->setLightCategory(category);
    return isChanged;
}

/**
 * @brief Takes the refraction settings and light category of an info and reports whether any
 * changed.
 */
ALWAYS_INLINE bool checkAndSetRefractChange(al::EnvTexId* pId, const al::EnvTexInfo& rInfo,
                                            s32 defaultCategory) {
    s32 category = getLightCategory(rInfo, defaultCategory);
    bool isChanged = (pId->getRefractCubeMapId() != getRefractCubeMapId(rInfo)) |
                     (pId->getRoughness() != getRoughness(rInfo)) |
                     (pId->getRefract() != getRefract(rInfo)) |
                     (pId->getLightCategory() != category);
    pId->setRefractCubeMapId(getRefractCubeMapId(rInfo));
    pId->setRoughness(getRoughness(rInfo));
    pId->setRefract(getRefract(rInfo));
    pId->setLightCategory(category);
    return isChanged;
}

/**
 * @brief Takes the irradiance id and light category of an info and reports whether either changed.
 */
ALWAYS_INLINE bool checkAndSetIrradianceChange(al::EnvTexId* pId, const al::EnvTexInfo& rInfo,
                                               s32 defaultCategory) {
    s32 category = getLightCategory(rInfo, defaultCategory);
    bool isChanged =
        (pId->getIrradiance() != getIrradiance(rInfo)) | (pId->getLightCategory() != category);
    pId->setIrradiance(getIrradiance(rInfo));
    pId->setLightCategory(category);
    return isChanged;
}

/**
 * @brief Binds an indirect texture, or the black primitive texture when there is none.
 * @return Whether the given texture was bound.
 */
ALWAYS_INLINE bool activateIndirectTexture(const agl::TextureData* pTexture) {
    const agl::SamplerLocation& location = al::getSamplerLocationIndirect();

    if (pTexture == nullptr) {
        agl::utl::PrimitiveTexture::instance()
            ->getTextureSampler(agl::utl::PrimitiveTexture::cType_Black2D)
            ->activate(al::GameFrameworkNx::getAglDrawContext(), location, -1, false);
        return false;
    }

    if (!location.isValid()) {
        return false;
    }

    agl::TextureSampler sampler(*pTexture);
    sampler.setWrapDirect(6, 6, 6);
    sampler.activate(al::GameFrameworkNx::getAglDrawContext(), location, -1, false);
    return true;
}
/**
 * @brief Applies the render state of the cube map sky: depth test without depth write.
 * @param isMRT Whether the target is the g-buffer.
 */
ALWAYS_INLINE void applySkyContext(sead::GraphicsContext* pContext) {
    pContext->setDepthEnable(true, false);
    pContext->setDepthFunc(3);
    pContext->apply(al::GameFrameworkNx::getAglDrawContext());
}

ALWAYS_INLINE void applySkyContext(bool isMRT) {
    if (isMRT) {
        sead::GraphicsContextMRT context;
        al::GBufferArray::setContextMRT(&context);
        applySkyContext(&context);
    } else {
        sead::GraphicsContextMRT context;
        context.setBlendEnable(false);
        applySkyContext(&context);
    }
}
}  // namespace

namespace al {

/**
 * @brief Creates the keeper; textures are set up later by initTexture.
 * @param pInfo Graphics system info.
 * @param pPlayerHolder Player holder (unused).
 */
ShaderEnvTextureKeeper::ShaderEnvTextureKeeper(GraphicsSystemInfo* pInfo,
                                               PlayerHolder* pPlayerHolder)
    : mGraphicsSystemInfo(pInfo) {}

/**
 * @brief Destroys the fresnel keeper, the uniform block and the sky quad.
 */
ShaderEnvTextureKeeper::~ShaderEnvTextureKeeper() {
    if (mFresnelTextureKeeper != nullptr) {
        delete mFresnelTextureKeeper;
        mFresnelTextureKeeper = nullptr;
    }

    if (mUniformBlock != nullptr) {
        delete mUniformBlock;
        mUniformBlock = nullptr;
    }

    if (mFullScreenQuadModel != nullptr) {
        delete mFullScreenQuadModel;
        mFullScreenQuadModel = nullptr;
    }
}

/**
 * @brief Creates the fresnel textures and, when the shader exists, the cube map sky renderer.
 * @param pShaderHolder Shader holder to take the shading models from.
 */
void ShaderEnvTextureKeeper::initTexture(ShaderHolder* pShaderHolder) {
    mFresnelTextureKeeper =
        new ShaderFresnelTextureKeeper(mGraphicsSystemInfo->getGpuMemAllocator(), pShaderHolder,
                                       mGraphicsSystemInfo->getLodSettingName());
    mUniformBlock = createUniformBlock(cRenderCubeMapLayout, 1, nullptr, 2);

    nn::g3d::ResShadingModel* shadingModel = pShaderHolder->getShadingModel("RenderCubeMap");
    if (shadingModel != nullptr) {
        mShadingModel = shadingModel;
        mFullScreenQuadModel = new FullScreenQuadModel();
        mIsEnable = true;
    }
}

/**
 * @brief Loads the alpha mask projection parameters of the stage.
 * @param pAreaDirector Graphics area director.
 * @param pStageName Stage name.
 */
void ShaderEnvTextureKeeper::initGraphicsAreaParam(GraphicsAreaDirector* pAreaDirector,
                                                   const char* pStageName) {
    mAlphaMaskProjectionInfo = new AlphaMaskProjectionInfo(mGraphicsSystemInfo);
    mAlphaMaskProjectionInfo->init(pAreaDirector, pStageName);
}

/**
 * @brief Finishes initialization (nothing to do).
 */
void ShaderEnvTextureKeeper::endInit() {}

/**
 * @brief Binds the environment textures of a material, skipping the ones already bound for the
 * model.
 * @param rInfo Environment texture settings of the material.
 * @param pAdditionalInfo Model info caching the currently bound ids.
 * @param isForce Whether to force the fresnel textures.
 * @return Whether every cube map texture could be bound.
 */
bool ShaderEnvTextureKeeper::activateEnvTexture(const EnvTexInfo& rInfo,
                                                ModelAdditionalInfo* pAdditionalInfo,
                                                bool isForce) const {
    if (!mIsEnable) {
        return true;
    }

    EnvTexId* cache = pAdditionalInfo->getEnvTexId();
    s32 defaultCategory = pAdditionalInfo->getCategory();

    if (isValidTexId(getFresnel(rInfo)) && checkAndSetFresnelChange(cache, rInfo)) {
        mFresnelTextureKeeper->activateFresnelTexture(getFresnel(rInfo), isForce);
    }

    if (isValidTexId(getThickness(rInfo)) && checkAndSetThicknessChange(cache, rInfo)) {
        mFresnelTextureKeeper->activateThicknessCurveTexture(getThickness(rInfo), isForce);
    }

    const CubeMapDirector* cubeMapDirector = mGraphicsSystemInfo->getCubeMapDirector();
    bool isActivated = true;

    if (checkAndSetReflectCubeMapChange(cache, rInfo, defaultCategory)) {
        isActivated = cubeMapDirector->activateCubeMapTexture(
            getCubeMapId(rInfo), getRoughness(rInfo), getLightCategory(rInfo, defaultCategory),
            false);
    }

    if (static_cast<u32>(rInfo.getBaseId().getRefract()) < 6) {
        if (checkAndSetRefractChange(cache, rInfo, defaultCategory)) {
            isActivated &= cubeMapDirector->activateCubeMapTexture(
                getRefractCubeMapId(rInfo), getRefract(rInfo),
                getLightCategory(rInfo, defaultCategory), true);
        }
    }

    if (rInfo.getBaseId().getRefract() == 6) {
        if (cubeMapDirector->isDrawCapturePointCubeMap()) {
            agl::utl::PrimitiveTexture::instance()
                ->getTextureSampler(agl::utl::PrimitiveTexture::cType_Black2D)
                ->activate(GameFrameworkNx::getAglDrawContext(), getSamplerLocationIndirect(), -1,
                           false);
        } else if (checkAndSetRefractChange(cache, rInfo, defaultCategory)) {
            isActivated &= activateIndirectTexture(mIndirectTexture);
        }
    }

    if (checkAndSetIrradianceChange(cache, rInfo, defaultCategory)) {
        isActivated &= cubeMapDirector->activateCubeMapTexture(
            getIrradiance(rInfo), 5, getLightCategory(rInfo, defaultCategory), false);
    }

    if (checkAndSetMirrorTexChange(cache, rInfo)) {
        pAdditionalInfo->getGraphicsSystemInfo()->getShaderMirrorDirector()->activateMirrorTexture(
            getMirrorTexId(rInfo));
    }

    return isActivated;
}

/**
 * @brief Clears the indirect texture of the previous frame and updates the fresnel curves.
 */
void ShaderEnvTextureKeeper::updateEnvTexture() {
    mIndirectTexture = nullptr;

    if (mIsEnable) {
        mFresnelTextureKeeper->updateLutCurveTex();
    }
}

/**
 * @brief Updates the alpha mask projection direction from the current graphics area.
 */
void ShaderEnvTextureKeeper::execute() {
    if (mAlphaMaskProjectionInfo != nullptr) {
        mAlphaMaskProjectionInfo->update();
        mFrontDir = mAlphaMaskProjectionInfo->getCurrentDir();
        mIsUseViewMtx = mAlphaMaskProjectionInfo->isCameraDir();
    }
}

/**
 * @brief Draws the sky of a cube map face.
 * @param rViewMtx View matrix (unused).
 * @param rProjMtx Projection matrix (unused).
 * @param viewIndex View index to prepare the model environment for.
 * @param isMRT Whether the target is the g-buffer.
 * @param pModelEnv Model environment.
 * @param shaderMode Current shader mode.
 * @param intensity Sky intensity (negative for the default of 2).
 * @return The shader mode after drawing.
 */
agl::ShaderMode ShaderEnvTextureKeeper::renderCubeMapSky(const sead::Matrix34f& rViewMtx,
                                                         const sead::Matrix44f& rProjMtx,
                                                         s32 viewIndex, bool isMRT,
                                                         const SimpleModelEnv* pModelEnv,
                                                         agl::ShaderMode shaderMode,
                                                         f32 intensity) const {
    if (!mIsEnable) {
        return shaderMode;
    }

    applySkyContext(isMRT);
    mUniformBlock->setValue(0, intensity < 0.0f ? 2.0f : intensity);

    const char* macros[] = {"cIsMRT"};
    const char* values[] = {"1"};

    if (!isMRT) {
        values[0] = "0";
    }

    const nn::g3d::ResShaderProgram* program = searchVariation(mShadingModel, 1, macros, values);
    pModelEnv->prepareModelDraw(viewIndex);
    agl::ShaderMode newShaderMode = activateShader(program, shaderMode);
    mUniformBlock->activate(GameFrameworkNx::getAglDrawContext(),
                            getUniformBlockLocationRenderSky());
    mUniformBlock->flushCurrentBuffer();
    mFullScreenQuadModel->drawQuad();
    return newShaderMode;
}

}  // namespace al

namespace alEnvTexFunction {

/**
 * @brief Reads a light category from a render info ("1" -> 0, "2" -> 1, else -1).
 */
s32 calcLightCategory(const nn::g3d::ResRenderInfo* pRenderInfo) {
    if (pRenderInfo == nullptr) {
        return -1;
    }

    if (pRenderInfo->GetArrayLength() == 0) {
        return -1;
    }

    const char* category = pRenderInfo->GetString(0);

    if (al::isEqualString(category, "1")) {
        return 0;
    }

    return al::isEqualString(category, "2") ? 1 : -1;
}

}  // namespace alEnvTexFunction

namespace al {

// The alpha mask projection functions are called, not inlined, by the game (they most likely
// lived in their own source file), hence NOINLINE.

/**
 * @brief Creates the parameters of one projection setting.
 * @param isDefault Whether this is the default setting (named "Default").
 */
NOINLINE AlphaMaskProjectionParam::AlphaMaskProjectionParam(bool isDefault) {
    mName = new agl::utl::Parameter<sead::FixedSafeString<64>>(
        sead::FixedSafeString<64>(isDefault ? "Default" : cUnsetParamName), "name", "パラメータ名",
        this);
    mDir = new agl::utl::Parameter<sead::Vector3f>({0.0f, 0.0f, -1.0f}, "dir", "投影方向", this);
    mIsCameraDir =
        new agl::utl::Parameter<bool>(false, "is_camera_dir", "カメラからの投影", this);
}

/**
 * @brief Creates an empty info.
 * @param pInfo Graphics system info.
 */
NOINLINE AlphaMaskProjectionInfo::AlphaMaskProjectionInfo(const GraphicsSystemInfo* pInfo)
    : mGraphicsSystemInfo(pInfo) {}

/**
 * @brief Creates one setting per graphics area plus the default one and loads them from the
 * stage's parameter file. Settings left unnamed are not used.
 * @param pAreaDirector Graphics area director.
 * @param pStageName Stage name.
 */
NOINLINE void AlphaMaskProjectionInfo::init(const GraphicsAreaDirector* pAreaDirector,
                                   const char* pStageName) {
    mAreaDirector = pAreaDirector;
    mParamIO = new agl::utl::IParameterIO();

    u32 paramNum = pAreaDirector->getGraphicsAreaNum() + 1;
    mParams.allocBuffer(paramNum, nullptr);

    for (u32 i = 0; i < paramNum; i++) {
        AlphaMaskProjectionParam* param = new AlphaMaskProjectionParam(i == 0);
        mParams.pushBack(param);
        mParamIO->addObj(param, StringTmp<256>("%s_%d", "alpha_mask_projection_param", i).cstr());
    }

    mDefaultParam = mParams[0];
    mCurrentParam = mDefaultParam;

    const void* file = tryFindStageParameterFileDesign(pStageName, "DefaultParam.baglamp", 1);
    if (file != nullptr) {
        agl::utl::ResParameterArchive archive(file);
        mParamIO->applyResParameterArchive(archive);
        mIsLoaded = true;
    }

    for (u32 i = 0; i < paramNum; i++) {
        if (isEqualString(cUnsetParamName, mParams(i)->getName())) {
            mParamIO->removeObj(mParams[i]);
        }
    }
}

/**
 * @brief Selects the setting named by the current graphics area, or the default one.
 */
NOINLINE void AlphaMaskProjectionInfo::update() {
    CurrentGraphicsAreaParam areaParam;
    mAreaDirector->getCurrentGraphicsAreaParam(&areaParam,
                                               GraphicsAreaParamType::AlphaMaskProjection);

    if (areaParam.mParamName != nullptr) {
        u32 paramNum = mParams.size();
        for (u32 i = 0; i < paramNum; i++) {
            if (isEqualString(areaParam.mParamName, mParams(i)->getName())) {
                mCurrentParam = mParams[i];
                return;
            }
        }
    }

    mCurrentParam = mDefaultParam;
}

/**
 * @brief Checks whether the current setting projects from the camera.
 */
NOINLINE bool AlphaMaskProjectionInfo::isCameraDir() const {
    return mCurrentParam->isCameraDir();
}

/**
 * @brief Gets the projection direction of the current setting, following the camera when
 * requested.
 */
NOINLINE const sead::Vector3f& AlphaMaskProjectionInfo::getCurrentDir() const {
    const sead::LookAtCamera* camera = mGraphicsSystemInfo->getDrawCamera();

    if (camera != nullptr && mCurrentParam->isCameraDir()) {
        sead::Vector3f lookDir;
        camera->getLookVectorByMatrix(&lookDir);
        normalizeOrZero(&lookDir);
        mCurrentParam->setDir(-lookDir);
    }

    return mCurrentParam->getDir();
}

}  // namespace al
