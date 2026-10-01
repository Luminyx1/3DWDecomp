#include <nn/ui2d/ui2d_Scissor.h>
#include <nn/ui2d/ui2d_DrawInfo.h>
#include <nn/gfx/gfx_CommandBuffer.h>
#include <cmath>
namespace nn::ui2d {
Scissor::Scissor() = default;
// resource supplies serialized pane properties; args supplies the layout context.
Scissor::Scissor(const ResScissor* resource, const BuildArgSet& args)
    : Pane(reinterpret_cast<const ResPane*>(resource), args) {}
Scissor::~Scissor() = default;
// scissor receives the integer rectangle; x/y are its origin and width/height
// its extent. Round both edges before subtracting to preserve pixel boundaries.
void Scissor::SetScissorStateInfoValue(nn::gfx::ScissorStateInfo* scissor, float x, float y, float width, float height) {
    float right = x + width, bottom = y + height;
    scissor->SetOriginX(int(x + 0.5f));
    scissor->SetOriginY(int(y + 0.5f));
    scissor->SetWidth(int(right + 0.5f) - scissor->GetOriginX());
    scissor->SetHeight(int(bottom + 0.5f) - scissor->GetOriginY());
}

// pane provides its transformed rectangle; info supplies the framebuffer size.
// The viewport bounds map it to pixels, returned through x/y/width/height.
static inline void CalculateScissorRectangle(float& x, float& y, float& width, float& height,
    const Pane& pane, const DrawInfo& info, float viewportX, float viewportY, float viewportWidth, float viewportHeight) {
    float32x4_t row0 = vld1q_f32(pane.GetGlobalMtx());
    float32x4_t row1 = vld1q_f32(pane.GetGlobalMtx() + 4);
    float halfWidth = pane.GetSizeX() * vgetq_lane_f32(row0, 0) * 0.5f;
    float halfHeight = pane.GetSizeY() * vgetq_lane_f32(row1, 1) * 0.5f;
    float centerX = vgetq_lane_f32(row0, 3);
    float centerY = vgetq_lane_f32(row1, 3);
    width = std::fabs(halfWidth) * 2.0f;
    height = std::fabs(halfHeight) * 2.0f;
    int originX = pane.GetBasePositionX();
    int originY = pane.GetBasePositionY();

    if (originX == 1) {
        centerX += halfWidth;
    } else if (originX == 2) {
        centerX -= halfWidth;
    }

    if (originY == 1) {
        centerY -= halfHeight;
    } else if (originY == 2) {
        centerY += halfHeight;
    }

    if (info.mFlags & 8) {
        centerY = -centerY;
    }

    float frameWidth = float(info.mFramebufferWidth);
    float frameHeight = float(info.mFramebufferHeight);

    if ((info.mFramebufferWidth | info.mFramebufferHeight) == 0) {
        frameWidth = viewportWidth;
        frameHeight = viewportHeight;
    }

    float scaleX = viewportWidth / frameWidth, scaleY = viewportHeight / frameHeight;
    centerX -= width * 0.5f;
    centerY -= height * 0.5f;
    centerX += frameWidth * 0.5f;
    centerY += frameHeight * 0.5f;
    x = viewportX + scaleX * centerX;
    y = viewportY + scaleY * centerY;
    width *= scaleX;
    height *= scaleY;
}

// info supplies framebuffer and viewport dimensions; commands receives the
// clipped drawing and restoration of the caller's previous scissor rectangle.
void Scissor::Draw(DrawInfo& info, nn::gfx::CommandBuffer& commands) {
    if (!(mFlags & 1) || !mAlphaInfluence) { Pane::Draw(info, commands); return; }
    float viewportX = info.mViewport.GetOriginX();
    float viewportY = info.mViewport.GetOriginY();
    float viewportWidth = info.mViewport.GetWidth();
    float viewportHeight = info.mViewport.GetHeight();
    float x, y, width, height;
    CalculateScissorRectangle(x, y, width, height, *this, info, viewportX, viewportY, viewportWidth, viewportHeight);

    if (x < viewportX) { width -= viewportX - x; x = viewportX; }
    if (x + width > viewportWidth + viewportX) width = viewportWidth + viewportX - x;

    if (y < viewportY) { height -= viewportY - y; y = viewportY; }
    if (y + height > viewportHeight + viewportY) height = viewportHeight + viewportY - y;

    if (!(y < viewportHeight + viewportY && x < viewportWidth + viewportX)) return;

    if (!(width > 0)) return;

    if (!(height > 0)) return;
    {
        nn::gfx::ScissorStateInfo scissor;
        SetScissorStateInfoValue(&scissor, x, y, width, height);
        auto previous = info.mScissor;
        commands.SetScissors(0, 1, &scissor);
        info.mScissor = scissor;
        Pane::Draw(info, commands);
        commands.SetScissors(0, 1, &previous);
        info.mScissor = previous;
    }
}
}
