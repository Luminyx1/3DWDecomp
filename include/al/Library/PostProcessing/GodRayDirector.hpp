#pragma once

#include <basis/seadTypes.h>
#include <common/aglShaderEnum.h>
#include <gfx/seadColor.h>
#include <math/seadMatrix.h>
#include <utility/aglParameterObj.h>

#include "Project/Draw/GraphicsParamKeeper.hpp"

namespace agl {
class RenderBuffer;
class RenderTargetDepth;
class TextureData;
}  // namespace agl

namespace sead {
class Viewport;
}  // namespace sead

namespace al {
class GraphicsSystemInfo;
class LiveActor;
class LiveActorKit;
class SimpleModelEnv;
class Resource;

/**
 * God ray parameters of a graphics area.
 */
class GodRayParam {
public:
    void init();
    void initSystem();
    bool isEnable() const;
    const sead::Color4f& getColor() const;
    bool operator==(const GodRayParam& rOther) const;
    GodRayParam& operator=(const GodRayParam& rOther);
    void interp(const GodRayParam& rA, const GodRayParam& rB, f32 rate);

    agl::utl::IParameterObj* getParamObj() { return &mParamObj; }

private:
    agl::utl::ParameterObj mParamObj;
    u8 _30[0x1a0 - 0x30];
};

static_assert(sizeof(GodRayParam) == 0x1a0);

/**
 * Draws the god rays requested by graphics areas.
 */
class GodRayDirector : public GraphicsParamRequestInterpKeeper<GodRayParam> {
public:
    GodRayDirector(GraphicsSystemInfo* pInfo);
    virtual ~GodRayDirector();

    void initStageResource(const Resource* pResource, const char* pStageName) override;
    bool isEnable() const;
    s32 getBlurQuality() const;
    void releaseBuffer();
    agl::ShaderMode draw(s32 viewIndex, const agl::RenderTargetDepth& rDepthTarget,
                         const agl::TextureData& rTexture, const sead::Matrix34f& rViewMtx,
                         const sead::Matrix44f& rProjMtx, agl::ShaderMode shaderMode,
                         const SimpleModelEnv* pModelEnv, LiveActorKit* pKit) const;
    agl::ShaderMode composeToRenderBuffer(s32 viewIndex, const agl::RenderBuffer& rRenderBuffer,
                                          const sead::Viewport& rViewport,
                                          agl::ShaderMode shaderMode) const;

private:
    u8 _a80[0xac0 - 0xa80];
};

static_assert(sizeof(GodRayDirector) == 0xac0);

}  // namespace al

namespace GodRayFunction {
al::GodRayDirector* getGodRayDirector(const al::LiveActor* pActor);
}  // namespace GodRayFunction
