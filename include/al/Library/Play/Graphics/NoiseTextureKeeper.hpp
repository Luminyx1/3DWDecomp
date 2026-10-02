#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>

#include "Library/Play/Draw/PartsGraphics.hpp"

namespace agl {
class DrawContext;
class ShaderProgram;
class TextureSampler;
}  // namespace agl

class NoiseTexture;

namespace al {
class CubeMapDrawInfo;
class FullScreenTriangle;
class GraphicsSystemInfo;
class ShaderHolder;

/**
 * @brief Owns the procedural noise textures (perlin, worley, curl, ...) and renders the ones that
 * have been declared as used.
 */
class NoiseTextureKeeper : public PartsGraphics {
public:
    NoiseTextureKeeper(GraphicsSystemInfo* pInfo, ShaderHolder* pShaderHolder);
    ~NoiseTextureKeeper();

    void finalize() override;
    void endInit() override;

    void declareUsingGemNoiseCubemapTexture();
    void declareUsingPerlinFbmNoise2DTexture();
    void declareUsingWorleyNoise2DTexture();
    void declareUsingFrostNoise2DTexture();
    void declareUsingCurlNoise2DTexture();
    void declareUsingPerlinNoise2DTexture();
    void declareUsingFrostNoise3DTexture();
    void declareUsingWorleyNoise3DTexture();
    void declareUsingWorleyThinNoise3DTexture();
    void declareUsingWorleyThinAnimNoise3DTexture();
    void declareUsingCloudLikeFbm3DNoiseTexture();
    void declareUsingOceanFoam3DNoiseTexture();
    void declareUsingSnowCovered3DNoiseTexture();
    void declareUsingCurlNoise3DTexture();
    void declareUsingRidgeNoise3DTexture();
    void declareUsingCaustics3DTexture();
    void declareUsingPerlinNoise3DTexture();
    void declareUsingPerlinFbmNoise3DTexture();
    void declareUsingGemNoise3DTexture();
    void declareUsingSimpleNoiseTexture();
    void declareUsingCloudNoise3DTexture();

    const agl::TextureSampler* getCloudVolume3DSampler() const;
    void activateGemNoiseCubemapTexture(agl::DrawContext* pDrawContext) const;
    void activateTexture2D(agl::DrawContext* pDrawContext, s32 index) const;
    const NoiseTexture* getTexture2D(s32 index) const;
    const NoiseTexture* getTexture3D(s32 index) const;
    const agl::TextureSampler* getTexture2DSampler(s32 index) const;
    const agl::TextureSampler* getTexture3DSampler(s32 index) const;
    void activateTexture3D(agl::DrawContext* pDrawContext, s32 index) const;

    void update(const GraphicsUpdateInfo& rInfo) override;
    void calcGpu(const GraphicsCalcGpuInfo& rInfo) override;
    void drawSystem(const GraphicsRenderInfo* pInfo) const override;

    const char* getName() const override { return "ノイズテクスチャ管理"; }

private:
    NoiseTexture* mBlack2D;
    NoiseTexture* mBlack3D;
    const agl::ShaderProgram* mCurlShader;
    NoiseTexture* mCurl2D;
    NoiseTexture* mCurl3D;
    const agl::ShaderProgram* mSimpleShader;
    NoiseTexture* mSimple;
    const agl::ShaderProgram* mPerlinShader;
    NoiseTexture* mPerlin2D;
    NoiseTexture* mPerlin3D;
    NoiseTexture* mPerlinFbm2D;
    NoiseTexture* mPerlinFbm3D;
    NoiseTexture* mRidge3D;
    const agl::ShaderProgram* mWorleyShader;
    NoiseTexture* mWorley2D;
    NoiseTexture* mWorley3D;
    NoiseTexture* mWorleyThin3D;
    NoiseTexture* mWorleyThinAnim3D;
    NoiseTexture* mCloud3D;
    const agl::ShaderProgram* mCloudLikeFbmShader;
    NoiseTexture* mCloudLikeFbm3D;
    NoiseTexture* mOceanFoam3D;
    NoiseTexture* mSnowCovered3D;
    NoiseTexture* mFrost2D;
    NoiseTexture* mFrost3D;
    CubeMapDrawInfo* mCubeMapDrawInfo;
    const agl::ShaderProgram* mGemShader;
    NoiseTexture* mGemNoiseCubemap;
    NoiseTexture* mGemNoise3D;
    const agl::ShaderProgram* mCausticsShader;
    NoiseTexture* mCaustics3D;
    NoiseTexture* mCaustics3D1ch;
    sead::FixedPtrArray<NoiseTexture, 32> mTextures;
    FullScreenTriangle* mFullScreenTriangle;
    f32 mTime;
    f32 mDeltaTime;
};

static_assert(sizeof(NoiseTextureKeeper) == 0x248);

}  // namespace al
