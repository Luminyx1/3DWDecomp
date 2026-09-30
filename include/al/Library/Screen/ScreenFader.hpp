#pragma once

#include <basis/seadTypes.h>
#include <gfx/seadColor.h>

namespace agl {
class DrawContext;
class RenderBuffer;
}  // namespace agl

namespace sead {
class Viewport;
}

namespace al {
class ScreenFader {
public:
    enum class State : s32 {
        FadeIn = 0,
        FadeOut = 1,
        End = 2,
    };

    void start(s32 delayFrame, s32 fadeFrame, f32 maxAlpha, const sead::Color4f& rColor);
    void end(s32 fadeFrame);
    void update();
    bool tryDraw(agl::DrawContext* pDrawContext, const sead::Viewport& rViewport,
                 const agl::RenderBuffer& rRenderBuffer) const;

private:
    s32 mFrame;
    s32 mDelayFrame;
    s32 mFadeFrame;
    f32 mMaxAlpha;
    sead::Color4f mColor;
    State mState;
};
}  // namespace al
