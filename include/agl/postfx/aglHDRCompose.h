#pragma once

#include <container/seadBuffer.h>
#include <hostio/seadHostIONode.h>
#include <math/seadVector.h>
#include "common/aglRenderBuffer.h"
#include "common/aglTextureData.h"
#include "common/aglTextureSampler.h"
#include "utility/aglDebugTexturePage.h"

namespace sead {
class Heap;
class Viewport;
namespace hostio {
class Context;
class PropertyEvent;
}  // namespace hostio
}  // namespace sead

namespace agl {
class DrawContext;
}

namespace agl::pfx {

class HDRCompose : public sead::hostio::Node {
public:
    enum Flag {
        cFlag_EnableSampler2 = 1 << 0,
        cFlag_EnableSampler1 = 1 << 1,
        cFlag_Mode1 = 1 << 2,
        cFlag_EnableSampler0 = 1 << 4,
        cFlag_EnableExposure = 1 << 5,
        cFlag_Mode2 = 1 << 7,
    };

    struct Context {
        mutable TextureSampler mSampler;
        const TextureSampler* mpSampler0;
        const TextureSampler* mpSampler1;
        const TextureSampler* mpSampler2;
        sead::Vector2f mSampler1Param;
        f32 mExposure;
        sead::Vector2f mTexCoordScale;
        sead::Vector2f mTexCoordOffset;
        f32 mTexCoordRotate;
    };
    static_assert(sizeof(Context) == 0x1a8);

    HDRCompose();
    virtual ~HDRCompose();

    void initialize(s32 contextNum, sead::Heap* pHeap);
    void calc();
    void calcGPU() const;
    void calcGPU(s32 context) const;
    void draw(DrawContext* pDrawContext, s32 context, const RenderBuffer& rRenderBuffer,
              const sead::Viewport& rViewport, const TextureData& rTexture) const;
    void setTexCoordCoeff(s32 context, const sead::Vector2f& rScale,
                          const sead::Vector2f& rOffset, f32 rotate);
    void genMessage(sead::hostio::Context* pContext);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);

private:
    sead::Buffer<Context> mContexts;
    s32 mVariation2 = 0;
    s32 mSampler2Variation = 0;
    sead::Vector2f mParam{1.0f, 1.0f};
    u32 mFlags = cFlag_EnableExposure;
    utl::DebugTexturePage mDebugTexturePage;
};

}  // namespace agl::pfx
