#pragma once

#include <basis/seadTypes.h>
#include <common/aglShaderEnum.h>
#include <container/seadPtrArray.h>
#include <gfx/seadColor.h>

#include "Library/Yaml/ParameterBase.hpp"
#include "Project/AreaObj/AreaObj.hpp"
#include "Project/AreaObj/IUseAreaObj.hpp"
#include "Project/Base/ParamRequestInterp.hpp"

namespace agl {
class RenderBuffer;
class ShaderProgram;
class TextureData;
}  // namespace agl

namespace al {
class AreaObjDirector;
class FullScreenQuadModel;
class GraphicsSystemInfo;
class PlayerHolder;
class SceneCameraInfo;
class ShaderHolder;
class UniformBlock;

class EdgeDrawParam : public IUseRequestParam {
public:
    EdgeDrawParam();

    const char* getParamName() const override { return "Edge Draw"; }

    ParameterObj* getParamObj() override { return mParamObj; }

    const ParameterObj* getParamObj() const override { return mParamObj; }

    bool isEnable() const;
    bool isConstColor() const;
    bool isBold() const;
    f32 getBoundDepth() const;
    f32 getEdgeEndDepth() const;
    f32 getEdgePowerMin() const;
    f32 getEdgeNormalEdgeBound() const;
    const sead::Color4f& getOffsetColor() const;

private:
    ParameterObj* mParamObj;
    ParameterBool* mIsEnable;
    ParameterBool* mIsConstColor;
    ParameterBool* mIsBold;
    ParameterF32* mBoundDepth;
    ParameterF32* mEdgeEndDepth;
    ParameterF32* mEdgePowerMin;
    ParameterF32* mEdgeNormalEdgeBound;
    ParameterC4f* mOffsetColor;
};

static_assert(sizeof(EdgeDrawParam) == 0x50);

class EdgeDrawArea : public AreaObj {
public:
    bool isConstColor() const { return mIsConstColor; }

    f32 getEdgeEndDepth() const { return mEdgeEndDepth; }

    f32 getEdgePowerMin() const { return mEdgePowerMin; }

    f32 getBoundDepth() const { return mBoundDepth; }

    const sead::Color4f& getOffsetColor() const { return mOffsetColor; }

private:
    bool mIsConstColor;
    f32 mEdgeEndDepth;
    f32 mEdgePowerMin;
    f32 mBoundDepth;
    sead::Color4f mOffsetColor;
};

class EdgeDrawer : public IUseAreaObj {
public:
    using UniformBlockArray = sead::PtrArray<UniformBlock>;

    EdgeDrawer(const GraphicsSystemInfo* pSystemInfo, s32 viewNum);
    ~EdgeDrawer();

    void init(const SceneCameraInfo* pCameraInfo, ShaderHolder* pShaderHolder,
              AreaObjDirector* pAreaObjDirector, const PlayerHolder* pPlayerHolder);
    void update();
    agl::ShaderMode draw(const agl::RenderBuffer* pRenderBuffer,
                         const agl::RenderBuffer* pEdgeBuffer, const agl::TextureData* pNormalTexture,
                         const agl::TextureData* pDepthTexture, agl::ShaderMode mode, s32 viewIndex,
                         f32 near, f32 far) const;
    agl::ShaderMode drawEdge(const agl::RenderBuffer* pRenderBuffer,
                             const agl::RenderBuffer* pEdgeBuffer,
                             const agl::TextureData* pNormalTexture,
                             const agl::TextureData* pDepthTexture, agl::ShaderMode mode,
                             s32 viewIndex, f32 near, f32 far, bool isKeepBuffer) const;
    agl::ShaderMode drawReduction(const agl::RenderBuffer& rRenderBuffer,
                                  const agl::TextureData* pDepthTexture, s32 viewIndex,
                                  agl::ShaderMode mode, bool isMinDepth) const;
    agl::ShaderMode drawMix(const agl::RenderBuffer& rRenderBuffer,
                            const agl::TextureData* pDepthTexture, agl::ShaderMode mode) const;

    AreaObjDirector* getAreaObjDirector() const override { return mAreaObjDirector; }

private:
    const GraphicsSystemInfo* mSystemInfo;
    const SceneCameraInfo* mCameraInfo = nullptr;
    AreaObjDirector* mAreaObjDirector = nullptr;
    const PlayerHolder* mPlayerHolder = nullptr;
    bool mIsEnable = false;
    bool mIsEnableArea = true;
    bool mIsInArea = false;
    FullScreenQuadModel* mQuadModel = nullptr;
    const agl::ShaderProgram* mShaderProgram = nullptr;
    f32 mBoundDepth = 0.00015f;
    bool mIsConstColor = false;
    sead::Color4f mEdgeColor = {0.0f, 0.0f, 0.0f, 1.0f};
    sead::Color4f mEdgeScale = {1.0f, 1.0f, 1.0f, 1.0f};
    f32 mEdgeEndDepth = 0.07f;
    f32 mEdgePowerMin = 0.1f;
    sead::Color4f mOffsetColor;
    UniformBlockArray mEdgeUniformBlocks;
    UniformBlockArray mReductionUniformBlocks;
};

static_assert(sizeof(EdgeDrawer) == 0xa0);
}  // namespace al
