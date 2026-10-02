#pragma once

#include <basis/seadTypes.h>
#include <common/aglShaderEnum.h>

#include "Project/Draw/GraphicsParamKeeper.hpp"

namespace agl {
class RenderBuffer;
class RenderTargetDepth;
class TextureData;
}  // namespace agl

namespace sead {
class Viewport;
}  // namespace sead

namespace agl::pfx {
class FlareFilter;
}  // namespace agl::pfx

namespace al {
class GraphicsSystemInfo;

/**
 * Keeps the flare filter and its stage parameters.
 */
class FlareFilterDirector : public GraphicsParamKeeper<agl::pfx::FlareFilter> {
public:
    FlareFilterDirector(s32 viewNum, GraphicsSystemInfo* pInfo);
    ~FlareFilterDirector();

    void movement();
    agl::ShaderMode draw(s32 viewIndex, const agl::RenderBuffer& rRenderBuffer,
                         const sead::Viewport& rViewport, const agl::TextureData& rTexture,
                         agl::ShaderMode shaderMode) const;

private:
    u8 _148[0x1d8 - 0x148];
};

static_assert(sizeof(FlareFilterDirector) == 0x1d8);

}  // namespace al
