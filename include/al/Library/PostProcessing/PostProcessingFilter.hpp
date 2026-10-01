#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>
#include <prim/seadSafeString.h>
#include <utility/aglParameter.h>

namespace agl {
class DrawContext;
class RenderBuffer;
class TextureData;
}  // namespace agl

namespace agl::pfx {
class ColorCorrection;
}  // namespace agl::pfx

namespace agl::utl {
class IParameterIO;
class ParameterObj;
}  // namespace agl::utl

namespace nn::g3d {
class ResFile;
}

namespace sead {
class Camera;
class PerspectiveProjection;
}  // namespace sead

namespace al {
class CartoonDrawer;
class CartoonDrawParam;
class ColorClampDrawer;
class ColorClampDrawParam;
class ContoursDrawer;
class ContoursDrawParam;
class DepthOfFieldDrawer;
class DepthOfFieldParam;
class EdgeDrawerPostEffect;
class EdgeDrawPostEffectParam;
class FullScreenTriangle;
template <typename T>
class GraphicsParamRequester;
class KaleidoscopeDrawer;
class KaleidoscopeParam;
class MangaDrawer;
class MangaDrawParam;
class MetalReliefDrawer;
class MetalReliefDrawParam;
class MosaicPictureDrawer;
class MosaicPictureDrawParam;
class NoiseTextureKeeper;
class ParameterList;
class ParameterObj;
class ParameterString64;
class PencilSketchDrawer;
class PencilSketchDrawParam;
class PostProcessingFilter;
class Projection;
class Resource;
class RetroColorDrawer;
class RetroColorDrawParam;
class ScreenBlurDrawer;
class ScreenBlurDrawParam;
class ShaderHolder;
class SimpleModelEnv;
class UniformBlock;
class ViewDepthDrawer;
class ViewDepthDrawParam;
class VignettingDrawer;
class VignettingParam;

/**
 * A named set of post processing parameters loaded from the filter preset archive.
 */
class PostProcessingFilterPreset {
public:
    PostProcessingFilterPreset(const char* pName, const Resource* pResource,
                               PostProcessingFilter* pFilter);

    const char* getName() const;

    const DepthOfFieldParam* getDepthOfFieldParam() const { return mDepthOfFieldParam; }

    const VignettingParam& getVignettingParam() const { return *mVignettingParam; }

    const ViewDepthDrawParam& getViewDepthDrawParam() const { return *mViewDepthDrawParam; }

    const EdgeDrawPostEffectParam& getEdgeDrawParam() const { return *mEdgeDrawParam; }

    const CartoonDrawParam& getCartoonDrawParam() const { return *mCartoonDrawParam; }

    const RetroColorDrawParam& getRetroColorDrawParam() const { return *mRetroColorDrawParam; }

    const ScreenBlurDrawParam& getScreenBlurDrawParam() const { return *mScreenBlurDrawParam; }

    const PencilSketchDrawParam& getPencilSketchDrawParam() const {
        return *mPencilSketchDrawParam;
    }

    void* getColorCorrectionData() const { return mColorCorrectionData; }

    const ColorClampDrawParam& getColorClampDrawParam() const { return *mColorClampDrawParam; }

    const MetalReliefDrawParam& getMetalReliefDrawParam() const {
        return *mMetalReliefDrawParam;
    }

    const MosaicPictureDrawParam& getMosaicPictureDrawParam() const {
        return *mMosaicPictureDrawParam;
    }

    const ContoursDrawParam& getContoursDrawParam() const { return *mContoursDrawParam; }

    const KaleidoscopeParam& getKaleidoscopeParam() const { return *mKaleidoscopeParam; }

    const MangaDrawParam& getMangaDrawParam() const { return *mMangaDrawParam; }

private:
    PostProcessingFilter* mFilter;
    ParameterObj* mParamObj;
    ParameterString64* mNameParam;
    ParameterList* mParamList;
    agl::utl::Parameter<sead::FixedSafeString<64>> mName;
    agl::utl::ParameterObj* mAglParamObj;
    agl::utl::IParameterIO* mParamIo;
    DepthOfFieldParam* mDepthOfFieldParam = nullptr;
    VignettingParam* mVignettingParam;
    ViewDepthDrawParam* mViewDepthDrawParam = nullptr;
    EdgeDrawPostEffectParam* mEdgeDrawParam = nullptr;
    CartoonDrawParam* mCartoonDrawParam = nullptr;
    RetroColorDrawParam* mRetroColorDrawParam = nullptr;
    ScreenBlurDrawParam* mScreenBlurDrawParam = nullptr;
    PencilSketchDrawParam* mPencilSketchDrawParam = nullptr;
    void* mColorCorrectionData = nullptr;
    ColorClampDrawParam* mColorClampDrawParam = nullptr;
    MetalReliefDrawParam* mMetalReliefDrawParam = nullptr;
    MosaicPictureDrawParam* mMosaicPictureDrawParam = nullptr;
    ContoursDrawParam* mContoursDrawParam = nullptr;
    KaleidoscopeParam* mKaleidoscopeParam = nullptr;
    MangaDrawParam* mMangaDrawParam = nullptr;
};

static_assert(sizeof(PostProcessingFilterPreset) == 0x118);

/**
 * Applies the screen space filters of the selected preset (edge, cartoon, manga, ...).
 */
class PostProcessingFilter {
public:
    using PresetArray = sead::PtrArray<PostProcessingFilterPreset>;
    using ColorCorrectionRequester = GraphicsParamRequester<agl::pfx::ColorCorrection>;

    PostProcessingFilter(ShaderHolder* pShaderHolder, NoiseTextureKeeper* pNoiseTextureKeeper,
                         UniformBlock* pUniformBlock, const FullScreenTriangle* pTriangle);
    ~PostProcessingFilter();

    void incrementPreset();
    void decrementPreset();
    void resetPreset();
    void setPresetByName(const char* pName);
    void initProjectResource();
    void endInit();
    void clearRequest();
    void update();
    PostProcessingFilterPreset* findPreset(s32 index) const;
    void updateViewGpu(s32 index, const sead::Camera* pCamera, const Projection* pProjection);
    void updateViewGpu(s32 index, const sead::Camera* pCamera,
                       const sead::PerspectiveProjection* pProjection);
    void drawFilter(agl::DrawContext* pDrawContext, s32 index, SimpleModelEnv* pModelEnv,
                    const agl::RenderBuffer& rRenderBuffer, const agl::TextureData& rDepth,
                    const agl::TextureData& rLinearDepth, const agl::TextureData* pNormal,
                    const agl::TextureData& rBaseColor, const agl::TextureData& rColor,
                    const sead::Camera& rCamera, const Projection& rProjection, f32 near, f32 far,
                    bool isLinearDepthReady) const;
    void drawFilter(agl::DrawContext* pDrawContext, s32 index, SimpleModelEnv* pModelEnv,
                    const agl::RenderBuffer& rRenderBuffer, const agl::TextureData& rDepth,
                    const agl::TextureData& rLinearDepth, const agl::TextureData* pNormal,
                    const agl::TextureData& rBaseColor, const agl::TextureData& rColor,
                    const sead::Camera& rCamera, const sead::PerspectiveProjection& rProjection,
                    f32 near, f32 far, bool isLinearDepthReady) const;

    void validate() { mIsValid = true; }

    void invalidate() { mIsValid = false; }

    bool isValid() const { return mIsValid; }

    s32 getPresetId() const { return mPresetId; }

    s32 getPresetNum() const { return mPresets.size(); }

private:
    bool mIsValid = false;
    DepthOfFieldDrawer* mDepthOfFieldDrawer = nullptr;
    ColorCorrectionRequester* mColorCorrectionRequester = nullptr;
    ViewDepthDrawer* mViewDepthDrawer = nullptr;
    VignettingDrawer* mVignettingDrawer = nullptr;
    EdgeDrawerPostEffect* mEdgeDrawer = nullptr;
    CartoonDrawer* mCartoonDrawer = nullptr;
    RetroColorDrawer* mRetroColorDrawer = nullptr;
    ScreenBlurDrawer* mScreenBlurDrawer = nullptr;
    PencilSketchDrawer* mPencilSketchDrawer = nullptr;
    ColorClampDrawer* mColorClampDrawer = nullptr;
    MetalReliefDrawer* mMetalReliefDrawer = nullptr;
    MosaicPictureDrawer* mMosaicPictureDrawer = nullptr;
    ContoursDrawer* mContoursDrawer = nullptr;
    KaleidoscopeDrawer* mKaleidoscopeDrawer = nullptr;
    MangaDrawer* mMangaDrawer = nullptr;
    PresetArray mPresets;
    s32 mPresetId = 0;
    const PostProcessingFilterPreset* mLastPreset = nullptr;
    nn::g3d::ResFile* mResFile = nullptr;
    const FullScreenTriangle* mFullScreenTriangle;
};

static_assert(sizeof(PostProcessingFilter) == 0xb0);

}  // namespace al
