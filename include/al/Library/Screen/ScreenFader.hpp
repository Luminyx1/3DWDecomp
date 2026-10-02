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

    ScreenFader();

    void start(s32 delayFrame, s32 fadeFrame, f32 maxAlpha, const sead::Color4f& rColor);
    void end(s32 fadeFrame);
    void update();
    bool tryDraw(agl::DrawContext* pDrawContext, const sead::Viewport& rViewport,
                 const agl::RenderBuffer& rRenderBuffer) const;

    bool isEnd() const { return mState == State::End; }

private:
    s32 mFrame = -1;
    s32 mDelayFrame = 0;
    s32 mFadeFrame = 0;
    f32 mMaxAlpha = 0.0f;
    sead::Color4f mColor = sead::Color4f::cBlack;
    State mState = State::End;
};
}  // namespace al
