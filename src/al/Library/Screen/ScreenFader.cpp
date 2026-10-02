#include "Library/Screen/ScreenFader.hpp"

#include <agl/common/aglDrawContext.h>
#include <agl/common/aglRenderBuffer.h>
#include <gfx/seadCamera.h>
#include <gfx/seadGraphicsContext.h>
#include <gfx/seadPrimitiveRenderer.h>
#include <gfx/seadProjection.h>
#include <gfx/seadViewport.h>

#include "Library/Math/MathUtil.hpp"

namespace al {
namespace {

/**
 * Draws a screen-filling quad in the fade color.
 * @param pDrawContext draw context
 * @param rColor fade color
 * @param alpha alpha of the fade
 * @param maxAlpha maximum alpha of the fade
 * @param rViewport viewport
 * @param rFrameBuffer frame buffer to draw to
 */
void drawFadeQuad(agl::DrawContext* pDrawContext, const sead::Color4f& rColor, f32 alpha,
                  f32 maxAlpha, const sead::Viewport& rViewport,
                  const sead::FrameBuffer& rFrameBuffer) {
    sead::GraphicsContext context;
    context.setDepthEnable(false, false);
    context.setBlendEnable(true);
    context.setBlendFactor(0, 5, 6);
    context.setBlendEquation(0, 1);
    context.setCullingMode(0);
    context.apply(pDrawContext);
    pDrawContext->changeShaderMode(agl::cShaderMode_UniformRegister, agl::ShaderOptimizeType(0));
    rViewport.apply(pDrawContext, rFrameBuffer);
    rFrameBuffer.bind(pDrawContext);

    sead::OrthoProjection projection(0.0f, 10000.0f, rViewport);
    sead::OrthoCamera camera(projection);
    sead::PrimitiveDrawer::QuadArg quadArg;
    quadArg.setCenter(sead::Vector3f(0.0f, 0.0f, 0.0f));
    quadArg.setSize(rFrameBuffer.getVirtualSize());
    quadArg.setColor(
        sead::Color4f(rColor.r, rColor.g, rColor.b, sead::Mathf::min(alpha, maxAlpha)));

    sead::PrimitiveDrawer drawer(pDrawContext);
    drawer.setCamera(&camera);
    drawer.setProjection(&projection);
    drawer.setModelMatrix(&sead::Matrix34f::ident);
    drawer.begin();
    drawer.drawQuad(quadArg);
    drawer.end();
    drawer.setModelMatrix(&sead::Matrix34f::ident);

    context.setDepthEnable(true, true);
    context.setCullingMode(2);
    context.apply(pDrawContext);
    pDrawContext->changeShaderMode(agl::cShaderMode_UniformBlock, agl::ShaderOptimizeType(0));
}

}  // namespace

/**
 * Constructs a fader that is not fading.
 */
ScreenFader::ScreenFader() = default;

/**
 * Starts fading the screen in.
 * @param delayFrame frames to wait before fading
 * @param fadeFrame frames the fade takes
 * @param maxAlpha maximum alpha of the fade
 * @param rColor fade color
 */
void ScreenFader::start(s32 delayFrame, s32 fadeFrame, f32 maxAlpha, const sead::Color4f& rColor) {
    mFrame = 0;
    mDelayFrame = delayFrame;
    mFadeFrame = fadeFrame;
    mMaxAlpha = maxAlpha;
    mColor = rColor;
    mState = State::FadeIn;
}

/**
 * Starts fading the screen out.
 * @param fadeFrame frames the fade takes, or -1 to end immediately
 */
void ScreenFader::end(s32 fadeFrame) {
    mFrame = 0;
    mDelayFrame = 0;
    mFadeFrame = fadeFrame;
    mState = State::FadeOut;

    if (fadeFrame == -1) {
        mState = State::End;
        mFrame = -1;
    }
}

/**
 * Advances the fade.
 */
void ScreenFader::update() {
    if (mFrame < 0) {
        return;
    }

    mFrame++;

    if (mState == State::FadeOut && mFrame >= mFadeFrame) {
        mState = State::End;
    }
}

/**
 * Draws the fade if it is active.
 * @param pDrawContext draw context
 * @param rViewport viewport
 * @param rRenderBuffer render buffer to draw to
 * @return whether the fade was drawn
 */
bool ScreenFader::tryDraw(agl::DrawContext* pDrawContext, const sead::Viewport& rViewport,
                          const agl::RenderBuffer& rRenderBuffer) const {
    if (mFrame < 0) {
        return false;
    }

    f32 rate = sead::Mathi::max(mFrame - mDelayFrame, 0) / static_cast<f32>(mFadeFrame);
    rate = rate > 1.0f ? 1.0f : rate;
    f32 maxAlpha = mMaxAlpha;

    if (mState == State::FadeIn) {
        drawFadeQuad(pDrawContext, mColor, rate, maxAlpha, rViewport, rRenderBuffer);
    } else {
        f32 alpha = lerpValueNew(maxAlpha, 0.0f, rate);
        drawFadeQuad(pDrawContext, mColor, alpha, alpha, rViewport, rRenderBuffer);
    }

    return true;
}
}  // namespace al
