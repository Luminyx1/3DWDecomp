#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>
#include <math/seadVector.h>
#include <nn/g3d/g3d_ViewVolume.h>

namespace agl::sdw {
    class PrimitiveOcclusion;
}

namespace sead {
    class LookAtCamera;
    class PerspectiveProjection;
}  // namespace sead

namespace al {
    class AtmosScatter;
    class AtmosScatterDrawer;
    class CubeMapDirector;
    class DirectionalLightKeeper;
    class FogDirector;
    class FullScreenTriangle;
    class GBufferArray;
    class GpuMemAllocator;
    class GraphicsStressDirector;
    class GraphicsAreaDirector;
    class LightIntensityDirector;
    class ModelLightDirector;
    class PrePassLightKeeper;
    class ShaderEnvTextureKeeper;
    class ShaderMirrorDirector;
    class ShadowDirector;
    class SkyboxDirector;
    class SSIIKeeper;
    class UniformBlock;
    class ViewRenderer;
    class LiveActorKit;
    class SceneCameraInfo;
    struct GraphicsInitArg;

    // Partial layout (3DW); only the members used so far are named.
    class ShaderCubeMapKeeper;

    class GraphicsSystemInfo {
    public:
        GraphicsSystemInfo(const char* pStageName);

        void init(const GraphicsInitArg& rArg, LiveActorKit* pKit);
        void endInit();
        void clearGraphicsRequest();
        void updateGraphics(bool isPaused);
        void preDrawGraphics(const SceneCameraInfo* pCameraInfo);
        ShaderCubeMapKeeper* getShaderCubeMapKeeper() const;
        void activateDirLitColorTex() const;
        const sead::PtrArray<UniformBlock>* getViewIndexedUboArray(const char* pName) const;

        u8 _0[0x40];
        s32 _40;
        u8 _44[0x60 - 0x44];
        s32 _60;
        u8 _64[0x70 - 0x64];
        CubeMapDirector* mCubeMapDirector;
        DirectionalLightKeeper* mDirectionalLightKeeper;
        SkyboxDirector* mSkyboxDirector;
        GraphicsAreaDirector* mGraphicsAreaDirector;
        LightIntensityDirector* mLightIntensityDirector;
        void* mRadialBlurDirector;
        PrePassLightKeeper* mPrePassLightKeeper;
        ShaderEnvTextureKeeper* mShaderEnvTextureKeeper;
        ModelLightDirector* mModelLightDirector;
        ShadowDirector* mShadowDirector;
        void* mEdgeDrawer;
        void* mDepthOfFieldDrawer;
        GraphicsStressDirector* mGraphicsStressDirector;
        ShaderMirrorDirector* mShaderMirrorDirector;
        void* mSSAOParamKeeper;
        void* mColorCorrectionParamKeeper;
        void* mFlareFilterDirector;
        void* mGodRayDirector;
        FogDirector* mFogDirector;
        void* mOccludedEffectDirector;
        void* mLightStreakDirector;
        void* mHdrCompose;
        SSIIKeeper* mSSIIKeeper;
        agl::sdw::PrimitiveOcclusion* mPrimitiveOcclusion;
        u8 _130[0x150 - 0x130];
        nn::g3d::ViewVolume mViewVolume;
        ViewRenderer* mViewRenderer;
        u8 _248[0x250 - 0x248];
        GBufferArray* mDrawGBufferArray;
        sead::LookAtCamera* mDrawCamera;
        sead::PerspectiveProjection* mDrawProjection;
        s32 mDrawViewIndex;
        s32 mDrawEnvUpdateCount;
        sead::Vector3f mDrawCameraPos;
        u8 _27c[0xd58 - 0x27c];
        AtmosScatter* mAtmosScatter;
        void* _d60;
        AtmosScatterDrawer* mAtmosScatterDrawer;
        u8 _d70[0x1048 - 0xd70];
        GpuMemAllocator* mGpuMemAllocator;
        FullScreenTriangle* mFullScreenTriangle;
        const char* mLodSettingName;
        u8 _1060[0x1080 - 0x1060];
    };
};  // namespace al
