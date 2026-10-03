#include "Library/Shader/DeferredRendering/DeferredRenderingShader.hpp"

#include <common/aglVertexAttribute.h>
#include <driver/aglGraphicsDriverMgr.h>
#include <g3d/aglShaderUtilG3D.h>
#include <nn/g3d/g3d_ResShader.h>

#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/Shader/Block/UniformBlock.hpp"
#include "Library/Shader/Block/UniformBlockUtil.hpp"
#include "Library/Shader/DeferredRendering/PeripheryRendering.hpp"
#include "Library/Shader/ForwardRendering/ShaderHolder.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Draw/GpuMemAllocator.hpp"

namespace {
const char* const cDefaultOptionValues[al::DeferredRenderingShader::cOptionNum] = {
    "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0",
};

// Not const: the variation search code does not assume that the names are non-null.
const char* cOptionNames[al::DeferredRenderingShader::cOptionNum] = {
    "cRenderType",
    "cIsEnableNormalMap",
    "cAlbedoType",
    "cRefractionType",
    "cReflectionType",
    "cSpecularType",
    "cApplyAlbedoToSpecularPath",
    "cApplyIrradiancePerFragment",
    "cUsingModelLightPreset",
    "VtxColorType",
    "EmissionType",
    "cSkinWeightNum",
    "AlphaMaskType",
};

const al::UniformBlockLayout cMatUboLayout[] = {
    {0, agl::UniformBlock::cType_Vec4, 2},   {1, agl::UniformBlock::cType_Vec4, 2},
    {2, agl::UniformBlock::cType_Vec4, 2},   {3, agl::UniformBlock::cType_Vec4, 2},
    {4, agl::UniformBlock::cType_Vec4, 2},   {5, agl::UniformBlock::cType_Vec4, 1},
    {6, agl::UniformBlock::cType_Vec4, 1},   {7, agl::UniformBlock::cType_Vec4, 1},
    {8, agl::UniformBlock::cType_Vec4, 1},   {9, agl::UniformBlock::cType_Vec2, 1},
    {10, agl::UniformBlock::cType_Float, 1}, {11, agl::UniformBlock::cType_Float, 1},
    {12, agl::UniformBlock::cType_Vec4, 1},  {13, agl::UniformBlock::cType_Float, 1},
    {14, agl::UniformBlock::cType_Vec4, 2},  {15, agl::UniformBlock::cType_Vec4, 2},
    {16, agl::UniformBlock::cType_Vec4, 2},  {17, agl::UniformBlock::cType_Vec4, 2},
    {18, agl::UniformBlock::cType_Vec4, 2},  {19, agl::UniformBlock::cType_Vec4, 4},
};

/**
 * @brief Gets the graphics device.
 * @return Graphics device.
 */
nn::gfx::Device* getGfxDevice() {
    return static_cast<nn::gfx::Device*>(
        agl::driver::GraphicsDriverMgr::instance()->getGfxDevice());
}

/**
 * @brief Writes the shader key of one option, as a dynamic option if the shading model has one
 * with that name and as a static option otherwise.
 * @param pSelector Shader selector whose key is written.
 * @param pName Name of the option.
 * @param rValue Value of the option.
 * @return True if the option and its value were found.
 */
bool writeShaderKey(nn::g3d::ShaderSelector* pSelector, const char* pName,
                    const char* const& rValue) {
    nn::g3d::ShadingModelObj* shadingModelObj = pSelector->GetShadingModel();
    const nn::g3d::ResShadingModel* resShadingModel = shadingModelObj->GetResource();
    s32 optionIndex = resShadingModel->FindDynamicOptionIndex(pName);

    if (optionIndex >= 0) {
        s32 choiceIndex =
            resShadingModel->GetDynamicOption(optionIndex)->FindChoiceIndex(rValue);

        if (choiceIndex < 0) {
            return false;
        }

        pSelector->WriteDynamicKey(optionIndex, choiceIndex);
        return true;
    }

    optionIndex = resShadingModel->FindStaticOptionIndex(pName);

    if (optionIndex < 0) {
        return false;
    }

    s32 choiceIndex = resShadingModel->GetStaticOption(optionIndex)->FindChoiceIndex(rValue);

    if (choiceIndex < 0) {
        return false;
    }

    shadingModelObj->WriteStaticKey(optionIndex, choiceIndex);
    return true;
}

/**
 * @brief Writes the shader keys of the given options and selects the matching variation.
 * @param pSelector Shader selector whose keys are written.
 * @param pOptionNames Names of the options.
 * @param pOptionValues Values of the options.
 * @return Selected shader program, or nullptr if an option or value was not found.
 */
const nn::g3d::ResShaderProgram* selectVariation(nn::g3d::ShaderSelector* pSelector,
                                                 const char* const* pOptionNames,
                                                 const char* const* pOptionValues) {
    for (s32 i = 0; i < al::DeferredRenderingShader::cOptionNum; i++) {
        if (!writeShaderKey(pSelector, pOptionNames[i], pOptionValues[i])) {
            return nullptr;
        }
    }

    pSelector->GetShadingModel()->UpdateShaderRange();
    pSelector->UpdateVariation(getGfxDevice());
    pSelector->GetShadingModel()->CalculateOptionBlock(0);
    return pSelector->GetProgram();
}
}  // namespace

namespace al {

/**
 * @brief Allocates the option value strings and sets every option to its default value.
 * @param pAllocator GPU memory allocator for the uber shader option block.
 * @param isUnused Unused.
 */
DeferredRenderingShader::DeferredRenderingShader(GpuMemAllocator* pAllocator, bool isUnused)
    : mAllocator(pAllocator) {
    mOptionValueBuffers = new char*[cOptionNum];
    mOptionValues = new sead::BufferedSafeString*[cOptionNum];

    for (s32 i = 0; i < cOptionNum; i++) {
        mOptionValueBuffers[i] = new char[3];
        mOptionValues[i] = new sead::BufferedSafeString(mOptionValueBuffers[i], 3);
        mOptionValues[i]->format("%s", cDefaultOptionValues[i]);
    }
}

/**
 * @brief Sets the render type option.
 * @param type Render type.
 */
void DeferredRenderingShader::setRenderType(ERenderType type) {
    mOptionValues[0]->format("%d", static_cast<s32>(type));
}

/**
 * @brief Sets the albedo type option.
 * @param type Albedo type.
 */
void DeferredRenderingShader::setAlbedoType(EAlbedoType type) {
    mOptionValues[2]->format("%d", type - 1);
}

/**
 * @brief Sets the refraction type option.
 * @param type Refraction type.
 */
void DeferredRenderingShader::setRefractionType(ERefractionType type) {
    mOptionValues[3]->format("%d", static_cast<s32>(type));
}

/**
 * @brief Sets the reflection type option from whether the material has a metallic luster.
 * @param isMetallicLuster Whether the material has a metallic luster.
 */
void DeferredRenderingShader::setMetallicLuster(bool isMetallicLuster) {
    mOptionValues[4]->format("%d", isMetallicLuster);
}

/**
 * @brief Sets the skinning option.
 * @param isEnableSkinning Whether skinning is enabled.
 */
void DeferredRenderingShader::setEnableSkinning(bool isEnableSkinning) {
    mOptionValues[11]->format("%d", isEnableSkinning);
}

/**
 * @brief Sets the normal map count option.
 * @param num Number of normal maps.
 */
void DeferredRenderingShader::setNormalMapNum(u32 num) {
    mOptionValues[1]->format("%d", num);
}

/**
 * @brief Sets the specular mask type option.
 * @param type Specular mask type.
 */
void DeferredRenderingShader::setSpecularType(ESpecularMaskType type) {
    mOptionValues[5]->format("%d", static_cast<s32>(type));
}

/**
 * @brief Sets whether the albedo is applied to the specular path.
 * @param isApply Whether the albedo is applied.
 */
void DeferredRenderingShader::setApplyAlbedoToSpecularPath(bool isApply) {
    mOptionValues[6]->format("%d", isApply);
}

/**
 * @brief Sets whether the irradiance is applied per fragment.
 * @param isApply Whether the irradiance is applied per fragment.
 */
void DeferredRenderingShader::setApplyIrradiancePerFragment(bool isApply) {
    mOptionValues[7]->format("%d", isApply);
}

/**
 * @brief Sets the model light preset type option.
 * @param type Model light preset type.
 */
void DeferredRenderingShader::setModelLightPresetType(EModelLightPresetType type) {
    mOptionValues[8]->format("%d", static_cast<s32>(type));
}

/**
 * @brief Sets the vertex color type option.
 * @param type Vertex color type.
 */
void DeferredRenderingShader::setVtxColorType(EVtxColorType type) {
    mOptionValues[9]->format("%d", static_cast<s32>(type));
}

/**
 * @brief Sets the emission type option.
 * @param type Emission type.
 */
void DeferredRenderingShader::setEmissionType(EEmissionType type) {
    mOptionValues[10]->format("%d", static_cast<s32>(type));
}

/**
 * @brief Sets the alpha mask type option.
 * @param type Alpha mask type.
 */
void DeferredRenderingShader::setAlphaMaskType(EAlphaMaskType type) {
    mOptionValues[12]->format("%d", static_cast<s32>(type));
}

/**
 * @brief Searches the shader variation matching the current option values, building it from
 * the uber shading model if no prebuilt variation exists.
 * @param isAlphaMask Whether to use the alpha mask shading model.
 * @param isMiiFace Whether to use the Mii face shading model.
 */
void DeferredRenderingShader::setup(bool isAlphaMask, bool isMiiFace) {
    ShaderHolder* holder = ShaderHolder::sInstance;
    const char* name = isAlphaMask ? "RenderMaterialAlphaMask" : "RenderMaterial";
    mShadingModel = holder->getShadingModel(isMiiFace ? "RenderMaterialMiiFace" : name);
    mShaderProgram = trySearchVariation(mShadingModel, cOptionNum, cOptionNames,
                                        mOptionValueBuffers);

    if (mShaderProgram != nullptr) {
        return;
    }

    mShadingModelObj = new nn::g3d::ShadingModelObj();
    mShadingModel = ShaderHolder::sInstance->getShadingModelUber("RenderMaterial");
    nn::g3d::ShadingModelObj::Builder builder(mShadingModel);
    builder.CalculateMemorySize();
    size_t memorySize = builder.GetWorkMemorySize();
    builder.Build(mShadingModelObj, new (8) u8[memorySize], memorySize);

    size_t blockBufferSize = mShadingModelObj->CalculateBlockBufferSize(getGfxDevice());
    s32 alignment = mShadingModelObj->GetBlockBufferAlignment(getGfxDevice());
    mOptionBlockAddr =
        mAllocator->allocMemory("Deferred Rendering Context", blockBufferSize, alignment);
    nn::gfx::MemoryPool* memoryPool = mAllocator->allocMemoryPool();
    mShadingModelObj->SetupBlockBuffer(getGfxDevice(), memoryPool,
                                       mOptionBlockAddr.getByteOffset(), blockBufferSize);
    mAllocator->registerShadingModelObj(mShadingModelObj);

    mShaderSelector = new nn::g3d::ShaderSelector();
    {
        nn::g3d::ShaderSelector::Builder selectorBuilder(mShadingModelObj);
        selectorBuilder.CalculateMemorySize();
        size_t selectorMemorySize = selectorBuilder.GetWorkMemorySize();
        selectorBuilder.Build(mShaderSelector, new (8) u8[selectorMemorySize],
                              selectorMemorySize);
    }

    mShaderProgram = selectVariation(mShaderSelector, cOptionNames, mOptionValueBuffers);
}

/**
 * @brief Loads the selected shader program into the command buffer.
 */
void DeferredRenderingShader::activate() {
    if (mShaderProgram != nullptr) {
        mShaderProgram->Load(GameFrameworkNx::getAglDrawContext()->getCommandBuffer());
    }
}

/**
 * @brief Allocates the vertex buffer tables and the vertex attribute ring.
 * @param bufferNum Number of buffers in the ring.
 */
DeferredRenderingFetchShader::DeferredRenderingFetchShader(s32 bufferNum)
    : mBufferNum(bufferNum), mAttributes(new agl::VertexAttribute*[bufferNum]),
      mCurrentIndex(0) {
    for (s32 type = 0; type < cVertexBufferType_Num; type++) {
        mVertexBuffers[type] = new agl::VertexBuffer*[bufferNum];
    }

    for (s32 i = 0; i < bufferNum; i++) {
        for (s32 type = 0; type < cVertexBufferType_Num; type++) {
            mVertexBuffers[type][i] = nullptr;
        }

        mAttributes[i] = new agl::VertexAttribute();
    }
}

/**
 * @brief Sets the vertex buffer of one attribute type for one buffer of the ring.
 * @param type Vertex buffer type.
 * @param pVertexBuffer Vertex buffer.
 * @param index Index of the buffer in the ring.
 */
void DeferredRenderingFetchShader::setVertexBuffer(EVertexBufferType type,
                                                   agl::VertexBuffer* pVertexBuffer, s32 index) {
    mVertexBuffers[type][index] = pVertexBuffer;
}

/**
 * @brief Creates the vertex attributes for the attributes used by the shader.
 * @param pShader Deferred rendering shader.
 */
void DeferredRenderingFetchShader::setup(DeferredRenderingShader* pShader) {
    const nn::g3d::ResShadingModel* shadingModel = pShader->getShadingModel();
    s32 attribNum = 0;

    for (s32 i = 0; i < shadingModel->GetAttribCount(); i++) {
        if (shadingModel->GetAttrib(i)->location != -1) {
            attribNum++;
        }
    }

    for (u32 i = 0; i < mBufferNum; i++) {
        mAttributes[i] = new agl::VertexAttribute();
        mAttributes[i]->create(attribNum, nullptr);
    }

    for (s32 i = 0; i < shadingModel->GetAttribCount(); i++) {
        const nn::g3d::ResAttribVarData* attrib = shadingModel->GetAttrib(i);

        if (attrib->location == -1) {
            continue;
        }

        const char* name = shadingModel->GetAttribName(i);
        EVertexBufferType type;

        if (isEqualString("_p0", name)) {
            type = cVertexBufferType_Position;
        } else if (isEqualString("_n0", name)) {
            type = cVertexBufferType_Normal;
        } else if (isEqualString("_w0", name)) {
            type = cVertexBufferType_Weight;
        } else if (isEqualString("_i0", name)) {
            type = cVertexBufferType_Index;
        } else if (isEqualString("_u0", name)) {
            type = cVertexBufferType_Uv0;
        } else if (isEqualString("_u4", name)) {
            type = cVertexBufferType_Uv4;
        } else if (isEqualString("_u8", name)) {
            type = cVertexBufferType_Uv8;
        } else if (isEqualString("_c0", name)) {
            type = cVertexBufferType_Color0;
        } else if (isEqualString("_t0", name)) {
            type = cVertexBufferType_Tangent0;
        } else if (isEqualString("_u2", name)) {
            type = cVertexBufferType_Uv2;
        } else if (isEqualString("_u5", name)) {
            type = cVertexBufferType_Uv5;
        } else if (isEqualString("_u3", name)) {
            type = cVertexBufferType_Uv3;
        } else if (isEqualString("_u6", name)) {
            type = cVertexBufferType_Uv6;
        } else if (isEqualString("_u7", name)) {
            type = cVertexBufferType_Uv7;
        } else if (isEqualString("_u10", name)) {
            type = cVertexBufferType_Uv10;
        } else if (isEqualString("_u9", name)) {
            type = cVertexBufferType_Uv9;
        } else {
            continue;
        }

        agl::AttributeLocation location(name, attrib->location);

        for (u32 j = 0; j < mBufferNum; j++) {
            const agl::VertexBuffer* vertexBuffer = mVertexBuffers[type][j] != nullptr ?
                                                        mVertexBuffers[type][j] :
                                                        mVertexBuffers[type][0];

            if (vertexBuffer != nullptr) {
                mAttributes[j]->setVertexStream(location.getLocation(agl::cShaderType_Vertex),
                                                vertexBuffer, 0);
            }
        }
    }

    for (u32 i = 0; i < mBufferNum; i++) {
        mAttributes[i]->setUp();
    }
}

/**
 * @brief Activates the vertex attribute written last.
 */
void DeferredRenderingFetchShader::activate() {
    s32 index = (mCurrentIndex + mBufferNum - 1) % mBufferNum;
    mAttributes[index]->activate(GameFrameworkNx::getAglDrawContext());
}

/**
 * @brief Moves to the next vertex attribute of the ring.
 */
void DeferredRenderingFetchShader::swap() {
    mCurrentIndex = mCurrentIndex + 1 >= mBufferNum ? 0 : mCurrentIndex + 1;
}

/**
 * @brief Creates the uniform blocks for the material block of a deferred rendering shader.
 * @param pShader Deferred rendering shader.
 * @param uboNum Number of uniform blocks in the ring.
 */
DeferredRenderingMatUbo::DeferredRenderingMatUbo(const DeferredRenderingShader* pShader,
                                                 s32 uboNum)
    : mUboNum(uboNum), mUbos(new UniformBlock*[uboNum]),
      mTexMtxAlbedo0{sead::Matrix22f::ident, {0.0f, 0.0f}, mUboNum},
      mTexMtxNormal{sead::Matrix22f::ident, {0.0f, 0.0f}, mUboNum},
      mTexMtxSpecular{sead::Matrix22f::ident, {0.0f, 0.0f}, mUboNum},
      mTexMtxEmission{sead::Matrix22f::ident, {0.0f, 0.0f}, mUboNum},
      mTexMtxNormal2nd{sead::Matrix22f::ident, {0.0f, 0.0f}, mUboNum},
      mSelectiveReflection{{0.0f, 0.0f, 0.0f, 0.0f}, mUboNum},
      mDiffuseColor{{1.0f, 1.0f, 1.0f, 1.0f}, mUboNum},
      mModelLightColor{{0.0f, 0.0f, 0.0f, 0.0f}, mUboNum},
      mEmissionColor{{0.0f, 0.0f, 0.0f, 0.0f}, mUboNum}, mWrapCoef{{0.0f, 0.0f}, mUboNum},
      mRoughness{0.0f, mUboNum}, mEta{0.0f, mUboNum},
      mAbsorbRefraction{{0.0f, 0.0f, 0.0f, 0.0f}, mUboNum}, mNoiseScale{1.0f, mUboNum} {
    agl::g3d::ShaderUtilG3D::search(&mLocation, pShader->getShadingModel(),
                                    pShader->getShaderProgram(), "cMat");

    for (u32 i = 0; i < mUboNum; i++) {
        mUbos[i] = createUniformBlock(cMatUboLayout, 20, nullptr, 2);
    }
}

/**
 * @brief Sets the albedo 0 texture matrix.
 * @param pMtx Rotation and scale part.
 * @param rTrans Translation part.
 */
void DeferredRenderingMatUbo::setTexMtxAlbedo0(const sead::Matrix22f* pMtx,
                                               const sead::Vector2f& rTrans) {
    mTexMtxAlbedo0.updateCount = mUboNum;
    mTexMtxAlbedo0.mtx = *pMtx;
    mTexMtxAlbedo0.trans.set(rTrans);
}

/**
 * @brief Sets the normal map texture matrix.
 * @param pMtx Rotation and scale part.
 * @param rTrans Translation part.
 */
void DeferredRenderingMatUbo::setTexMtxNormal(const sead::Matrix22f* pMtx,
                                              const sead::Vector2f& rTrans) {
    mTexMtxNormal.updateCount = mUboNum;
    mTexMtxNormal.mtx = *pMtx;
    mTexMtxNormal.trans.set(rTrans);
}

/**
 * @brief Sets the specular map texture matrix.
 * @param pMtx Rotation and scale part.
 * @param rTrans Translation part.
 */
void DeferredRenderingMatUbo::setTexMtxSpecular(const sead::Matrix22f* pMtx,
                                                const sead::Vector2f& rTrans) {
    mTexMtxSpecular.updateCount = mUboNum;
    mTexMtxSpecular.mtx = *pMtx;
    mTexMtxSpecular.trans.set(rTrans);
}

/**
 * @brief Sets the emission map texture matrix.
 * @param pMtx Rotation and scale part.
 * @param rTrans Translation part.
 */
void DeferredRenderingMatUbo::setTexMtxEmission(const sead::Matrix22f* pMtx,
                                                const sead::Vector2f& rTrans) {
    mTexMtxEmission.updateCount = mUboNum;
    mTexMtxEmission.mtx = *pMtx;
    mTexMtxEmission.trans.set(rTrans);
}

/**
 * @brief Sets the second normal map texture matrix.
 * @param pMtx Rotation and scale part.
 * @param rTrans Translation part.
 */
void DeferredRenderingMatUbo::setTexMtxNormal2nd(const sead::Matrix22f* pMtx,
                                                 const sead::Vector2f& rTrans) {
    mTexMtxNormal2nd.updateCount = mUboNum;
    mTexMtxNormal2nd.mtx = *pMtx;
    mTexMtxNormal2nd.trans.set(rTrans);
}

/**
 * @brief Sets the selective reflection parameters.
 * @param rValue Selective reflection parameters.
 */
void DeferredRenderingMatUbo::setSelectiveReflection(const sead::Vector4f& rValue) {
    mSelectiveReflection.updateCount = mUboNum;
    mSelectiveReflection.value.set(rValue);
}

/**
 * @brief Sets the diffuse color.
 * @param rColor Diffuse color.
 */
void DeferredRenderingMatUbo::setDiffuseColor(const sead::Vector4f& rColor) {
    mDiffuseColor.updateCount = mUboNum;
    mDiffuseColor.value.set(rColor);
}

/**
 * @brief Sets the model light color.
 * @param rColor Model light color.
 */
void DeferredRenderingMatUbo::setModelLightColor(const sead::Vector4f& rColor) {
    mModelLightColor.updateCount = mUboNum;
    mModelLightColor.value.set(rColor);
}

/**
 * @brief Sets the emission color.
 * @param rColor Emission color.
 */
void DeferredRenderingMatUbo::setEmissionColor(const sead::Vector4f& rColor) {
    mEmissionColor.updateCount = mUboNum;
    mEmissionColor.value.set(rColor);
}

/**
 * @brief Sets the wrap lighting coefficients.
 * @param rCoef Wrap lighting coefficients.
 */
void DeferredRenderingMatUbo::setWrapCoef(const sead::Vector2f& rCoef) {
    mWrapCoef.updateCount = mUboNum;
    mWrapCoef.value.set(rCoef);
}

/**
 * @brief Sets the roughness.
 * @param roughness Roughness.
 */
void DeferredRenderingMatUbo::setRoughness(f32 roughness) {
    mRoughness.updateCount = mUboNum;
    mRoughness.value = roughness;
}

/**
 * @brief Sets the refractive index ratio.
 * @param eta Refractive index ratio.
 */
void DeferredRenderingMatUbo::setEta(f32 eta) {
    mEta.updateCount = mUboNum;
    mEta.value = eta;
}

/**
 * @brief Sets the refraction absorption parameters.
 * @param rValue Refraction absorption parameters.
 */
void DeferredRenderingMatUbo::setAbsorbRefraction(const sead::Vector4f& rValue) {
    mAbsorbRefraction.updateCount = mUboNum;
    mAbsorbRefraction.value.set(rValue);
}

/**
 * @brief Sets the noise scale.
 * @param scale Noise scale.
 */
void DeferredRenderingMatUbo::setNoiseScale(f32 scale) {
    mNoiseScale.updateCount = mUboNum;
    mNoiseScale.value = scale;
}

/**
 * @brief Writes a value to an element of a member of a uniform block.
 * @param pUbo Uniform block.
 * @param memberIndex Index of the member.
 * @param arrayIndex Index of the element in the member.
 * @param value Value.
 */
template <typename T>
static void setUboValue(const UniformBlock* pUbo, s32 memberIndex, s32 arrayIndex, T value) {
    pUbo->setData(memberIndex, &value, arrayIndex, 1);
}

/**
 * @brief Writes a texture matrix as two rows to a member of the current block.
 * @param memberIndex Index of the member.
 * @param rTexMtx Texture matrix.
 */
inline void DeferredRenderingMatUbo::writeTexMtx(s32 memberIndex, const TexMtx& rTexMtx) {
    setUboValue(mUbos[mCurrentIndex], memberIndex, 0,
                sead::Vector4f(rTexMtx.mtx.m[0][0], rTexMtx.mtx.m[0][1], rTexMtx.mtx.m[1][0],
                               rTexMtx.mtx.m[1][1]));
    setUboValue(mUbos[mCurrentIndex], memberIndex, 1,
                sead::Vector4f(rTexMtx.trans.x, rTexMtx.trans.y, 0.0f, 0.0f));
}

/**
 * @brief Writes the pending values to the current block and moves to the next one.
 */
void DeferredRenderingMatUbo::swap() {
    if (mTexMtxAlbedo0.updateCount != 0) {
        mTexMtxAlbedo0.updateCount--;
        writeTexMtx(0, mTexMtxAlbedo0);
    }

    if (mTexMtxNormal.updateCount != 0) {
        mTexMtxNormal.updateCount--;
        writeTexMtx(1, mTexMtxNormal);
    }

    if (mTexMtxSpecular.updateCount != 0) {
        mTexMtxSpecular.updateCount--;
        writeTexMtx(2, mTexMtxSpecular);
    }

    if (mTexMtxEmission.updateCount != 0) {
        mTexMtxEmission.updateCount--;
        writeTexMtx(3, mTexMtxEmission);
    }

    if (mTexMtxNormal2nd.updateCount != 0) {
        mTexMtxNormal2nd.updateCount--;
        writeTexMtx(4, mTexMtxNormal2nd);
    }

    if (mSelectiveReflection.updateCount != 0) {
        mSelectiveReflection.updateCount--;
        mUbos[mCurrentIndex]->setData(5, &mSelectiveReflection.value, 0, 1);
    }

    if (mDiffuseColor.updateCount != 0) {
        mDiffuseColor.updateCount--;
        mUbos[mCurrentIndex]->setData(6, &mDiffuseColor.value, 0, 1);
    }

    if (mModelLightColor.updateCount != 0) {
        mModelLightColor.updateCount--;
        mUbos[mCurrentIndex]->setData(7, &mModelLightColor.value, 0, 1);
    }

    if (mEmissionColor.updateCount != 0) {
        mEmissionColor.updateCount--;
        mUbos[mCurrentIndex]->setData(8, &mEmissionColor.value, 0, 1);
    }

    if (mWrapCoef.updateCount != 0) {
        mWrapCoef.updateCount--;
        mUbos[mCurrentIndex]->setData(9, &mWrapCoef.value, 0, 1);
    }

    if (mRoughness.updateCount != 0) {
        mRoughness.updateCount--;
        mUbos[mCurrentIndex]->setValue(10, mRoughness.value);
    }

    if (mEta.updateCount != 0) {
        mEta.updateCount--;
        mUbos[mCurrentIndex]->setValue(11, mEta.value);
    }

    if (mAbsorbRefraction.updateCount != 0) {
        mAbsorbRefraction.updateCount--;
        mUbos[mCurrentIndex]->setData(12, &mAbsorbRefraction.value, 0, 1);
    }

    if (mNoiseScale.updateCount != 0) {
        mNoiseScale.updateCount--;
        mUbos[mCurrentIndex]->setValue(13, mNoiseScale.value);
    }

    const UniformBlock* ubo = mUbos[mCurrentIndex];
    u32 size = ubo->getBlockSize();
    s32 offset = ubo->getCurrentBlockOffset(0);
    agl::GPUMemVoidAddr(ubo->getBuffer(), offset).flushCPUCache(size);
    mCurrentIndex = mCurrentIndex + 1 >= mUboNum ? 0 : mCurrentIndex + 1;
}

/**
 * @brief Binds the block written last.
 */
void DeferredRenderingMatUbo::activate() {
    agl::DrawContext* drawContext = GameFrameworkNx::getAglDrawContext();
    const UniformBlock* ubo = mUbos[static_cast<s32>((mCurrentIndex + mUboNum - 1) % mUboNum)];
    u64 address = nvnBufferGetAddress(ubo->getNvnBuffer()) + ubo->getCurrentBlockOffset(0);
    ubo->setUniform(drawContext, address, mLocation, 0, ubo->getBlockSize());
}

}  // namespace al
