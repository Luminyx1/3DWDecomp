#include "Library/Shader/DeferredRendering/ModelLightParam.hpp"

#include <cstdlib>

#include <attributes.h>

#include <common/aglShaderLocation.h>
#include <common/aglTextureSampler.h>
#include <nn/g3d/g3d_Resources.h>
#include "utility/aglPrimitiveTexture.h"
#include "utility/aglResParameter.h"

#include "Library/File/FileUtil.hpp"
#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/Model/Function/alModelFunction.hpp"
#include "Library/Resource/Resource.hpp"
#include "Library/Resource/ResourceFunction.hpp"
#include "Library/Shader/DeferredRendering/SamplerLocation.hpp"
#include "Library/Shader/ForwardRendering/ShaderSamplerSetter.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Draw/GraphicsParamKeeper.hpp"

namespace {

/**
 * Looks up a render info entry of a material by name.
 * @param pMaterial Material.
 * @param pName Name of the render info.
 * @return The render info, or nullptr if it does not exist.
 */
const nn::g3d::ResRenderInfo* findRenderInfo(const nn::g3d::ResMaterial* pMaterial,
                                             const char* pName) {
    const nn::util::ResDic* dictionary = pMaterial->ToData().pRenderInfoDic.Get();

    if (dictionary == nullptr) {
        return nullptr;
    }

    s32 index = dictionary->FindIndex(pName);

    if (index == nn::util::ResDic::Npos) {
        return nullptr;
    }

    return &pMaterial->ToData().pRenderInfoArray.Get()[index];
}

}  // namespace

namespace sead {

/**
 * Sorts the model light parameters by preset name.
 */
template <>
NOINLINE inline void PtrArray<al::ModelLightParam>::sort() {
    if (size() < 2) {
        return;
    }

    shakerSort_<al::ModelLightParam>(
        [](const al::ModelLightParam* pA, const al::ModelLightParam* pB) -> s32 {
            if (*pA < *pB) {
                return -1;
            }

            if (*pB < *pA) {
                return 1;
            }

            return 0;
        });
}

}  // namespace sead

namespace al {

/**
 * Creates a model light parameter.
 * @param pPresetName Default light preset name.
 */
ModelLightParam::ModelLightParam(const char* pPresetName)
    : mLightPresetName(StringTmp<64>(pPresetName), "LightPresetName", "ライトプリセット名",
                       &mParamObj),
      mModelLightName(StringTmp<64>("無効"), "ModelLightName", "モデルライト名", &mParamObj),
      mModelLightColorScale(sead::Color4f::cWhite, "ModelLightColorScale",
                            "カラースケール（シーンモデルライト以外にも適用されます）",
                            &mParamObj) {}

/**
 * Gets the index of the model light texture selected by the model light name.
 * @return The texture index, or -1 if the name is unknown.
 */
s32 ModelLightParam::getModelLightTexIndex() const {
    if (isEqualString("0:上が明るいリム（お化け）", mModelLightName->cstr())) {
        return 0;
    }

    if (isEqualString("1:リム", mModelLightName->cstr())) {
        return 1;
    }

    if (isEqualString("2:縦線（土管）", mModelLightName->cstr())) {
        return 2;
    }

    if (isEqualString("3:リム＋スフィアマップ（ブロック）", mModelLightName->cstr())) {
        return 3;
    }

    if (isEqualString("4:暗めスフィアマップ（パネル）", mModelLightName->cstr())) {
        return 4;
    }

    if (isEqualString("5:くっきり下明るめスフィアマップ（鉱石ブロック）", mModelLightName->cstr())) {
        return 5;
    }

    if (isEqualString("6:ぼんやりリム（妖精）", mModelLightName->cstr())) {
        return 6;
    }

    if (isEqualString("7:下が暗いリム＋真中弱（汎用木）", mModelLightName->cstr())) {
        return 7;
    }

    if (isEqualString("8:上部横線（ルート土管外側）", mModelLightName->cstr())) {
        return 8;
    }

    if (isEqualString("9:中央縦", mModelLightName->cstr())) {
        return 9;
    }

    if (isEqualString("10:フチと中央", mModelLightName->cstr())) {
        return 10;
    }

    if (isEqualString("11:中央横線（横土管）", mModelLightName->cstr())) {
        return 11;
    }

    if (isEqualString("12:強リム＋中央強", mModelLightName->cstr())) {
        return 12;
    }

    if (isEqualString("13:弱リム＋上ライト（広葉樹）", mModelLightName->cstr())) {
        return 13;
    }

    if (isEqualString("14:横が明るいリム(スロットブロック)", mModelLightName->cstr())) {
        return 14;
    }

    if (isEqualString("15:くっきりスフィアマップ（ボスブンレツ）", mModelLightName->cstr())) {
        return 15;
    }

    if (isEqualString("16:細いリム", mModelLightName->cstr())) {
        return 16;
    }

    if (isEqualString("17:シーン晴天", mModelLightName->cstr())) {
        return 17;
    }

    if (isEqualString("18:InkObj", mModelLightName->cstr())) {
        return 18;
    }

    if (isEqualString("19:InkDarkBowser", mModelLightName->cstr())) {
        return 19;
    }

    if (isEqualString("20:InkOcean", mModelLightName->cstr())) {
        return 20;
    }

    if (isEqualString("21:InkOceanDisaster", mModelLightName->cstr())) {
        return 21;
    }

    if (isEqualString("22:Waterfall", mModelLightName->cstr())) {
        return 22;
    }

    return -1;
}

/**
 * Creates the model light director.
 * @param pInfo Graphics system info.
 */
ModelLightDirector::ModelLightDirector(GraphicsSystemInfo* pInfo)
    : mTextureInfoArray(new TextureInfoArray()),
      mParamFilePath(new GraphicsParamFilePath("SceneModelLightParam", "agl_scene_model_light")),
      mParamNum(0, "SceneModelLightParamNum", "シーンモデルライトパラメータ数", &mParamObj) {
    mGraphicsSystemInfo = pInfo;
}

/**
 * Destroys the model light director and frees the model light textures.
 */
ModelLightDirector::~ModelLightDirector() {
    if (mParamFilePath != nullptr) {
        delete mParamFilePath;
        mParamFilePath = nullptr;
    }

    TextureInfoArray* textureInfoArray = mTextureInfoArray;

    while (!textureInfoArray->isEmpty()) {
        freeTextureInfo(textureInfoArray->popBack());
    }

    textureInfoArray->freeBuffer();
}

/**
 * Loads the scene model light presets and the model light textures.
 */
void ModelLightDirector::endInit() {
    mParamIO.addObj(&mParamObj, "SceneModelLightParam");

    StringTmp<256> archiveName("ObjectData/SceneModelLightParam");
    Resource* resource = nullptr;

    if (isExistArchive(archiveName)) {
        resource = findOrCreateResource(archiveName, nullptr);
    }

    StringTmp<256> path;
    mParamFilePath->makeBinaryPath(&path);
    const void* file = nullptr;

    if (resource != nullptr && resource->isExistFile(path)) {
        file = resource->getOtherFile(path, nullptr);
        mParamIO.applyResParameterArchive(agl::utl::ResParameterArchive(file));
    }

    s32 paramNum = *mParamNum;

    if (paramNum > 0) {
        mParams.allocBuffer(paramNum, nullptr);

        for (s32 i = 0; i < paramNum; i++) {
            mParams.pushBack(new ModelLightParam("NoData"));
        }

        mParams.sort();

        for (s32 i = 0; i < paramNum; i++) {
            mParamList.addObj(mParams[i]->getParamObj(),
                              StringTmp<128>("SceneModelLightParam%02d", i).cstr());
        }

        mParamIO.addList(&mParamList, "SceneModelLightParamList");

        if (resource != nullptr && resource->isExistFile(path)) {
            mParamIO.applyResParameterArchive(agl::utl::ResParameterArchive(file));
        }

        mParamList.clearObj();
        s32 uniqueNum = 0;

        for (s32 i = 0; i < paramNum; i++) {
            bool isDuplicate = false;

            for (s32 j = 0; j < i; j++) {
                if (isEqualString(mParams[j]->getPresetName(), mParams[i]->getPresetName())) {
                    isDuplicate = true;
                    break;
                }
            }

            if (!isDuplicate) {
                mParamList.addObj(mParams[i]->getParamObj(),
                                  StringTmp<128>("SceneModelLightParam%02d", uniqueNum++).cstr());
            }
        }

        *mParamNum = uniqueNum;
    }

    loadTextureInfoArray(mTextureInfoArray, "ObjectData/TextureModelSpecular",
                         "TextureModelSpecular.bfres", "ModelSpecularEnvMap", false);
}

/**
 * Checks if a material uses the model light preset of the scene.
 * @param pMaterial Material.
 * @return True if the material's model light preset is the scene preset.
 */
bool ModelLightDirector::isUsingSceneModelLightPreset(
    const nn::g3d::ResMaterial* pMaterial) const {
    return calcModelLightPresetId(pMaterial) == 999;
}

/**
 * Reads the model light preset of a material.
 * @param pMaterial Material.
 * @return The preset id, or -1 if the material has none.
 */
s32 ModelLightDirector::calcModelLightPresetId(const nn::g3d::ResMaterial* pMaterial) const {
    const nn::g3d::ResRenderInfo* renderInfo = findRenderInfo(pMaterial, "model_light_preset");

    if (renderInfo == nullptr) {
        return -1;
    }

    if (renderInfo->GetArrayLength() == 0) {
        return -1;
    }

    const char* str = renderInfo->GetString(0);
    char* end = nullptr;
    return strtol(str, &end, 0);
}

/**
 * Activates the model light texture of a material.
 * @param pMaterial Material.
 * @param pEnvTexInfo Environment texture info, unused.
 * @param isForce Unused.
 */
void ModelLightDirector::activateModelLightTexture(const nn::g3d::ResMaterial* pMaterial,
                                                   const EnvTexInfo* pEnvTexInfo,
                                                   bool isForce) const {
    if (!alModelFunction::isMaterialUsingModelLight(pMaterial)) {
        return;
    }

    s32 presetId = calcModelLightPresetId(pMaterial);

    if (presetId < -1) {
        // The result of this check is unused.
        isEqualString("KinopioGlasses", pMaterial->GetName());
        return;
    }

    activateModelLightTexture(presetId);
}

/**
 * Activates a model light texture.
 * @param index Index of the texture, clamped to the loaded textures.
 */
void ModelLightDirector::activateModelLightTexture(s32 index) const {
    s32 clampedIndex = sead::Mathi::clamp(index, 0, mTextureInfoArray->size() - 1);
    agl::TextureData* textureData = mTextureInfoArray->unsafeAt(clampedIndex)->mTextureData;
    const agl::SamplerLocation& location = getSamplerLocationModelLight();

    if (textureData == nullptr) {
        agl::utl::PrimitiveTexture::instance()
            ->getTextureSampler(agl::utl::PrimitiveTexture::cType_Black2D)
            ->activate(GameFrameworkNx::getAglDrawContext(), location, -1, false);
        return;
    }

    if (!location.isValid()) {
        return;
    }

    agl::TextureSampler sampler(*textureData);
    sampler.activate(GameFrameworkNx::getAglDrawContext(), location, -1, false);
}

/**
 * Finds the model light parameter of a light preset.
 * @param pName Name of the light preset.
 * @return The parameter, or nullptr if there is none.
 */
ModelLightParam* ModelLightDirector::tryGetModelLightParam(const char* pName) const {
    if (pName == nullptr) {
        return nullptr;
    }

    s32 paramNum = mParams.size();

    for (s32 i = 0; i < paramNum; i++) {
        ModelLightParam* param = mParams[i];

        if (isEqualString(param->getPresetName(), pName)) {
            return param;
        }
    }

    return nullptr;
}

}  // namespace al
