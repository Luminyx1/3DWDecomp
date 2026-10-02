#pragma once

#include <basis/seadTypes.h>
#include <utility/aglParameterObj.h>

#include "Project/Draw/GraphicsParamKeeper.hpp"

#include "common/aglShaderEnum.h"

namespace agl {
class RenderBuffer;
class TextureData;
}  // namespace agl

namespace sead {
class Viewport;
}  // namespace sead

namespace agl::pfx {
class Bloom;
}  // namespace agl::pfx

namespace al {
class GraphicsSystemInfo;

/**
 * HDR compose parameters of a graphics area.
 */
class HdrParam {
public:
    void init();
    bool operator==(const HdrParam& rOther) const;
    HdrParam& operator=(const HdrParam& rOther);
    void interp(const HdrParam& rA, const HdrParam& rB, f32 rate);

    agl::utl::IParameterObj* getParamObj() { return &mParamObj; }

private:
    agl::utl::ParameterObj mParamObj;
    u8 _30[0x398 - 0x30];
};

static_assert(sizeof(HdrParam) == 0x398);

/**
 * Composes the HDR post effects (bloom, flare filter, god ray, light streak).
 */
class HdrCompose : public GraphicsParamRequestInterpKeeper<HdrParam> {
public:
    HdrCompose(s32 viewNum, GraphicsSystemInfo* pInfo);
    ~HdrCompose();

    virtual void endInit();

    bool isUsingMyHdrCompose() const;
    void movement(bool isPaused);
    void preDrawGraphics();
    void releaseComposeBuffer();
    void calcGPU();
    void setupComposeBuffer(s32 viewIndex, const agl::RenderBuffer& rBuffer,
                            const agl::pfx::Bloom* pBloom);
    bool isAtLeastOneCompose(s32 viewIndex, const agl::pfx::Bloom* pBloom) const;
    bool isAtLeastOneComposeMask(s32 viewIndex, const agl::pfx::Bloom* pBloom) const;
    const agl::RenderBuffer* getRenderBufferFlareFilter() const;
    const agl::RenderBuffer* getRenderBufferBloom() const;
    const agl::RenderBuffer* getRenderBufferGodRay() const;
    const agl::RenderBuffer* getRenderBufferLightStreak() const;
    agl::ShaderMode draw(s32 viewIndex, const agl::RenderBuffer& rBuffer,
                         const sead::Viewport& rViewport, const agl::TextureData& rTexture,
                         const agl::pfx::Bloom* pBloom, agl::ShaderMode shaderMode) const;

    void setEnableDangerIndicator(bool isEnable) { mIsEnableDangerIndicator = isEnable; }

private:
    u8 _1458[0x2530 - 0x1458];
    bool mIsEnableDangerIndicator;
    u8 _2531[0x2558 - 0x2531];
};

static_assert(sizeof(HdrCompose) == 0x2558);

}  // namespace al
