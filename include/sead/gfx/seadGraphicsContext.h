#pragma once

#include <basis/seadTypes.h>
#include <gfx/seadColor.h>
#include <nvn/nvn.h>

namespace sead
{
class DrawContext;

class GraphicsContext
{
public:
    struct BlendTarget
    {
        u8 mBlendFactorSrcRGB;
        u8 mBlendFactorSrcA;
        u8 mBlendFactorDstRGB;
        u8 mBlendFactorDstA;
        u8 mBlendEquationRGB;
        u8 mBlendEquationA;
    };
    static_assert(sizeof(BlendTarget) == 6);

    static constexpr s32 cRenderTargetNum = 8;

    GraphicsContext();

    void apply(DrawContext* pDrawContext) const;
    void applyAlphaTest(DrawContext* pDrawContext) const;
    void applyDepthAndStencilTest(DrawContext* pDrawContext) const;
    void applyColorMask(DrawContext* pDrawContext) const;
    void applyBlendAndFastZ(DrawContext* pDrawContext) const;
    void applyBlendConstantColor(DrawContext* pDrawContext) const;
    void applyCullingAndPolygonModeAndPolygonOffset(DrawContext* pDrawContext) const;

    void setDepthTestEnable(bool enable) { mDepthTestEnable = enable; }
    void setDepthWriteEnable(bool enable) { mDepthWriteEnable = enable; }
    void setCullingMode(u8 mode) { mCullingMode = mode; }
    void setColorMask(u32 mask) { mColorMask = mask; }
    void setDepthFunc(u8 func) { mDepthFunc = func; }

    void setDepthEnable(bool test_enable, bool write_enable)
    {
        mDepthTestEnable = test_enable;
        mDepthWriteEnable = write_enable;
    }
    void setBlendEnable(bool enable)
    {
        if (enable)
            mBlendEnableMask |= 1;
        else
            mBlendEnableMask &= ~1u;
    }

private:
    bool mDepthTestEnable;
    bool mDepthWriteEnable;
    bool mAlphaTestEnable;
    bool mStencilTestEnable;
    u32 mBlendEnableMask;
    Color4f mBlendConstantColor;
    f32 mAlphaTestRef;
    u32 mColorMask;
    s32 mStencilTestRef;
    u32 mStencilTestMask;
    BlendTarget mBlendTargets[cRenderTargetNum];
    f32 mPolygonOffsetFactor;
    f32 mPolygonOffsetUnits;
    f32 mPolygonOffsetClamp;
    u8 mDepthFunc;
    u8 mCullingMode;
    u8 mAlphaTestFunc;
    u8 mStencilOpFail;
    u8 mStencilOpZFail;
    u8 mStencilOpZPass;
    u8 mStencilTestFunc;
    u8 mPolygonModeFront;
    u8 mPolygonModeBack;
    u8 mPolygonOffsetFrontEnable;
    u8 mPolygonOffsetBackEnable;
    u32 mStencilWriteMask;
};
static_assert(sizeof(GraphicsContext) == 0x74);

}  // namespace sead
