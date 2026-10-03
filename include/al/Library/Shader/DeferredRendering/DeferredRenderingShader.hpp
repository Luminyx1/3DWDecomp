#pragma once

#include <basis/seadTypes.h>
#include <common/aglGPUMemAddr.h>
#include <common/aglShaderLocation.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>
#include <prim/seadEnum.h>
#include <prim/seadSafeString.h>

namespace agl {
class VertexAttribute;
class VertexBuffer;
}  // namespace agl

namespace nn::g3d {
class ResShadingModel;
class ResShaderProgram;
class ShadingModelObj;
class ShaderSelector;
}  // namespace nn::g3d

namespace al {
class GpuMemAllocator;
class UniformBlock;

/**
 * @brief Picks and sets up the deferred rendering material shader variation from option values.
 */
class DeferredRenderingShader {
public:
    // The enumerator names are not known; the values are the shader option values.
    SEAD_ENUM(ERenderType, cRenderType0, cRenderType1, cRenderType2, cRenderType3)
    SEAD_ENUM(EAlbedoType, cAlbedoType0, cAlbedoType1, cAlbedoType2, cAlbedoType3)
    SEAD_ENUM(ERefractionType, cRefractionType0, cRefractionType1, cRefractionType2)
    SEAD_ENUM(ESpecularMaskType, cSpecularMaskType0, cSpecularMaskType1, cSpecularMaskType2)
    SEAD_ENUM(EModelLightPresetType, cModelLightPresetType0, cModelLightPresetType1)
    SEAD_ENUM(EVtxColorType, cVtxColorType0, cVtxColorType1, cVtxColorType2)
    SEAD_ENUM(EEmissionType, cEmissionType0, cEmissionType1, cEmissionType2)
    SEAD_ENUM(EAlphaMaskType, cAlphaMaskType0, cAlphaMaskType1, cAlphaMaskType2)

    static constexpr s32 cOptionNum = 13;

    DeferredRenderingShader(GpuMemAllocator* pAllocator, bool isUnused);

    void setRenderType(ERenderType type);
    void setAlbedoType(EAlbedoType type);
    void setRefractionType(ERefractionType type);
    void setMetallicLuster(bool isMetallicLuster);
    void setEnableSkinning(bool isEnableSkinning);
    void setNormalMapNum(u32 num);
    void setSpecularType(ESpecularMaskType type);
    void setApplyAlbedoToSpecularPath(bool isApply);
    void setApplyIrradiancePerFragment(bool isApply);
    void setModelLightPresetType(EModelLightPresetType type);
    void setVtxColorType(EVtxColorType type);
    void setEmissionType(EEmissionType type);
    void setAlphaMaskType(EAlphaMaskType type);
    void setup(bool isAlphaMask, bool isMiiFace);
    void activate();

    const nn::g3d::ResShadingModel* getShadingModel() const { return mShadingModel; }

    const nn::g3d::ResShaderProgram* getShaderProgram() const { return mShaderProgram; }

private:
    char** mOptionValueBuffers = nullptr;
    sead::BufferedSafeString** mOptionValues = nullptr;
    void* _10 = nullptr;
    nn::g3d::ResShadingModel* mShadingModel = nullptr;
    const nn::g3d::ResShaderProgram* mShaderProgram = nullptr;
    nn::g3d::ShadingModelObj* mShadingModelObj = nullptr;
    nn::g3d::ShaderSelector* mShaderSelector = nullptr;
    GpuMemAllocator* mAllocator;
    agl::GPUMemAddr<u8> mOptionBlockAddr;
};

static_assert(sizeof(DeferredRenderingShader) == 0x58);

/**
 * @brief Ring of vertex attributes binding the vertex buffers to a deferred rendering shader.
 */
class DeferredRenderingFetchShader {
public:
    enum EVertexBufferType : u32 {
        cVertexBufferType_Position = 0,
        cVertexBufferType_Normal = 1,
        cVertexBufferType_Weight = 2,
        cVertexBufferType_Index = 3,
        cVertexBufferType_Uv0 = 4,
        cVertexBufferType_Uv4 = 5,
        cVertexBufferType_Uv8 = 6,
        cVertexBufferType_Color0 = 7,
        cVertexBufferType_Tangent0 = 8,
        cVertexBufferType_Uv2 = 9,
        cVertexBufferType_Uv5 = 10,
        cVertexBufferType_Uv3 = 11,
        cVertexBufferType_Uv6 = 12,
        cVertexBufferType_Uv7 = 13,
        cVertexBufferType_Uv10 = 14,
        cVertexBufferType_Uv9 = 15,
        cVertexBufferType_Num = 16,
    };

    DeferredRenderingFetchShader(s32 bufferNum);
    virtual ~DeferredRenderingFetchShader() = default;

    void setVertexBuffer(EVertexBufferType type, agl::VertexBuffer* pVertexBuffer, s32 index);
    void setup(DeferredRenderingShader* pShader);
    void activate();
    void swap();

private:
    u32 mBufferNum;
    agl::VertexBuffer** mVertexBuffers[cVertexBufferType_Num];
    agl::VertexAttribute** mAttributes;
    s32 mCurrentIndex;
};

static_assert(sizeof(DeferredRenderingFetchShader) == 0xa0);

/**
 * @brief Ring of "material" uniform blocks (texture matrices, colors and material parameters)
 * for the deferred rendering shader.
 */
class DeferredRenderingMatUbo {
public:
    DeferredRenderingMatUbo(const DeferredRenderingShader* pShader, s32 uboNum);
    virtual ~DeferredRenderingMatUbo() = default;

    void setTexMtxAlbedo0(const sead::Matrix22f* pMtx, const sead::Vector2f& rTrans);
    void setTexMtxNormal(const sead::Matrix22f* pMtx, const sead::Vector2f& rTrans);
    void setTexMtxSpecular(const sead::Matrix22f* pMtx, const sead::Vector2f& rTrans);
    void setTexMtxEmission(const sead::Matrix22f* pMtx, const sead::Vector2f& rTrans);
    void setTexMtxNormal2nd(const sead::Matrix22f* pMtx, const sead::Vector2f& rTrans);
    void setSelectiveReflection(const sead::Vector4f& rValue);
    void setDiffuseColor(const sead::Vector4f& rColor);
    void setModelLightColor(const sead::Vector4f& rColor);
    void setEmissionColor(const sead::Vector4f& rColor);
    void setWrapCoef(const sead::Vector2f& rCoef);
    void setRoughness(f32 roughness);
    void setEta(f32 eta);
    void setAbsorbRefraction(const sead::Vector4f& rValue);
    void setNoiseScale(f32 scale);
    void swap();
    void activate();

private:
    /**
     * @brief A pending value and the number of ring blocks it still has to be written to.
     */
    template <typename T>
    struct Param {
        T value;
        u32 updateCount;
    };

    struct TexMtx {
        sead::Matrix22f mtx;
        sead::Vector2f trans;
        u32 updateCount;
    };

    void writeTexMtx(s32 memberIndex, const TexMtx& rTexMtx);

    u32 mUboNum;
    s32 mCurrentIndex = 0;
    agl::UniformBlockLocation mLocation;
    UniformBlock** mUbos;
    TexMtx mTexMtxAlbedo0;
    TexMtx mTexMtxNormal;
    TexMtx mTexMtxSpecular;
    TexMtx mTexMtxEmission;
    TexMtx mTexMtxNormal2nd;
    Param<sead::Vector4f> mSelectiveReflection;
    Param<sead::Vector4f> mDiffuseColor;
    Param<sead::Vector4f> mModelLightColor;
    Param<sead::Vector4f> mEmissionColor;
    Param<sead::Vector2f> mWrapCoef;
    Param<f32> mRoughness;
    Param<f32> mEta;
    Param<sead::Vector4f> mAbsorbRefraction;
    Param<f32> mNoiseScale;
};

static_assert(sizeof(DeferredRenderingMatUbo) == 0x148);

}  // namespace al
