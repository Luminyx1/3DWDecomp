#pragma once

#include <container/seadPtrArray.h>
#include <common/aglShaderEnum.h>
#include <math/seadVector.h>

#include "Library/PostProcessing/LightStreakParam.hpp"
#include "Project/Draw/GraphicsParamKeeper.hpp"

namespace agl {
class RenderBuffer;
class ShaderProgram;
class TextureData;

}  // namespace agl

namespace sead {
class Viewport;
}

namespace al {
class FullScreenQuadModel;
class GraphicsSystemInfo;
class Resource;
class UniformBlock;

class LightStreakDirector : public GraphicsParamRequestInterpKeeper<LightStreakParam> {
public:
    LightStreakDirector(GraphicsSystemInfo* pInfo);
    ~LightStreakDirector();

    void initStageResource(const Resource* pResource, const char* pStageName) override;
    bool isEnable() const;
    void setIntensity(f32 intensity);
    void updateUbo(s32 index, s32 passIndex) const;
    agl::ShaderMode drawToRenderBuffer(s32 index, const agl::RenderBuffer& rRenderBuffer,
                            const sead::Viewport& rViewport, const agl::TextureData& rTexture,
                            agl::ShaderMode shaderMode) const;
    agl::ShaderMode drawMask(s32 index, s32 width, s32 height, const agl::TextureData& rColorTexture,
                  const agl::TextureData& rDepthTexture, agl::ShaderMode shaderMode) const;
    agl::ShaderMode drawBlurMrt(s32 index, s32 width, s32 height, const agl::TextureData& rTextureA,
                     const agl::TextureData& rTextureB, s32 passIndex,
                     agl::ShaderMode shaderMode) const;
    agl::ShaderMode composeBlurToBuffer(s32 index, s32 width, s32 height,
                             const agl::RenderBuffer& rRenderBuffer,
                             const sead::Viewport& rViewport, const agl::TextureData& rTexture,
                             agl::ShaderMode shaderMode) const;

private:
    sead::FixedPtrArray<UniformBlock, 4> mUniformBlocks;
    agl::ShaderProgram* mMaskShader;
    agl::ShaderProgram* mBlurShader;
    agl::ShaderProgram* mComposeShader;
    FullScreenQuadModel* mFullScreenQuadModel;
};

static_assert(sizeof(LightStreakDirector) == 0xa58);

}  // namespace al
