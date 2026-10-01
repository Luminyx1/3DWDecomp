#pragma once

#include <gfx/seadFrameBuffer.h>
#include <gfx/seadViewport.h>
#include <hostio/seadHostIONode.h>
#include "common/aglGPUMemAddr.h"
#include "common/aglRenderBuffer.h"
#include "common/aglRenderTarget.h"
#include "common/aglTextureData.h"
#include "common/aglTextureSampler.h"
#include "utility/aglImageFilter2D.h"

namespace sead {
namespace hostio {
class Context;
class PropertyEvent;
}  // namespace hostio
}  // namespace sead

namespace agl {
class DrawContext;
}

namespace al {
class BlurFilter;
}

namespace agl::pfx {

class BlurFilter : public sead::hostio::Node {
public:
    BlurFilter();
    virtual ~BlurFilter();

    void setupRenderTarget(const sead::LogicalFrameBuffer& rFrameBuffer,
                           const sead::Viewport& rViewport, utl::ImageFilter2D::ReduceScale scale);
    void draw(DrawContext* pDrawContext, const RenderBuffer& rRenderBuffer) const;
    void genMessage(sead::hostio::Context* pContext);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);

    void setIteration(s32 iteration) { mIteration = iteration; }
    void setEnable(bool isEnable) { mIsEnable = isEnable; }
    bool isEnable() const { return mIsEnable; }
    void setBlurType(s32 type, s32 param) {
        mBlurType = type;
        _984 = param;
    }

private:
    friend class al::BlurFilter;

    TextureData mTexture[2];
    mutable TextureSampler mSampler[2];
    RenderTargetColor mTarget[2];
    RenderBuffer mRenderBuffer[2];
    GPUMemAddr<u8> mImage[2];
    utl::ImageFilter2D::ReduceScale mReduceScale = utl::ImageFilter2D::cReduceScale_2;
    sead::LogicalFrameBuffer mFrameBuffer{sead::Vector2f(1.0f, 1.0f), 0.0f, 0.0f, 1.0f, 1.0f};
    sead::Viewport mViewport;
    s32 mIteration = 0;
    bool mIsEnable = false;
    bool mIsPointSampling = false;
    bool mIsUseReduce = true;
    s32 mBlurType = 1;
    s32 _984 = 1;
};
static_assert(sizeof(BlurFilter) == 0x988);

}  // namespace agl::pfx
