#pragma once

#include <basis/seadTypes.h>
#include <common/aglShaderEnum.h>

namespace agl {
class RenderBuffer;
}

namespace sead {
class Viewport;
}

namespace al {
class GraphicsSystemInfo;
class LiveActorKit;
class RenderVariables;
class SceneCameraInfo;

class ViewRenderer {
public:
    ViewRenderer(GraphicsSystemInfo* pInfo);
    virtual ~ViewRenderer();

    virtual void preDrawGraphics(const SceneCameraInfo* pCameraInfo);
    virtual void drawView(s32 viewIndex, s32 index, LiveActorKit* pKit,
                          const SceneCameraInfo* pCameraInfo, const agl::RenderBuffer* pBuffer,
                          const sead::Viewport& rViewport, bool, bool,
                          agl::ShaderMode shaderMode) const;
    virtual void drawSystem(LiveActorKit* pKit, agl::ShaderMode shaderMode) const;
    virtual void drawHdr(s32 viewIndex, const RenderVariables& rVariables, bool, bool, bool,
                         agl::ShaderMode shaderMode) const;
    virtual void drawMirror(s32 viewIndex, RenderVariables* pVariables, bool,
                            agl::ShaderMode shaderMode) const;

    void updatePreDraw();
    void setReducedEffectRender(bool, bool);
    void setSingleModeRendering();
    void dangerIndicatorEnable(bool isEnable);
    void enableSSR();

private:
    u8 _8[0xe90 - 0x8];
};

static_assert(sizeof(ViewRenderer) == 0xe90);

}  // namespace al
