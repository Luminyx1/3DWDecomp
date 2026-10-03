#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>
#include <gfx/seadColor.h>
#include <prim/seadSafeString.h>
#include "utility/aglParameter.h"
#include "utility/aglParameterIO.h"
#include "utility/aglParameterList.h"
#include "utility/aglParameterObj.h"

namespace nn::g3d {
class ResMaterial;
}

namespace al {
class EnvTexInfo;
class GraphicsParamFilePath;
class GraphicsSystemInfo;
class TextureInfoArray;

/**
 * @brief A named model light preset: which model light texture a light preset name uses.
 */
class ModelLightParam {
public:
    ModelLightParam(const char* pPresetName);

    s32 getModelLightTexIndex() const;

    const char* getPresetName() const { return mLightPresetName->cstr(); }

    agl::utl::IParameterObj* getParamObj() { return &mParamObj; }

    bool operator<(const ModelLightParam& rOther) const {
        return *mLightPresetName < *rOther.mLightPresetName;
    }

private:
    agl::utl::ParameterObj mParamObj;
    agl::utl::Parameter<sead::FixedSafeString<64>> mLightPresetName;
    agl::utl::Parameter<sead::FixedSafeString<64>> mModelLightName;
    agl::utl::Parameter<sead::Color4f> mModelLightColorScale;
};

static_assert(sizeof(ModelLightParam) == 0x138);

/**
 * @brief Holds the scene model light presets and the model light (specular) textures.
 */
class ModelLightDirector {
public:
    ModelLightDirector(GraphicsSystemInfo* pInfo);
    ~ModelLightDirector();

    void endInit();
    bool isUsingSceneModelLightPreset(const nn::g3d::ResMaterial* pMaterial) const;
    s32 calcModelLightPresetId(const nn::g3d::ResMaterial* pMaterial) const;
    void activateModelLightTexture(const nn::g3d::ResMaterial* pMaterial,
                                   const EnvTexInfo* pEnvTexInfo, bool isForce) const;
    void activateModelLightTexture(s32 index) const;
    ModelLightParam* tryGetModelLightParam(const char* pName) const;

private:
    GraphicsSystemInfo* mGraphicsSystemInfo;
    TextureInfoArray* mTextureInfoArray;
    sead::PtrArray<ModelLightParam> mParams;
    GraphicsParamFilePath* mParamFilePath;
    agl::utl::IParameterIO mParamIO;
    agl::utl::ParameterObj mParamObj;
    agl::utl::ParameterList mParamList;
    agl::utl::Parameter<s32> mParamNum;
};

static_assert(sizeof(ModelLightDirector) == 0x330);

}  // namespace al
