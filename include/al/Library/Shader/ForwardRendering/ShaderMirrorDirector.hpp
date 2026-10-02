#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>
#include <utility/aglParameterObj.h>

#include "common/aglShaderEnum.h"
#include "Project/Draw/GraphicsParamKeeper.hpp"

#include "common/aglRenderBuffer.h"

namespace agl {
class TextureData;
}  // namespace agl

namespace sead {
class LookAtCamera;
}  // namespace sead

namespace al {
class GraphicsSystemInfo;
class LiveActorKit;

/**
 * Mirror parameters of a graphics area.
 */
class MirrorParam {
public:
    MirrorParam();

    void init();
    bool operator==(const MirrorParam& rOther) const;
    MirrorParam& operator=(const MirrorParam& rOther);
    void interp(const MirrorParam& rA, const MirrorParam& rB, f32 rate);

    agl::utl::IParameterObj* getParamObj() { return &mParamObj; }

private:
    agl::utl::ParameterObj mParamObj;
    u8 _30[0x390 - 0x30];
};

static_assert(sizeof(MirrorParam) == 0x390);

/**
 * Renders the mirror (planar reflection) views requested by graphics areas.
 */
class ShaderMirrorDirector : public GraphicsParamRequestInterpKeeper<MirrorParam> {
public:
    ShaderMirrorDirector(GraphicsSystemInfo* pInfo, LiveActorKit* pKit);
    ~ShaderMirrorDirector();

    void movement();
    bool isEnable() const;
    const sead::LookAtCamera* getRenderingCamera() const;
    void calcMirrorRenderBufferSize(sead::Vector2i* pSize) const;
    f32 getRenderingCameraNearOffset() const;
    void allocRenderBuffer();
    void startRendering();
    agl::ShaderMode endRendering(agl::ShaderMode shaderMode);
    void freeRenderBuffer();

    const agl::RenderBuffer& getRenderBuffer() const { return mRenderBuffer; }

    agl::TextureData* getDepthTexture() const { return mDepthTexture; }

    agl::TextureData* getColorTexture() const { return mColorTexture; }

private:
    u8 _1430[0x5f38 - 0x1430];
    agl::TextureData* mColorTexture;
    agl::TextureData* mDepthTexture;
    agl::RenderBuffer mRenderBuffer;
    u8 _5fb0[0x62b0 - 0x5fb0];
};

static_assert(sizeof(ShaderMirrorDirector) == 0x62b0);

}  // namespace al
