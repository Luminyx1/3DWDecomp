#include "gfx/seadGraphicsContext.h"

#include "gfx/seadDrawContext.h"
#include "nvn/nvn_FuncPtrInline.h"

namespace sead
{
/**
 * Constructs a graphics context with alpha blending and depth testing enabled.
 */
GraphicsContext::GraphicsContext()
{
    mDepthTestEnable = true;
    mDepthWriteEnable = true;
    mAlphaTestEnable = false;
    mStencilTestEnable = false;
    mBlendEnableMask = 0xff;
    mBlendConstantColor = Color4f(1.0f, 1.0f, 1.0f, 1.0f);
    mAlphaTestRef = 0.0f;
    mColorMask = 0xffffffff;
    mStencilTestRef = 0;
    mStencilTestMask = 0xffffffff;
    for (s32 i = 0; i < cRenderTargetNum; i++)
    {
        mBlendTargets[i].mBlendFactorSrcRGB = NVN_BLEND_FUNC_SRC_ALPHA;
        mBlendTargets[i].mBlendFactorSrcA = NVN_BLEND_FUNC_SRC_ALPHA;
        mBlendTargets[i].mBlendFactorDstRGB = NVN_BLEND_FUNC_ONE_MINUS_SRC_ALPHA;
        mBlendTargets[i].mBlendFactorDstA = NVN_BLEND_FUNC_ONE_MINUS_SRC_ALPHA;
        mBlendTargets[i].mBlendEquationRGB = NVN_BLEND_EQUATION_ADD;
        mBlendTargets[i].mBlendEquationA = NVN_BLEND_EQUATION_ADD;
    }
    mPolygonOffsetFactor = 0.0f;
    mPolygonOffsetUnits = 0.0f;
    mPolygonOffsetClamp = 0.0f;
    mDepthFunc = NVN_DEPTH_FUNC_LEQUAL;
    mCullingMode = NVN_FACE_BACK;
    mAlphaTestFunc = NVN_ALPHA_FUNC_GREATER;
    mStencilOpFail = NVN_STENCIL_OP_KEEP;
    mStencilOpZFail = NVN_STENCIL_OP_KEEP;
    mStencilOpZPass = NVN_STENCIL_OP_KEEP;
    mStencilTestFunc = NVN_STENCIL_FUNC_NEVER;
    mPolygonModeFront = NVN_POLYGON_MODE_FILL;
    mPolygonModeBack = NVN_POLYGON_MODE_FILL;
    mPolygonOffsetFrontEnable = false;
    mPolygonOffsetBackEnable = false;
    mStencilWriteMask = 0xffffffff;
}

/**
 * Applies the whole graphics state to the draw context's command buffer.
 * @param pDrawContext draw context
 */
void GraphicsContext::apply(DrawContext* pDrawContext) const
{
    NVNcommandBuffer* commandBuffer = pDrawContext->getNvnCommandBuffer();

    NVNblendState blendState;
    NVNchannelMaskState channelMaskState;
    NVNcolorState colorState;
    NVNdepthStencilState depthStencilState;
    NVNpolygonState polygonState;
    nvnBlendStateSetDefaults(&blendState);
    nvnColorStateSetDefaults(&colorState);
    if (mBlendEnableMask != 0)
    {
        for (s32 i = 0; i < cRenderTargetNum; i++)
        {
            if (mBlendEnableMask & (1 << i))
            {
                nvnColorStateSetBlendEnable(&colorState, i, true);
                nvnBlendStateSetBlendTarget(&blendState, i);
                const BlendTarget& target = mBlendTargets[i];
                nvnBlendStateSetBlendFunc(
                    &blendState, NVNblendFunc(target.mBlendFactorSrcRGB),
                    NVNblendFunc(target.mBlendFactorDstRGB), NVNblendFunc(target.mBlendFactorSrcA),
                    NVNblendFunc(target.mBlendFactorDstA));
                nvnBlendStateSetBlendEquation(&blendState,
                                              NVNblendEquation(target.mBlendEquationRGB),
                                              NVNblendEquation(target.mBlendEquationA));
                nvnCommandBufferBindBlendState(commandBuffer, &blendState);
            }
        }
    }
    nvnColorStateSetLogicOp(&colorState, NVN_LOGIC_OP_COPY);
    nvnCommandBufferBindColorState(commandBuffer, &colorState);

    nvnCommandBufferSetBlendColor(commandBuffer, &mBlendConstantColor.r);

    for (s32 i = 0; i < cRenderTargetNum; i++)
    {
        nvnChannelMaskStateSetChannelMask(
            &channelMaskState, i, (mColorMask >> (i * 4)) & 1, (mColorMask >> (i * 4 + 1)) & 1,
            (mColorMask >> (i * 4 + 2)) & 1, (mColorMask >> (i * 4 + 3)) & 1);
    }
    nvnCommandBufferBindChannelMaskState(commandBuffer, &channelMaskState);

    nvnDepthStencilStateSetDepthTestEnable(&depthStencilState, mDepthTestEnable);
    nvnDepthStencilStateSetDepthWriteEnable(&depthStencilState, mDepthWriteEnable);
    nvnDepthStencilStateSetDepthFunc(&depthStencilState, NVNdepthFunc(mDepthFunc));
    nvnDepthStencilStateSetStencilTestEnable(&depthStencilState, mStencilTestEnable);
    nvnDepthStencilStateSetStencilFunc(&depthStencilState, NVN_FACE_FRONT_AND_BACK,
                                       NVNstencilFunc(mStencilTestFunc));
    nvnDepthStencilStateSetStencilOp(&depthStencilState, NVN_FACE_FRONT_AND_BACK,
                                     NVNstencilOp(mStencilOpFail), NVNstencilOp(mStencilOpZFail),
                                     NVNstencilOp(mStencilOpZPass));
    nvnCommandBufferBindDepthStencilState(commandBuffer, &depthStencilState);
    if (mStencilTestEnable)
    {
        nvnCommandBufferSetStencilValueMask(commandBuffer, NVN_FACE_FRONT_AND_BACK,
                                            mStencilTestMask);
        nvnCommandBufferSetStencilMask(commandBuffer, NVN_FACE_FRONT_AND_BACK, 0xff);
        nvnCommandBufferSetStencilRef(commandBuffer, NVN_FACE_FRONT_AND_BACK, mStencilTestRef);
    }

    nvnPolygonStateSetCullFace(&polygonState, NVNface(mCullingMode));
    nvnPolygonStateSetFrontFace(&polygonState, NVN_FRONT_FACE_CCW);
    nvnPolygonStateSetPolygonMode(&polygonState, NVNpolygonMode(mPolygonModeFront));
    nvnPolygonStateSetPolygonOffsetEnables(
        &polygonState, (mPolygonOffsetFrontEnable & 1) << 2);
    nvnCommandBufferBindPolygonState(commandBuffer, &polygonState);
}

/**
 * Applies the alpha test state; not supported on this backend.
 * @param pDrawContext draw context
 */
void GraphicsContext::applyAlphaTest(DrawContext* pDrawContext) const {}

/**
 * Applies the depth and stencil test state.
 * @param pDrawContext draw context
 */
void GraphicsContext::applyDepthAndStencilTest(DrawContext* pDrawContext) const
{
    NVNdepthStencilState depthStencilState;
    nvnDepthStencilStateSetDepthTestEnable(&depthStencilState, mDepthTestEnable);
    nvnDepthStencilStateSetDepthWriteEnable(&depthStencilState, mDepthWriteEnable);
    nvnDepthStencilStateSetDepthFunc(&depthStencilState, NVNdepthFunc(mDepthFunc));
    nvnDepthStencilStateSetStencilTestEnable(&depthStencilState, mStencilTestEnable);
    nvnDepthStencilStateSetStencilFunc(&depthStencilState, NVN_FACE_FRONT_AND_BACK,
                                       NVNstencilFunc(mStencilTestFunc));
    nvnDepthStencilStateSetStencilOp(&depthStencilState, NVN_FACE_FRONT_AND_BACK,
                                     NVNstencilOp(mStencilOpFail), NVNstencilOp(mStencilOpZFail),
                                     NVNstencilOp(mStencilOpZPass));
    nvnCommandBufferBindDepthStencilState(pDrawContext->getNvnCommandBuffer(),
                                          &depthStencilState);
    if (mStencilTestEnable)
    {
        nvnCommandBufferSetStencilValueMask(pDrawContext->getNvnCommandBuffer(),
                                            NVN_FACE_FRONT_AND_BACK, mStencilTestMask);
        nvnCommandBufferSetStencilMask(pDrawContext->getNvnCommandBuffer(),
                                       NVN_FACE_FRONT_AND_BACK, 0xff);
        nvnCommandBufferSetStencilRef(pDrawContext->getNvnCommandBuffer(),
                                      NVN_FACE_FRONT_AND_BACK, mStencilTestRef);
    }
}

/**
 * Applies the per render target color write masks.
 * @param pDrawContext draw context
 */
void GraphicsContext::applyColorMask(DrawContext* pDrawContext) const
{
    NVNchannelMaskState channelMaskState;
    for (s32 i = 0; i < cRenderTargetNum; i++)
    {
        nvnChannelMaskStateSetChannelMask(
            &channelMaskState, i, (mColorMask >> (i * 4)) & 1, (mColorMask >> (i * 4 + 1)) & 1,
            (mColorMask >> (i * 4 + 2)) & 1, (mColorMask >> (i * 4 + 3)) & 1);
    }
    nvnCommandBufferBindChannelMaskState(pDrawContext->getNvnCommandBuffer(), &channelMaskState);
}

/**
 * Applies the per render target blend state.
 * @param pDrawContext draw context
 */
void GraphicsContext::applyBlendAndFastZ(DrawContext* pDrawContext) const
{
    NVNcommandBuffer* commandBuffer = pDrawContext->getNvnCommandBuffer();

    NVNblendState blendState;
    nvnBlendStateSetDefaults(&blendState);
    NVNcolorState colorState;
    nvnColorStateSetDefaults(&colorState);
    if (mBlendEnableMask != 0)
    {
        for (s32 i = 0; i < cRenderTargetNum; i++)
        {
            if (mBlendEnableMask & (1 << i))
            {
                nvnColorStateSetBlendEnable(&colorState, i, true);
                nvnBlendStateSetBlendTarget(&blendState, i);
                const BlendTarget& target = mBlendTargets[i];
                nvnBlendStateSetBlendFunc(
                    &blendState, NVNblendFunc(target.mBlendFactorSrcRGB),
                    NVNblendFunc(target.mBlendFactorDstRGB), NVNblendFunc(target.mBlendFactorSrcA),
                    NVNblendFunc(target.mBlendFactorDstA));
                nvnBlendStateSetBlendEquation(&blendState,
                                              NVNblendEquation(target.mBlendEquationRGB),
                                              NVNblendEquation(target.mBlendEquationA));
                nvnCommandBufferBindBlendState(commandBuffer, &blendState);
            }
        }
    }
    nvnColorStateSetLogicOp(&colorState, NVN_LOGIC_OP_COPY);
    nvnCommandBufferBindColorState(commandBuffer, &colorState);
}

/**
 * Applies the blend constant color.
 * @param pDrawContext draw context
 */
void GraphicsContext::applyBlendConstantColor(DrawContext* pDrawContext) const
{
    nvnCommandBufferSetBlendColor(pDrawContext->getNvnCommandBuffer(), &mBlendConstantColor.r);
}

/**
 * Applies the culling, polygon mode and polygon offset state.
 * @param pDrawContext draw context
 */
void GraphicsContext::applyCullingAndPolygonModeAndPolygonOffset(DrawContext* pDrawContext) const
{
    NVNpolygonState polygonState;
    nvnPolygonStateSetCullFace(&polygonState, NVNface(mCullingMode));
    nvnPolygonStateSetFrontFace(&polygonState, NVN_FRONT_FACE_CCW);
    nvnPolygonStateSetPolygonMode(&polygonState, NVNpolygonMode(mPolygonModeFront));
    nvnPolygonStateSetPolygonOffsetEnables(
        &polygonState, (mPolygonOffsetFrontEnable & 1) << 2);
    nvnCommandBufferBindPolygonState(pDrawContext->getNvnCommandBuffer(), &polygonState);
}

}  // namespace sead
