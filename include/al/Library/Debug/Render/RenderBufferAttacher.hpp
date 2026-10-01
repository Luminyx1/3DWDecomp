#pragma once

#include <common/aglRenderTarget.h>

namespace agl {
class DrawContext;
class RenderBuffer;
class TextureData;
}  // namespace agl

namespace sead {
class Viewport;
}

namespace al {

/**
 * Temporarily attaches color/depth textures to a render buffer and binds it.
 */
class RenderBufferAttacher {
public:
    RenderBufferAttacher(agl::RenderBuffer* pRenderBuffer, const agl::TextureData* pColor0,
                         const agl::TextureData* pColor1, const agl::TextureData* pColor2,
                         const agl::TextureData* pColor3, const agl::RenderTargetDepth* pDepth);
    RenderBufferAttacher(agl::DrawContext* pDrawContext, agl::RenderBuffer* pRenderBuffer,
                         const agl::TextureData* pColor0, const agl::TextureData* pColor1,
                         const agl::TextureData* pColor2, const agl::TextureData* pColor3,
                         const agl::RenderTargetDepth* pDepth);
    RenderBufferAttacher(agl::RenderBuffer* pRenderBuffer, const sead::Viewport* pViewport,
                         const agl::TextureData* pColor0, const agl::TextureData* pColor1,
                         const agl::TextureData* pColor2, const agl::TextureData* pColor3,
                         const agl::RenderTargetDepth* pDepth);
    RenderBufferAttacher(agl::RenderBuffer* pRenderBuffer, s32 mipLevel,
                         const agl::TextureData* pColor0, const agl::TextureData* pColor1,
                         const agl::TextureData* pColor2, const agl::TextureData* pColor3,
                         const agl::RenderTargetDepth* pDepth);
    ~RenderBufferAttacher();

private:
    agl::RenderBuffer* mRenderBuffer;
    agl::RenderTargetColor mColorTargets[4];
    const agl::TextureData* mColorTextures[4];
};

static_assert(sizeof(RenderBufferAttacher) == 0x608);

}  // namespace al
