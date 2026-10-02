#pragma once

#include <common/aglGPUMemAddr.h>
#include <common/aglRenderTarget.h>

namespace agl {
class RenderBuffer;
class TextureData;
}  // namespace agl

namespace al {

/**
 * Temporarily attaches a depth texture (with its Z-cull buffer) and color textures to a render
 * buffer and binds it.
 */
class RenderBufferDepthAttacher {
public:
    RenderBufferDepthAttacher(agl::RenderBuffer* pRenderBuffer, const agl::TextureData* pDepth,
                              agl::ConstGPUMemVoidAddr zCullBuffer,
                              const agl::TextureData* pColor0, const agl::TextureData* pColor1,
                              const agl::TextureData* pColor2, const agl::TextureData* pColor3);
    ~RenderBufferDepthAttacher();

private:
    agl::RenderBuffer* mRenderBuffer;
    agl::RenderTargetColor mColorTargets[4];
    agl::RenderTargetDepth mDepthTarget;
    const agl::TextureData* mColorTextures[4];
    const agl::TextureData* mDepthTexture;
};

static_assert(sizeof(RenderBufferDepthAttacher) == 0x788);

}  // namespace al
