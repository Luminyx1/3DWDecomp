#pragma once

#include <container/seadPtrArray.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>
#include <prim/seadEnum.h>
#include <utility/aglParameter.h>
#include <utility/aglParameterObj.h>
#include "common/aglShaderEnum.h"
#include "utility/aglDynamicTextureAllocator.h"

namespace agl {
class RenderBuffer;
class ShaderProgram;
class TextureData;
class TextureSampler;
}  // namespace agl

namespace sead {
class Color4f;
}  // namespace sead

namespace al {
class FullScreenQuadModel;
class GBufferArray;
class GraphicsParamIo;
class GraphicsSystemInfo;
class LiveActor;
class Resource;
class UniformBlock;

/**
 * Render target of the far atmosphere.
 */
SEAD_ENUM(RenderType, cForward, cDeferred, cCubeMap)

/**
 * Precomputed atmospheric scattering (Bruneton) used to draw the sky and the far background.
 */
class AtmosScatter {
public:
    /**
     * Uniform block holding the per view atmosphere parameters.
     */
    struct ViewUbo {
        UniformBlock* mUniformBlock = nullptr;
    };

    /**
     * Intermediate textures that only live during the precomputation.
     */
    struct TemporaryTextures {
        TemporaryTextures(agl::utl::DynamicTextureAllocator* pAllocator)
            : mAllocator(pAllocator) {}

        ~TemporaryTextures() {
            mAllocator->free(mDeltaE);
            mAllocator->free(mDeltaSR);
            mAllocator->free(mDeltaSM);
            mAllocator->free(mDeltaJ);
        }

        void createTextures();

        agl::utl::DynamicTextureAllocator* mAllocator;
        agl::TextureData* mDeltaE = nullptr;
        agl::TextureData* mDeltaSR = nullptr;
        agl::TextureData* mDeltaSM = nullptr;
        agl::TextureData* mDeltaJ = nullptr;
    };

    AtmosScatter(GraphicsSystemInfo* pInfo, s32 viewNum, f32 sunIntensity);
    ~AtmosScatter();

    void createTextures();
    bool isPrecompute() const;
    f32 getRgKm() const;
    f32 getRtKm() const;
    void initStageResource(const Resource* pResource, const char* pStageName);
    void updateAtmosScatter();
    void preDrawGraphics();
    const agl::ShaderProgram* searchVariation(const agl::ShaderProgram* pProgram,
                                              s32 renderType) const;
    agl::ShaderMode drawPrecompute(agl::ShaderMode shaderMode) const;
    agl::ShaderMode drawFarForward(s32 viewIndex, const sead::Matrix34f& rViewMtx,
                                   const sead::Matrix44f& rProjMtx,
                                   const sead::Vector2f& rProjOffset, f32 fovy, f32 aspect,
                                   agl::ShaderMode shaderMode) const;
    agl::ShaderMode drawFar(s32 viewIndex, const sead::Matrix34f& rViewMtx,
                            const sead::Matrix44f& rProjMtx, const sead::Vector2f& rProjOffset,
                            f32 fovy, f32 aspect, RenderType renderType,
                            agl::ShaderMode shaderMode) const;
    agl::ShaderMode drawFarToCubeMap(s32 viewIndex, const sead::Matrix34f& rViewMtx,
                                     const sead::Matrix44f& rProjMtx,
                                     const sead::Vector2f& rProjOffset, f32 fovy, f32 aspect,
                                     agl::ShaderMode shaderMode) const;
    agl::ShaderMode drawFarDeferred(s32 viewIndex, GBufferArray* pGBufferArray,
                                    const sead::Matrix34f& rViewMtx,
                                    const sead::Matrix44f& rProjMtx,
                                    const sead::Vector2f& rProjOffset, f32 fovy, f32 aspect,
                                    agl::ShaderMode shaderMode) const;
    void calcInfo(sead::Vector3f* pCameraPos, sead::Vector3f* pSunDir,
                  sead::Vector3f* pLightSunDir) const;
    agl::ShaderMode drawDirLightColor(const sead::Color4f& rColor,
                                      const agl::TextureData* pTexture,
                                      agl::ShaderMode shaderMode) const;

    /**
     * Called after the parameters were applied.
     */
    virtual void onApplyParam() const {}

private:
    void calcCameraPos(sead::Vector3f* pCameraPos) const;
    void calcSunSizeCos(sead::Vector2f* pSunSizeCos) const;

    GraphicsSystemInfo* mGraphicsSystemInfo;
    void* _10 = nullptr;
    agl::ShaderProgram* mRenderEarthProgram;
    agl::ShaderProgram* mRenderDirLightColorProgram;
    agl::ShaderProgram* mTransmittanceProgram;
    agl::ShaderProgram* mIrradiance1Program;
    agl::ShaderProgram* mInscatter1Program;
    agl::ShaderProgram* mCopyInscatter1Program;
    agl::ShaderProgram* mCopyInscatterNProgram;
    agl::ShaderProgram* mCopyIrradianceProgram;
    agl::ShaderProgram* mInscatterSProgram;
    agl::ShaderProgram* mIrradianceNProgram;
    agl::ShaderProgram* mInscatterNProgram;
    agl::TextureData* mTransmittanceTex = nullptr;
    agl::TextureSampler* mTransmittanceSampler = nullptr;
    agl::TextureData* mIrradianceTex = nullptr;
    agl::TextureSampler* mIrradianceSampler = nullptr;
    agl::TextureData* mInscatterTex = nullptr;
    agl::TextureSampler* mInscatterSampler = nullptr;
    sead::PtrArray<ViewUbo> mViewUbos;
    agl::TextureSampler* mDeltaESampler = nullptr;
    agl::TextureSampler* mDeltaSRSampler = nullptr;
    agl::TextureSampler* mDeltaSMSampler = nullptr;
    agl::TextureSampler* mDeltaJSampler = nullptr;
    FullScreenQuadModel* mQuadModel;
    agl::RenderBuffer* mRenderBuffer;
    bool mIsRequestPrecompute = true;
    bool mIsPrecompute = false;
    bool mIsPreDrawn = false;
    bool mIsPreDrawnPrev = false;
    bool mIsTransmittanceNonLinear = false;
    bool mIsInscatterNonLinear = true;
    sead::Vector3f mSunAxis = sead::Vector3f::ex;
    f32 mSunXRot = 0.0f;
    sead::Vector3f mSunDir = sead::Vector3f::ey;
    f32 mSunIntensity;
    f32 mSunSizeDegree = 1.0f;
    f32 mSunSizeDegreeOuter = 0.0f;
    sead::Vector3f mCameraOffset = sead::Vector3f::zero;
    GraphicsParamIo* mParamIo;
    agl::utl::ParameterObj mParamObj;
    agl::utl::Parameter<f32> mSunYRotDegree;
    agl::utl::Parameter<f32> mSunZRotDegree;
    agl::utl::Parameter<f32> mSunXRotDegreeInit;
    agl::utl::Parameter<f32> mSunXRotSpeedDegree;
    agl::utl::Parameter<sead::Vector3f> mCameraOffsetParam;
};

static_assert(sizeof(AtmosScatter) == 0x200);
}  // namespace al

namespace AtmosScatterFunction {
al::AtmosScatter* getAtmosScatter(const al::LiveActor* pActor);
}  // namespace AtmosScatterFunction
