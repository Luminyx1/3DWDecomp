#pragma once

#include <basis/seadTypes.h>
#include <common/aglRenderTarget.h>
#include <common/aglTextureSampler.h>
#include <container/seadSafeArray.h>

namespace agl {
class SamplerLocation;
class TextureData;

namespace lght {
class LightPrePass;
}  // namespace lght
}  // namespace agl

namespace nn::g3d {
class MaterialObj;
}  // namespace nn::g3d

namespace sead {
class GraphicsContextMRT;
class LookAtCamera;
class PerspectiveProjection;
}  // namespace sead

namespace al {
class GraphicsSystemInfo;

/**
 * @brief One G-buffer texture with its render target and samplers.
 */
struct GBuffer {
    agl::TextureData* mTexture = nullptr;
    agl::RenderTargetColor mRenderTarget;
    agl::TextureSampler mSampler;
    agl::TextureSampler mNearestSampler;
    agl::TextureFormat mFormat = agl::TextureFormat(0);
    bool mIsClear = false;
};

static_assert(sizeof(GBuffer) == 0x468);

/**
 * @brief The deferred rendering G-buffers: albedo, view normal, view depth and light buffer.
 */
class GBufferArray {
public:
    enum Index {
        cIndex_Albedo,
        cIndex_NrmView,
        cIndex_DepthView,
        cIndex_LightBuffer,
        cIndex_Num
    };

    GBufferArray(const agl::RenderTargetDepth* pDepthTarget,
                 const GraphicsSystemInfo* pGraphicsSystemInfo, bool isHighPrecisionDepth,
                 bool isClearDepthView);
    ~GBufferArray();

    void freeGBuf(s32 index);
    void allocGBuffer(s32 subIndex);
    void clearGBuffer();
    void createLightBufferAndCalcContext(
        agl::lght::LightPrePass* pLightPrePass, s32 view, s32 width, s32 height,
        const sead::LookAtCamera& rCamera, const sead::PerspectiveProjection& rProjection,
        bool isUseMultiTarget);
    const GBuffer* getGBufAlbedo() const;
    const GBuffer* getGBufNrmView() const;
    const GBuffer* getGBufDepthView() const;
    const GBuffer* getGBufLightBuffer() const;
    agl::TextureData* getGBufAlbedoTex() const;
    agl::TextureData* getGBufNrmViewTex() const;
    agl::TextureData* getGBufDepthViewTex() const;
    agl::TextureData* getGBufLightBufferTex() const;
    void activateSamplerAlbedo(const agl::SamplerLocation& rLocation) const;
    void activateSamplerNearestAlbedo(const agl::SamplerLocation& rLocation) const;
    void activateSamplerNrmView(const agl::SamplerLocation& rLocation) const;
    void activateSamplerDepthView(const agl::SamplerLocation& rLocation) const;
    void activateSamplerNearestDepthView(const agl::SamplerLocation& rLocation) const;
    void activateSamplerLightBuffer(const agl::SamplerLocation& rLocation) const;
    static void setStencilTest(sead::GraphicsContextMRT* pContext);
    void freeGBufAlbedo();
    void freeGBufNrmView();
    void freeGBufDepthView();
    void bindRenderBuffer(s32 num);
    void bindRenderBufferLightBuf();
    static void setContextMRT(sead::GraphicsContextMRT* pContext);
    static void setContextMRTXlu(sead::GraphicsContextMRT* pContext);
    static void setContextMRTXluNrm(sead::GraphicsContextMRT* pContext);
    static void setContextMRTCustom(sead::GraphicsContextMRT* pContext,
                                    const nn::g3d::MaterialObj* pMaterial, bool isNoNrm);
    static void setContextMRTAlphaMask(sead::GraphicsContextMRT* pContext);
    static void setContextMRTMiiFaceXlu(sead::GraphicsContextMRT* pContext);
    static void setContextMRTOnlyDepth(sead::GraphicsContextMRT* pContext);
    void bindRenderBufferAndContextMRT();

private:
    sead::SafeArray<GBuffer, cIndex_Num> mGBuffers;
    const GraphicsSystemInfo* mGraphicsSystemInfo;
    const agl::RenderTargetDepth* mDepthTarget;
    bool mIsHighPrecisionDepth;
    bool mIsClearDepthView;
};

static_assert(sizeof(GBufferArray) == 0x11b8);

void setContextMRTAlphaMask(sead::GraphicsContextMRT* pContext);

}  // namespace al
