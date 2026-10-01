#pragma once

#include <basis/seadTypes.h>

#include "Library/Yaml/ParameterBase.hpp"
#include "Project/Base/ParamRequestInterp.hpp"

namespace agl {
class DrawContext;
class RenderBuffer;
class ShaderProgram;
class TextureData;
}  // namespace agl

namespace nn::g3d {
class ResFile;
}

namespace al {
class ShaderHolder;
class UniformBlock;

class MangaDrawParam : public IUseRequestParam {
public:
    MangaDrawParam();

    const char* getParamName() const override { return "マンガ描画"; }

    ParameterObj* getParamObj() override { return mParamObj; }

    const ParameterObj* getParamObj() const override { return mParamObj; }

    bool isEnable() const;
    bool isOnlyNormalLineDraw() const;
    bool isUseWhiteEdge() const;
    f32 getWhiteParam() const;
    f32 getBlackParam() const;
    s32 getMipLevelParam() const;
    bool isUseLinearDepth() const;
    s32 getKernelSize() const;
    s32 getGaussianType() const;
    s32 getGaussianTypeForEdge() const;
    f32 getThreshold() const;
    f32 getThresholdWhiteMin() const;
    f32 getThresholdWhiteMax() const;
    f32 getThresholdDepth() const;
    s32 getScreenToneType() const;
    s32 getScreenToneTypeDepth() const;
    f32 getNormalParam() const;
    f32 getNormalNearParam() const;
    f32 getDepthZParam() const;
    f32 getWhiteEdgeDistanceNear() const;
    f32 getWhiteEdgeDistanceFar() const;
    f32 getNearParam() const;
    f32 getWhiteAlphaBase() const;
    f32 getWhiteAlphaAdd() const;
    f32 getRepeateNumScreenDot() const;
    f32 getRepeateNumScreenLine() const;
    f32 getWhiteBaseColor() const;
    f32 getBlackBaseColor() const;

private:
    ParameterObj* mParamObj;
    ParameterBool* mIsEnable;
    ParameterBool* mIsOnlyNormalLineDraw;
    ParameterBool* mIsUseWhiteEdge;
    ParameterF32* mWhiteParam;
    ParameterF32* mBlackParam;
    ParameterS32* mMipLevelParam;
    ParameterBool* mIsUseLinearDepth;
    ParameterS32* mKernelSize;
    ParameterS32* mGaussianType;
    ParameterS32* mGaussianTypeForEdge;
    ParameterF32* mThreshold;
    ParameterF32* mThresholdWhiteMin;
    ParameterF32* mThresholdWhiteMax;
    ParameterF32* mThresholdDepth;
    ParameterS32* mScreenToneType;
    ParameterS32* mScreenToneTypeDepth;
    ParameterF32* mNormalParam;
    ParameterF32* mNormalNearParam;
    ParameterF32* mDepthZParam;
    ParameterF32* mWhiteEdgeDistanceNear;
    ParameterF32* mWhiteEdgeDistanceFar;
    ParameterF32* mNearParam;
    ParameterF32* mWhiteAlphaBase;
    ParameterF32* mWhiteAlphaAdd;
    ParameterF32* mRepeateNumScreenDot;
    ParameterF32* mRepeateNumScreenLine;
    ParameterF32* mWhiteBaseColor;
    ParameterF32* mBlackBaseColor;
};

static_assert(sizeof(MangaDrawParam) == 0xf0);

class MangaDrawer {
public:
    MangaDrawer(ShaderHolder* pShaderHolder);
    ~MangaDrawer();
    void initProjectResource(nn::g3d::ResFile* pResFile);
    void endInit();
    void clearRequest();
    void update();
    void draw(agl::DrawContext* pContext, const agl::RenderBuffer& rBuffer,
              const agl::TextureData* pLinearDepth, const agl::TextureData* pLinearDepth2,
              const agl::TextureData* pDepth, const agl::TextureData* pNormal, f32 near,
              f32 far, f32 depthZScale) const;
    const MangaDrawParam* getCurrentParam() const;
    void requestParam(s32 priority, s32 step, const MangaDrawParam& rParam);
    bool isEnable() const;
    void nextFrameId();

private:
    ShaderHolder* mShaderHolder = nullptr;
    agl::ShaderProgram* mRenderLuminanceProgram = nullptr;
    agl::ShaderProgram* mGenerateMipmapProgram = nullptr;
    ParamRequestInterp* mRequestInterp = nullptr;
    UniformBlock** mMipUniformBlocks = nullptr;
    agl::TextureData* mComicDotTexture = nullptr;
    agl::TextureData* mComicLineTexture = nullptr;
};

static_assert(sizeof(MangaDrawer) == 0x38);
}  // namespace al
