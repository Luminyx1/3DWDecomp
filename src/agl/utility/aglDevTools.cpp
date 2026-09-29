#include "utility/aglDevTools.h"

#include <cmath>

#include <controller/seadController.h>
#include <gfx/seadCamera.h>
#include <gfx/seadColor.h>
#include <gfx/seadGraphicsContext.h>
#include <gfx/seadProjection.h>
#include <gfx/seadViewport.h>
#include <math/seadMatrix.h>
#include <math/seadQuat.h>
#include <prim/seadRuntimeTypeInfo.h>

#include "common/aglDrawContext.h"
#include "common/aglRenderBuffer.h"
#include "common/aglRenderTarget.h"
#include "common/aglShaderProgram.h"
#include "common/aglTextureSampler.h"
#include "common/aglIndexStream.h"
#include "common/aglVertexAttribute.h"
#include "detail/aglPrivateResource.h"
#include "detail/aglShaderHolder.h"
#include "driver/aglGraphicsDriverMgr.h"
#include "utility/aglPrimitiveShape.h"
#include "utility/aglVertexAttributeHolder.h"

namespace agl::utl {

namespace {

bool sIsStickReverse = true;
bool sIsRotateUDReverse = true;
bool sIsRotateLRReverse = true;
f32 sMeterScale = 1.0f;
f32 sFrameSpeed = 1.0f;
f32 sCameraOperationSpeed = 1.0f;

template <typename T>
inline void setUniform(DrawContext* pDrawContext, const UniformLocation& rLocation,
                       const T& rValue)
{
    if (rLocation.isValid())
    {
        rLocation.setUniformNVN(pDrawContext, sizeof(T) / sizeof(u32), &rValue);
    }
}

inline void setUniform(DrawContext* pDrawContext, const UniformLocation& rLocation, f32 value)
{
    if (rLocation.isValid())
    {
        rLocation.setUniformNVN(pDrawContext, 1, &value);
    }
}

inline void setUniform(DrawContext* pDrawContext, const UniformLocation& rLocation, s32 value)
{
    if (rLocation.isValid())
    {
        rLocation.setUniformNVN(pDrawContext, 1, &value);
    }
}

inline void setUniform(DrawContext* pDrawContext, const UniformLocation& rLocation, u32 value)
{
    if (rLocation.isValid())
    {
        rLocation.setUniformNVN(pDrawContext, 1, &value);
    }
}

inline void setUniform(DrawContext* pDrawContext, const UniformLocation& rLocation,
                       const sead::Vector3f* pValues, s32 num)
{
    if (num == 1)
    {
        setUniform(pDrawContext, rLocation, *pValues);
    }
    else if (rLocation.isValid())
    {
        sead::Vector4f values[256];
        for (s32 i = 0; i < num; i++)
        {
            __builtin_memcpy(&values[i], &pValues[i], sizeof(sead::Vector3f));
            values[i].w = 0.0f;
        }
        rLocation.setUniformNVN(pDrawContext, num * 4, values);
    }
}

inline u32 getSlice(const TextureData& rTexture)
{
    s32 min = rTexture.getMinSlice_();
    s32 slice = rTexture.getDepth();
    return min > slice ? min : slice;
}

inline void drawIndexStream(DrawContext* pDrawContext, const IndexStream& rStream)
{
    u32 count = rStream.getCount();
    if (count != 0)
    {
        NVNdrawPrimitive primitive = rStream.getPrimitiveType();
        NVNcommandBuffer* pCommandBuffer = pDrawContext->getNvnCommandBuffer();
        NVNbufferAddress address = nvnBufferGetAddress(rStream.getNvnBuffer());
        nvnCommandBufferDrawElements(pCommandBuffer, primitive, NVNindexType(rStream.getFormat()),
                                     count, address);
    }
}

inline void drawArrays(DrawContext* pDrawContext, const IndexStream& rStream, u32 count)
{
    if (count != 0)
    {
        NVNdrawPrimitive primitive = rStream.getPrimitiveType();
        nvnCommandBufferDrawArrays(pDrawContext->getNvnCommandBuffer(), primitive, 0, count);
    }
}

inline void drawIndexStreamInstanced(DrawContext* pDrawContext, const IndexStream& rStream,
                                     s32 instanceNum)
{
    u32 count = rStream.getCount();
    if (count != 0)
    {
        NVNdrawPrimitive primitive = rStream.getPrimitiveType();
        NVNcommandBuffer* pCommandBuffer = pDrawContext->getNvnCommandBuffer();
        NVNbufferAddress address = nvnBufferGetAddress(rStream.getNvnBuffer());
        if (instanceNum > 1)
        {
            nvnCommandBufferDrawElementsInstanced(pCommandBuffer, primitive,
                                                  NVNindexType(rStream.getFormat()), count,
                                                  address, 0, 0, instanceNum);
        }
        else
        {
            nvnCommandBufferDrawElements(pCommandBuffer, primitive,
                                         NVNindexType(rStream.getFormat()), count, address);
        }
    }
}

inline void drawQuad(DrawContext* pDrawContext)
{
    VertexAttributeHolder::instance()
        ->getVertexAttribute(VertexAttributeHolder::cAttribute_Quad)
        .activate(pDrawContext);
    drawIndexStream(pDrawContext, PrimitiveShape::instance()->getQuadIndexStream());
}

inline void mulMtx44(sead::Matrix44f* pOut, const sead::Matrix44f& rA, const sead::Matrix44f& rB)
{
    float32x4_t a0 = vld1q_f32(rA.m[0]);
    float32x4_t a1 = vld1q_f32(rA.m[1]);
    float32x4_t a2 = vld1q_f32(rA.m[2]);
    float32x4_t a3 = vld1q_f32(rA.m[3]);

    float32x4_t b0 = vld1q_f32(rB.m[0]);
    float32x4_t b1 = vld1q_f32(rB.m[1]);
    float32x4_t b2 = vld1q_f32(rB.m[2]);
    float32x4_t b3 = vld1q_f32(rB.m[3]);

    float32x4_t c0 = vmulq_laneq_f32(b0, a0, 0);
    c0 = vfmaq_laneq_f32(c0, b1, a0, 1);
    c0 = vfmaq_laneq_f32(c0, b2, a0, 2);
    c0 = vfmaq_laneq_f32(c0, b3, a0, 3);
    float32x4_t c1 = vmulq_laneq_f32(b0, a1, 0);
    c1 = vfmaq_laneq_f32(c1, b1, a1, 1);
    c1 = vfmaq_laneq_f32(c1, b2, a1, 2);
    c1 = vfmaq_laneq_f32(c1, b3, a1, 3);
    float32x4_t c2 = vmulq_laneq_f32(b0, a2, 0);
    c2 = vfmaq_laneq_f32(c2, b1, a2, 1);
    c2 = vfmaq_laneq_f32(c2, b2, a2, 2);
    c2 = vfmaq_laneq_f32(c2, b3, a2, 3);
    float32x4_t c3 = vmulq_laneq_f32(b0, a3, 0);
    c3 = vfmaq_laneq_f32(c3, b1, a3, 1);
    c3 = vfmaq_laneq_f32(c3, b2, a3, 2);
    c3 = vfmaq_laneq_f32(c3, b3, a3, 3);

    vst1q_f32(pOut->m[0], c0);
    vst1q_f32(pOut->m[1], c1);
    vst1q_f32(pOut->m[2], c2);
    vst1q_f32(pOut->m[3], c3);
}

inline ShaderProgram* getProgram(s32 type)
{
    return detail::ShaderHolder::instance()->getShaderProgram(type);
}

inline ShaderProgram* getProgramUnsafe(s32 type)
{
    return detail::ShaderHolder::instance()->getShaderProgramUnsafe(type);
}

}  // namespace

/**
 * Sets the scale of one meter in world units.
 * @param scale world units per meter
 */
void DevTools::setMeterScale(f32 scale)
{
    sMeterScale = scale;
}

/**
 * Returns the scale of one meter in world units.
 * @return world units per meter
 */
f32 DevTools::getMeterScale()
{
    return sMeterScale;
}

/**
 * Converts meters to world units.
 * @param meter length in meters
 * @return length in world units
 */
f32 DevTools::calcScale(f32 meter)
{
    return sMeterScale * meter;
}

/**
 * Converts world units to meters.
 * @param scale length in world units
 * @return length in meters
 */
f32 DevTools::calcMeter(f32 scale)
{
    return scale / sMeterScale;
}

/**
 * Formats a meter range as world units.
 * @param min range minimum in meters
 * @param max range maximum in meters
 * @return the formatted range
 */
sead::FixedSafeString<256> DevTools::getStringMinMax(f32 min, f32 max)
{
    sead::FormatFixedSafeString<256> str("Min=%f,Max=%f", calcScale(min), calcScale(max));
    return str;
}

/**
 * Sets the debug frame speed.
 * @param speed frame speed
 */
void DevTools::setFrameSpeed(f32 speed)
{
    sFrameSpeed = speed;
}

/**
 * Returns the debug frame speed.
 * @return frame speed
 */
f32 DevTools::getFrameSpeed()
{
    return sFrameSpeed;
}

/**
 * Sets the debug camera operation speed.
 * @param speed camera operation speed
 */
void DevTools::setCameraOperationSpeed(f32 speed)
{
    sCameraOperationSpeed = speed;
}

/**
 * Returns the debug camera operation speed.
 * @return camera operation speed
 */
f32 DevTools::getCameraOperationSpeed()
{
    return sCameraOperationSpeed;
}

/**
 * Does nothing.
 * @param pContext unused
 */
void DevTools::genMessage(sead::hostio::Context* pContext) {}

/**
 * Draws a textured quad, multiplying the texture by a color.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 * @param rModelMtx model matrix of the quad
 * @param rProjMtx projection matrix
 * @param rColor color to multiply by
 */
void DevTools::drawTexture(DrawContext* pDrawContext, const TextureSampler& rSampler,
                           const sead::Matrix34f& rModelMtx, const sead::Matrix44f& rProjMtx,
                           const sead::Color4f& rColor)
{
    const ShaderProgram* pProgram = detail::ShaderHolder::instance()->getShaderProgram(
        rColor == sead::Color4f::cWhite ? detail::ShaderHolder::cTexture :
                                          detail::ShaderHolder::cTextureMultColor);
    pProgram->activate(pDrawContext, true);
    drawTexture_(pDrawContext, *pProgram, rSampler, rModelMtx, rProjMtx, rColor);
}

/**
 * Activates the immediate drawing shader with a view and projection matrix.
 * @param pDrawContext draw context
 * @param rViewMtx view matrix
 * @param rProjMtx projection matrix
 */
void DevTools::beginDrawImm(DrawContext* pDrawContext, const sead::Matrix34f& rViewMtx,
                            const sead::Matrix44f& rProjMtx)
{
    const ShaderProgram* pProgram = getProgram(detail::ShaderHolder::cDrawImm);
    pProgram->activate(pDrawContext, true);
    setUniform(pDrawContext, pProgram->getUniformLocation(0), sead::Matrix34f::ident);
    setUniform(pDrawContext, pProgram->getUniformLocation(1), rViewMtx);
    setUniform(pDrawContext, pProgram->getUniformLocation(2), rProjMtx);
    VertexAttribute::disableAttributeAll(pDrawContext);
}

/**
 * Draws a line with the immediate drawing shader.
 * @param pDrawContext draw context
 * @param rStart line start
 * @param rEnd line end
 * @param rColor line color
 * @param width line width
 */
void DevTools::drawLineImm(DrawContext* pDrawContext, const sead::Vector3f& rStart,
                           const sead::Vector3f& rEnd, const sead::Color4f& rColor, f32 width)
{
    const ShaderProgram* pProgram = getProgram(detail::ShaderHolder::cDrawImm);
    setUniform(pDrawContext, pProgram->getUniformLocation(3), rColor);
    sead::Vector3f pos[2] = {rStart, rEnd};
    setUniform(pDrawContext, pProgram->getUniformLocation(4), pos, 2);

    IndexStream stream;
    stream.setPrimitiveType(NVN_DRAW_PRIMITIVE_LINES);
    driver::GraphicsDriverMgr::instance()->setLineWidth(pDrawContext, width);
    NVNdrawPrimitive primitive = stream.getPrimitiveType();
    nvnCommandBufferDrawArrays(pDrawContext->getNvnCommandBuffer(), primitive, 0, 2);
}

/**
 * Draws a point with the immediate drawing shader.
 * @param pDrawContext draw context
 * @param rPos point position
 * @param rColor point color
 * @param size point size
 */
void DevTools::drawPointImm(DrawContext* pDrawContext, const sead::Vector3f& rPos,
                            const sead::Color4f& rColor, f32 size)
{
    const ShaderProgram* pProgram = getProgram(detail::ShaderHolder::cDrawImm);
    setUniform(pDrawContext, pProgram->getUniformLocation(3), rColor);
    setUniform(pDrawContext, pProgram->getUniformLocation(4), &rPos, 1);

    IndexStream stream;
    stream.setPrimitiveType(NVN_DRAW_PRIMITIVE_POINTS);
    driver::GraphicsDriverMgr::instance()->setPointSize(pDrawContext, size);
    NVNdrawPrimitive primitive = stream.getPrimitiveType();
    nvnCommandBufferDrawArrays(pDrawContext->getNvnCommandBuffer(), primitive, 0, 1);
}

/**
 * Draws a filled triangle with the immediate drawing shader.
 * @param pDrawContext draw context
 * @param rPos0 first vertex
 * @param rPos1 second vertex
 * @param rPos2 third vertex
 * @param rColor triangle color
 */
void DevTools::drawTriangleImm(DrawContext* pDrawContext, const sead::Vector3f& rPos0,
                               const sead::Vector3f& rPos1, const sead::Vector3f& rPos2,
                               const sead::Color4f& rColor)
{
    const ShaderProgram* pProgram = getProgram(detail::ShaderHolder::cDrawImm);
    setUniform(pDrawContext, pProgram->getUniformLocation(3), rColor);
    sead::Vector3f pos[3] = {rPos0, rPos1, rPos2};
    setUniform(pDrawContext, pProgram->getUniformLocation(4), pos, 3);

    IndexStream stream;
    stream.setPrimitiveType(NVN_DRAW_PRIMITIVE_TRIANGLES);
    NVNdrawPrimitive primitive = stream.getPrimitiveType();
    nvnCommandBufferDrawArrays(pDrawContext->getNvnCommandBuffer(), primitive, 0, 3);
}

/**
 * Draws a triangle outline with the immediate drawing shader.
 * @param pDrawContext draw context
 * @param rPos0 first vertex
 * @param rPos1 second vertex
 * @param rPos2 third vertex
 * @param rColor line color
 * @param width line width
 */
void DevTools::drawWireTriangleImm(DrawContext* pDrawContext, const sead::Vector3f& rPos0,
                                   const sead::Vector3f& rPos1, const sead::Vector3f& rPos2,
                                   const sead::Color4f& rColor, f32 width)
{
    const ShaderProgram* pProgram = getProgram(detail::ShaderHolder::cDrawImm);
    setUniform(pDrawContext, pProgram->getUniformLocation(3), rColor);
    sead::Vector3f pos[3] = {rPos0, rPos1, rPos2};
    setUniform(pDrawContext, pProgram->getUniformLocation(4), pos, 3);

    IndexStream stream;
    stream.setPrimitiveType(NVN_DRAW_PRIMITIVE_LINE_LOOP);
    driver::GraphicsDriverMgr::instance()->setLineWidth(pDrawContext, width);
    NVNdrawPrimitive primitive = stream.getPrimitiveType();
    nvnCommandBufferDrawArrays(pDrawContext->getNvnCommandBuffer(), primitive, 0, 3);
}

/**
 * Draws one channel of a render buffer's color or depth target into a viewport.
 * @param pDrawContext draw context
 * @param rRenderBuffer render buffer to draw
 * @param rViewport viewport to draw into
 * @param channel channel to draw; depth draws the depth target
 */
void DevTools::drawFrameBuffer(DrawContext* pDrawContext, const RenderBuffer& rRenderBuffer,
                               const sead::Viewport& rViewport, ImageFilter2D::Channel channel)
{
    sead::GraphicsContext graphicsContext;
    graphicsContext.setDepthEnable(false, false);
    graphicsContext.setBlendEnable(false);
    graphicsContext.setColorMask(true, true, true, false);
    graphicsContext.apply(pDrawContext);

    rRenderBuffer.getRenderTargetDepth()->expandHiZBuffer(pDrawContext);

    TextureSampler sampler;
    if (channel < ImageFilter2D::cChannel_Depth)
    {
        const RenderTargetColor* pColor = rRenderBuffer.getRenderTargetColor();
        if (pColor == nullptr)
        {
            return;
        }
        sampler.applyTextureData(*pColor);
    }
    else
    {
        const RenderTargetDepth* pDepth = rRenderBuffer.getRenderTargetDepth();
        if (pDepth == nullptr)
        {
            return;
        }
        sampler.applyTextureData(*pDepth);
    }

    sampler.setFilter(0, 0, 0);
    sampler.setWrap(7, 7, 7);
    ImageFilter2D::drawTextureChannel(pDrawContext, sampler, rViewport, channel,
                                      sead::Vector2f::ones, sead::Vector2f::zero);
}

/**
 * Draws a quad filled with a color.
 * @param pDrawContext draw context
 * @param rColor fill color
 * @param rModelMtx model matrix of the quad
 * @param rProjMtx projection matrix
 */
void DevTools::drawColorQuad(DrawContext* pDrawContext, const sead::Color4f& rColor,
                             const sead::Matrix34f& rModelMtx, const sead::Matrix44f& rProjMtx)
{
    const ShaderProgram* pProgram =
        getProgramUnsafe(detail::ShaderHolder::cColorQuad)->getVariation(0);
    pProgram->activate(pDrawContext, true);
    sead::Matrix44f mvp;
    mvp.setMul(rProjMtx, rModelMtx);
    setUniform(pDrawContext, pProgram->getUniformLocation(0), mvp);
    setUniform(pDrawContext, pProgram->getUniformLocation(8), rColor);
    drawQuad(pDrawContext);
}

/**
 * Draws a quad with a vertical color gradient.
 * @param pDrawContext draw context
 * @param rTopColor color at the top edge
 * @param rBottomColor color at the bottom edge
 * @param rModelMtx model matrix of the quad
 * @param rProjMtx projection matrix
 */
void DevTools::drawColorQuadTopBottom(DrawContext* pDrawContext, const sead::Color4f& rTopColor,
                                      const sead::Color4f& rBottomColor,
                                      const sead::Matrix34f& rModelMtx,
                                      const sead::Matrix44f& rProjMtx)
{
    const ShaderProgram* pProgram = getProgram(detail::ShaderHolder::cTopBottomColor);
    pProgram->activate(pDrawContext, true);
    sead::Matrix44f mvp;
    mvp.setMul(rProjMtx, rModelMtx);
    setUniform(pDrawContext, pProgram->getUniformLocation(0), mvp);
    setUniform(pDrawContext, pProgram->getUniformLocation(15), rTopColor);
    setUniform(pDrawContext, pProgram->getUniformLocation(16), rBottomColor);
    drawQuad(pDrawContext);
}

/**
 * Draws a textured quad with an activated shader program.
 * @param pDrawContext draw context
 * @param rProgram activated shader program
 * @param rSampler texture to draw
 * @param rModelMtx model matrix of the quad
 * @param rProjMtx projection matrix
 * @param rColor color to multiply by
 */
void DevTools::drawTexture_(DrawContext* pDrawContext, const ShaderProgram& rProgram,
                            const TextureSampler& rSampler, const sead::Matrix34f& rModelMtx,
                            const sead::Matrix44f& rProjMtx, const sead::Color4f& rColor)
{
    rProgram.validate_();
    rSampler.activate(pDrawContext, rProgram.getSamplerLocation(0), -1, false);
    sead::Matrix44f mvp;
    mvp.setMul(rProjMtx, rModelMtx);
    setUniform(pDrawContext, rProgram.getUniformLocation(0), mvp);
    setUniform(pDrawContext, rProgram.getUniformLocation(8), rColor);
    drawQuad(pDrawContext);
}

/**
 * Draws a single channel of a texture.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 * @param rModelMtx model matrix of the quad
 * @param rProjMtx projection matrix
 * @param channel channel to draw
 */
void DevTools::drawTextureChannel(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                  const sead::Matrix34f& rModelMtx,
                                  const sead::Matrix44f& rProjMtx, ImageFilter2D::Channel channel)
{
    s32 type;
    switch (channel)
    {
    case ImageFilter2D::cChannel_Green:
        type = detail::ShaderHolder::cGreen;
        break;
    case ImageFilter2D::cChannel_Blue:
        type = detail::ShaderHolder::cBlue;
        break;
    case ImageFilter2D::cChannel_Alpha:
        type = detail::ShaderHolder::cAlpha;
        break;
    case ImageFilter2D::cChannel_Depth:
        type = detail::ShaderHolder::cDepthRaw;
        break;
    default:
        type = detail::ShaderHolder::cRed;
        break;
    }

    const ShaderProgram* pProgram = getProgram(type);
    pProgram->activate(pDrawContext, true);
    drawTexture_(pDrawContext, *pProgram, rSampler, rModelMtx, rProjMtx, sead::Color4f::cWhite);
}

/**
 * Draws a textured quad with gamma correction.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 * @param rModelMtx model matrix of the quad
 * @param rProjMtx projection matrix
 * @param gamma gamma value
 */
void DevTools::drawTextureGamma(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                const sead::Matrix34f& rModelMtx, const sead::Matrix44f& rProjMtx,
                                f32 gamma)
{
    if (gamma == 1.0f)
    {
        drawTexture(pDrawContext, rSampler, rModelMtx, rProjMtx, sead::Color4f::cWhite);
        return;
    }

    const ShaderProgram* pProgram =
        getProgramUnsafe(detail::ShaderHolder::cTextureGamma)->getVariation(0);
    pProgram->activate(pDrawContext, true);
    setUniform(pDrawContext, pProgram->getUniformLocation(23), gamma);
    drawTexture_(pDrawContext, *pProgram, rSampler, rModelMtx, rProjMtx, sead::Color4f::cWhite);
}

/**
 * Draws one mip level of a texture.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 * @param rModelMtx model matrix of the quad
 * @param rProjMtx projection matrix
 * @param mipLevel mip level to draw
 */
void DevTools::drawTextureMipLevel(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                   const sead::Matrix34f& rModelMtx,
                                   const sead::Matrix44f& rProjMtx, f32 mipLevel)
{
    const ShaderProgram* pProgram =
        getProgramUnsafe(detail::ShaderHolder::cTexture)->getVariation(2);
    pProgram->activate(pDrawContext, true);
    setUniform(pDrawContext, pProgram->getUniformLocation(2), mipLevel);
    drawTexture_(pDrawContext, *pProgram, rSampler, rModelMtx, rProjMtx, sead::Color4f::cWhite);
}

/**
 * Draws one slice of a 2D array texture.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 * @param rModelMtx model matrix of the quad
 * @param rProjMtx projection matrix
 * @param slice slice to draw
 * @param mipLevel mip level to draw, or -1 for the default level
 * @param rColor color to multiply by
 */
void DevTools::drawTexture2DArray(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                  const sead::Matrix34f& rModelMtx,
                                  const sead::Matrix44f& rProjMtx, s32 slice, f32 mipLevel,
                                  const sead::Color4f& rColor)
{
    const ShaderProgram* pProgram = getProgramUnsafe(detail::ShaderHolder::cTexture2dArray)
                                        ->getVariation(mipLevel != -1.0f ? 1 : 0);
    pProgram->activate(pDrawContext, true);
    setUniform(pDrawContext, pProgram->getUniformLocation(3), f32(slice));
    setUniform(pDrawContext, pProgram->getUniformLocation(2), mipLevel);
    drawTexture_(pDrawContext, *pProgram, rSampler, rModelMtx, rProjMtx, rColor);
}

/**
 * Draws one depth slice of a 3D texture.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 * @param rModelMtx model matrix of the quad
 * @param rProjMtx projection matrix
 * @param depth depth coordinate to draw
 * @param mipLevel mip level to draw, or -1 for the default level
 */
void DevTools::drawTexture3D(DrawContext* pDrawContext, const TextureSampler& rSampler,
                             const sead::Matrix34f& rModelMtx, const sead::Matrix44f& rProjMtx,
                             f32 depth, f32 mipLevel)
{
    const ShaderProgram* pProgram = getProgramUnsafe(detail::ShaderHolder::cTexture3d)
                                        ->getVariation(mipLevel != -1.0f ? 1 : 0);
    pProgram->activate(pDrawContext, true);
    setUniform(pDrawContext, pProgram->getUniformLocation(3), depth);
    setUniform(pDrawContext, pProgram->getUniformLocation(2), mipLevel);
    drawTexture_(pDrawContext, *pProgram, rSampler, rModelMtx, rProjMtx, sead::Color4f::cWhite);
}

/**
 * Draws one face of a cube map texture.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 * @param rModelMtx model matrix of the quad
 * @param rProjMtx projection matrix
 * @param face face to draw
 * @param mipLevel mip level to draw, or -1 for the default level
 */
void DevTools::drawTextureCubeMap(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                  const sead::Matrix34f& rModelMtx,
                                  const sead::Matrix44f& rProjMtx, CubeMapFace face,
                                  f32 mipLevel)
{
    drawTextureCubeArray(pDrawContext, rSampler, rModelMtx, rProjMtx, 0, face, mipLevel);
}

/**
 * Draws one face of one cube of a cube map array texture.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 * @param rModelMtx model matrix of the quad
 * @param rProjMtx projection matrix
 * @param slice cube to draw
 * @param face face to draw
 * @param mipLevel mip level to draw, or -1 for the default level
 */
void DevTools::drawTextureCubeArray(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                    const sead::Matrix34f& rModelMtx,
                                    const sead::Matrix44f& rProjMtx, s32 slice, CubeMapFace face,
                                    f32 mipLevel)
{
    s32 type = detail::ShaderHolder::cTextureCubeMap;
    const TextureData& rTexture = rSampler.getTextureData();
    if (rTexture.getSurface().mTarget == NVN_TEXTURE_TARGET_CUBEMAP)
    {
        type = getSlice(rTexture) > 6 ? detail::ShaderHolder::cTextureCubeMapArray :
                                        detail::ShaderHolder::cTextureCubeMap;
    }

    const ShaderProgram* pProgram =
        getProgramUnsafe(type)->getVariation(mipLevel != -1.0f ? 1 : 0);
    pProgram->activate(pDrawContext, true);
    setUniform(pDrawContext, pProgram->getUniformLocation(3), f32(slice));
    setUniform(pDrawContext, pProgram->getUniformLocation(17), s32(face));
    setUniform(pDrawContext, pProgram->getUniformLocation(2), mipLevel);
    drawTexture_(pDrawContext, *pProgram, rSampler, rModelMtx, rProjMtx, sead::Color4f::cWhite);
}

/**
 * Draws a texture with transformed texture coordinates.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 * @param rModelMtx model matrix of the quad
 * @param rProjMtx projection matrix
 * @param rTexCoordScale texture coordinate scale
 * @param rotate texture coordinate rotation
 * @param rTexCoordTranslate texture coordinate translation
 */
void DevTools::drawTextureTexCoord(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                   const sead::Matrix34f& rModelMtx,
                                   const sead::Matrix44f& rProjMtx,
                                   const sead::Vector2f& rTexCoordScale, f32 rotate,
                                   const sead::Vector2f& rTexCoordTranslate)
{
    const ShaderProgram* pProgram = getProgram(detail::ShaderHolder::cTextureTexcoord);
    pProgram->activate(pDrawContext, true);
    setUniform(pDrawContext, pProgram->getUniformLocation(4), rTexCoordScale);
    setUniform(pDrawContext, pProgram->getUniformLocation(5), rotate);
    setUniform(pDrawContext, pProgram->getUniformLocation(6), rTexCoordTranslate);
    drawTexture_(pDrawContext, *pProgram, rSampler, rModelMtx, rProjMtx, sead::Color4f::cWhite);
}

/**
 * Draws a texture with transformed texture coordinates, multiplied by a color.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 * @param rModelMtx model matrix of the quad
 * @param rProjMtx projection matrix
 * @param rTexCoordScale texture coordinate scale
 * @param rotate texture coordinate rotation
 * @param rTexCoordTranslate texture coordinate translation
 * @param rColor color to multiply by
 */
void DevTools::drawTextureTexCoordMultColor(DrawContext* pDrawContext,
                                            const TextureSampler& rSampler,
                                            const sead::Matrix34f& rModelMtx,
                                            const sead::Matrix44f& rProjMtx,
                                            const sead::Vector2f& rTexCoordScale, f32 rotate,
                                            const sead::Vector2f& rTexCoordTranslate,
                                            const sead::Color4f& rColor)
{
    const ShaderProgram* pProgram = getProgram(detail::ShaderHolder::cTextureTexcoordMultColor);
    pProgram->activate(pDrawContext, true);
    setUniform(pDrawContext, pProgram->getUniformLocation(4), rTexCoordScale);
    setUniform(pDrawContext, pProgram->getUniformLocation(5), rotate);
    setUniform(pDrawContext, pProgram->getUniformLocation(6), rTexCoordTranslate);
    drawTexture_(pDrawContext, *pProgram, rSampler, rModelMtx, rProjMtx, rColor);
}

/**
 * Draws a multisampled texture with the shader matching its sample count.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 * @param rModelMtx model matrix of the quad
 * @param rProjMtx projection matrix
 */
void DevTools::drawTextureMSAA(DrawContext* pDrawContext, const TextureSampler& rSampler,
                               const sead::Matrix34f& rModelMtx, const sead::Matrix44f& rProjMtx)
{
    s32 type;
    switch (s8(rSampler.getTextureData().getSurface().mSamples))
    {
    case 0:
        type = detail::ShaderHolder::cTextureMultiSample1x;
        break;
    case 2:
        type = detail::ShaderHolder::cTextureMultiSample2x;
        break;
    case 4:
        type = detail::ShaderHolder::cTextureMultiSample4x;
        break;
    case 8:
        type = detail::ShaderHolder::cTextureMultiSample8x;
        break;
    default:
        type = detail::ShaderHolder::cTexture;
        break;
    }

    const ShaderProgram* pProgram = getProgram(type);
    pProgram->activate(pDrawContext, true);
    drawTexture_(pDrawContext, *pProgram, rSampler, rModelMtx, rProjMtx, sead::Color4f::cWhite);
}

/**
 * Draws a YUV NV12 texture pair converted to RGB.
 * @param pDrawContext draw context
 * @param rSamplerY luma texture
 * @param rSamplerUV chroma texture
 * @param rModelMtx model matrix of the quad
 * @param rProjMtx projection matrix
 */
void DevTools::drawNV12Decord(DrawContext* pDrawContext, const TextureSampler& rSamplerY,
                              const TextureSampler& rSamplerUV, const sead::Matrix34f& rModelMtx,
                              const sead::Matrix44f& rProjMtx)
{
    const ShaderProgram* pProgram = getProgram(detail::ShaderHolder::cNv12decode);
    pProgram->activate(pDrawContext, true);
    pProgram->validate_();
    rSamplerUV.activate(pDrawContext, pProgram->getSamplerLocation(1), -1, false);
    drawTexture_(pDrawContext, *pProgram, rSamplerY, rModelMtx, rProjMtx, sead::Color4f::cWhite);
}

/**
 * Draws a texture transformed by a color matrix.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 * @param rModelMtx model matrix of the quad
 * @param rProjMtx projection matrix
 * @param rColorMtx color matrix
 * @param rColorOffset color offset
 * @param mipLevel mip level to draw, or -1 for the default level
 */
void DevTools::drawTextureColorMatrix(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                      const sead::Matrix34f& rModelMtx,
                                      const sead::Matrix44f& rProjMtx,
                                      const sead::Matrix44f& rColorMtx,
                                      const sead::Vector4f& rColorOffset, f32 mipLevel)
{
    const ShaderProgram* pProgram = getProgramUnsafe(detail::ShaderHolder::cTextureClrmtx2d)
                                        ->getVariation(mipLevel != -1.0f ? 1 : 0);
    pProgram->activate(pDrawContext, true);
    setUniform(pDrawContext, pProgram->getUniformLocation(2), mipLevel);
    drawTexture_(pDrawContext, *pProgram, rSampler, rModelMtx, rProjMtx, rColorMtx, rColorOffset);
}

/**
 * Draws a textured quad transformed by a color matrix with an activated shader program.
 * @param pDrawContext draw context
 * @param rProgram activated shader program
 * @param rSampler texture to draw
 * @param rModelMtx model matrix of the quad
 * @param rProjMtx projection matrix
 * @param rColorMtx color matrix
 * @param rColorOffset color offset
 */
void DevTools::drawTexture_(DrawContext* pDrawContext, const ShaderProgram& rProgram,
                            const TextureSampler& rSampler, const sead::Matrix34f& rModelMtx,
                            const sead::Matrix44f& rProjMtx, const sead::Matrix44f& rColorMtx,
                            const sead::Vector4f& rColorOffset)
{
    rProgram.validate_();
    rSampler.activate(pDrawContext, rProgram.getSamplerLocation(0), -1, false);
    sead::Matrix44f mvp;
    mvp.setMul(rProjMtx, rModelMtx);
    setUniform(pDrawContext, rProgram.getUniformLocation(0), mvp);
    setUniform(pDrawContext, rProgram.getUniformLocation(9), rColorMtx);
    setUniform(pDrawContext, rProgram.getUniformLocation(8), rColorOffset);
    drawQuad(pDrawContext);
}

/**
 * Draws one slice of a 2D array texture transformed by a color matrix.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 * @param rModelMtx model matrix of the quad
 * @param rProjMtx projection matrix
 * @param rColorMtx color matrix
 * @param rColorOffset color offset
 * @param slice slice to draw
 * @param mipLevel mip level to draw, or -1 for the default level
 */
void DevTools::drawTexture2DArrayColorMatrix(DrawContext* pDrawContext,
                                             const TextureSampler& rSampler,
                                             const sead::Matrix34f& rModelMtx,
                                             const sead::Matrix44f& rProjMtx,
                                             const sead::Matrix44f& rColorMtx,
                                             const sead::Vector4f& rColorOffset, s32 slice,
                                             f32 mipLevel)
{
    const ShaderProgram* pProgram = getProgramUnsafe(detail::ShaderHolder::cTextureClrmtx2dArray)
                                        ->getVariation(mipLevel != -1.0f ? 1 : 0);
    pProgram->activate(pDrawContext, true);
    setUniform(pDrawContext, pProgram->getUniformLocation(3), f32(slice));
    setUniform(pDrawContext, pProgram->getUniformLocation(2), mipLevel);
    drawTexture_(pDrawContext, *pProgram, rSampler, rModelMtx, rProjMtx, rColorMtx, rColorOffset);
}

/**
 * Sets whether the camera control stick is reversed.
 * @param reverse whether to reverse
 */
void DevTools::setStickReverse(bool reverse)
{
    sIsStickReverse = reverse;
}

/**
 * Returns whether the camera control stick is reversed.
 * @return whether the stick is reversed
 */
bool DevTools::isStickReverse()
{
    return sIsStickReverse;
}

/**
 * Sets whether horizontal camera rotation is reversed.
 * @param reverse whether to reverse
 */
void DevTools::setRotateLRReverse(bool reverse)
{
    sIsRotateLRReverse = reverse;
}

/**
 * Returns whether horizontal camera rotation is reversed.
 * @return whether horizontal rotation is reversed
 */
bool DevTools::isRotateLRReverse()
{
    return sIsRotateLRReverse;
}

/**
 * Sets whether vertical camera rotation is reversed.
 * @param reverse whether to reverse
 */
void DevTools::setRotateUDReverse(bool reverse)
{
    sIsRotateUDReverse = reverse;
}

/**
 * Returns whether vertical camera rotation is reversed.
 * @return whether vertical rotation is reversed
 */
bool DevTools::isRotateUDReverse()
{
    return sIsRotateUDReverse;
}

/**
 * Draws a camera model for a camera object, aiming at its look-at target when available.
 * @param pDrawContext draw context
 * @param rCamera camera to visualize
 * @param rViewMtx view matrix used for drawing
 * @param rProjMtx projection matrix used for drawing
 * @param isFill whether to draw filled faces
 * @param rColor draw color
 * @param size model size
 */
void DevTools::drawCamera(DrawContext* pDrawContext, const sead::Camera& rCamera,
                          const sead::Matrix34f& rViewMtx, const sead::Matrix44f& rProjMtx,
                          bool isFill, const sead::Color4f& rColor, f32 size)
{
    driver::GraphicsDriverMgr::instance()->setLineWidth(pDrawContext, 1.0f);

    sead::Vector3f target;
    if (sead::IsDerivedFrom<sead::LookAtCamera>(&rCamera))
    {
        target = sead::DynamicCast<const sead::LookAtCamera>(&rCamera)->getAt();
    }
    else
    {
        target.set(rViewMtx.m[0][3], rViewMtx.m[1][3], rViewMtx.m[2][3]);
    }

    drawCamera(pDrawContext, rCamera.getMatrix(), target, rViewMtx, rProjMtx, isFill, rColor, size);
}

/**
 * Draws a camera model for a view matrix aiming at the given target.
 * @param pDrawContext draw context
 * @param rCameraMtx camera view matrix
 * @param rTarget camera target position
 * @param rViewMtx view matrix used for drawing
 * @param rProjMtx projection matrix used for drawing
 * @param isFill whether to draw filled faces
 * @param rColor draw color
 * @param size model size
 */
void DevTools::drawCamera(DrawContext* pDrawContext, const sead::Matrix34f& rCameraMtx,
                          const sead::Vector3f& rTarget, const sead::Matrix34f& rViewMtx,
                          const sead::Matrix44f& rProjMtx, bool isFill,
                          const sead::Color4f& rColor, f32 size)
{
    sead::GraphicsContext context;
    context.apply(pDrawContext);

    sead::Matrix34f cameraMtx;
    cameraMtx.setInverse(rCameraMtx);
    drawCamera_(pDrawContext, cameraMtx, rTarget, rViewMtx, rProjMtx, isFill, rColor, size);
}

/**
 * Draws the wireframe frustum of a camera object and projection.
 * @param pDrawContext draw context
 * @param rCamera camera to visualize
 * @param rProjection projection of the camera
 * @param rViewMtx view matrix used for drawing
 * @param rProjMtx projection matrix used for drawing
 * @param isFill unused
 * @param rColor draw color
 */
void DevTools::drawFrustum(DrawContext* pDrawContext, const sead::Camera& rCamera,
                           const sead::Projection& rProjection, const sead::Matrix34f& rViewMtx,
                           const sead::Matrix44f& rProjMtx, bool isFill,
                           const sead::Color4f& rColor)
{
    sead::GraphicsContext context;
    context.apply(pDrawContext);

    sead::Matrix34f cameraMtx;
    cameraMtx.setInverse(rCamera.getMatrix());
    drawFrustum_(pDrawContext, cameraMtx, rProjection.getProjectionMatrix(), rViewMtx, rProjMtx,
                 false, rColor);
}

/**
 * Draws the wireframe frustum of a camera view matrix and projection.
 * @param pDrawContext draw context
 * @param rCameraMtx camera view matrix
 * @param rProjection projection of the camera
 * @param rViewMtx view matrix used for drawing
 * @param rProjMtx projection matrix used for drawing
 * @param isFill unused
 * @param rColor draw color
 */
void DevTools::drawFrustum(DrawContext* pDrawContext, const sead::Matrix34f& rCameraMtx,
                           const sead::Projection& rProjection, const sead::Matrix34f& rViewMtx,
                           const sead::Matrix44f& rProjMtx, bool isFill,
                           const sead::Color4f& rColor)
{
    sead::GraphicsContext context;
    context.apply(pDrawContext);

    sead::Matrix34f cameraMtx;
    cameraMtx.setInverse(rCameraMtx);
    drawFrustum_(pDrawContext, cameraMtx, rProjection.getProjectionMatrix(), rViewMtx, rProjMtx,
                 false, rColor);
}

/**
 * Draws a camera model together with its wireframe frustum.
 * @param pDrawContext draw context
 * @param rCamera camera to visualize
 * @param rProjection projection of the camera
 * @param rViewMtx view matrix used for drawing
 * @param rProjMtx projection matrix used for drawing
 * @param isFill whether to draw the camera model filled
 * @param rCameraColor camera model color
 * @param rFrustumColor frustum color
 * @param size camera model size
 */
void DevTools::drawCameraAndFrustum(DrawContext* pDrawContext, const sead::Camera& rCamera,
                                    const sead::Projection& rProjection,
                                    const sead::Matrix34f& rViewMtx,
                                    const sead::Matrix44f& rProjMtx, bool isFill,
                                    const sead::Color4f& rCameraColor,
                                    const sead::Color4f& rFrustumColor, f32 size)
{
    sead::Matrix34f cameraMtx;
    cameraMtx.setInverse(rCamera.getMatrix());

    sead::Vector3f target;
    if (sead::IsDerivedFrom<sead::LookAtCamera>(&rCamera))
    {
        target = sead::DynamicCast<const sead::LookAtCamera>(&rCamera)->getAt();
    }
    else
    {
        target.set(cameraMtx.m[0][3], cameraMtx.m[1][3], cameraMtx.m[2][3]);
    }

    drawCamera_(pDrawContext, cameraMtx, target, rViewMtx, rProjMtx, isFill, rCameraColor, size);
    drawFrustum_(pDrawContext, cameraMtx, rProjection.getProjectionMatrix(), rViewMtx, rProjMtx,
                 false, rFrustumColor);
}

/**
 * Moves a look-at camera using the sticks and buttons of a controller.
 * @param pCamera camera to control
 * @param rController controller providing the input
 * @param speed operation speed
 * @param type camera control type
 */
void DevTools::controlCamera(sead::LookAtCamera* pCamera, const sead::Controller& rController,
                             f32 speed, CameraControlType type)
{
    const sead::Vector2f& rLeftStick = rController.getLeftStick();
    f32 zoom = rController.isHold(1 << sead::Controller::cPadIdx_B) ?
                   1.0f :
                   (rController.isHold(1 << sead::Controller::cPadIdx_X) ? -1.0f : 0.0f);
    f32 moveUD = rController.isHold(1 << sead::Controller::cPadIdx_Up) ?
                     1.0f :
                     (rController.isHold(1 << sead::Controller::cPadIdx_Down) ? -1.0f : 0.0f);
    f32 moveLR = rController.isHold(1 << sead::Controller::cPadIdx_Right) ?
                     1.0f :
                     (rController.isHold(1 << sead::Controller::cPadIdx_Left) ? -1.0f : 0.0f);
    controlCamera(pCamera, rLeftStick, rController.getRightStick(), zoom, moveUD, moveLR, speed,
                  true, type);
}

/**
 * Does nothing.
 * @param pCamera unused
 * @param pBuffer unused
 * @param rViewport unused
 */
void DevTools::controlCameraPointer(sead::LookAtCamera* pCamera, PoiningControlBuffer* pBuffer,
                                    const sead::Viewport& rViewport)
{
}

/**
 * Does nothing.
 * @param pCamera unused
 * @param pBuffer unused
 * @param width unused
 * @param height unused
 */
void DevTools::controlCameraPointer(sead::LookAtCamera* pCamera, PoiningControlBuffer* pBuffer,
                                    f32 width, f32 height)
{
}

/**
 * Moves a look-at camera from pointer button states and screen position.
 * @param pCamera camera to control
 * @param pBuffer pointer state kept between calls
 * @param isPress whether the pointer is pressed
 * @param isLeft whether the left button is held
 * @param isRight whether the right button is held
 * @param isMiddle whether the middle button is held
 * @param rPos pointer position in screen coordinates
 * @param isInside whether the pointer is inside the controlled area
 * @param width screen width
 * @param height screen height
 */
void DevTools::controlCameraPointer(sead::LookAtCamera* pCamera, PoiningControlBuffer* pBuffer,
                                    bool isPress, bool isLeft, bool isRight, bool isMiddle,
                                    const sead::Vector2f& rPos, bool isInside, f32 width,
                                    f32 height)
{
    bool isMove = isPress && isLeft;
    bool isRotate = isPress && isRight;
    bool isZoom = ((isLeft && isRight) || isMiddle) && isPress;
    bool isAny = isMove || isRotate || isZoom;

    bool isFirst;
    if (pBuffer->mIsActive == 0)
    {
        if (!(isAny && isInside))
        {
            return;
        }
        pBuffer->mIsActive = 1;
        isFirst = true;
    }
    else
    {
        if (!isAny)
        {
            pBuffer->mIsActive = 0;
            return;
        }
        isFirst = false;
    }

    sead::Vector2f pos = rPos;
    pos.x -= width * 0.5f;
    pos.y = -(pos.y - height * 0.5f);

    f32 rotate = 0.0f;
    f32 move = 0.0f;
    f32 zoom = 0.0f;
    if (isZoom)
    {
        zoom = 1.0f;
    }
    else if (isMove)
    {
        move = 1.0f;
    }
    else if (isRotate)
    {
        rotate = 1.0f;
    }

    sead::Vector2f delta = pos - pBuffer->mPrevPos;
    if (isFirst)
    {
        delta = sead::Vector2f::zero;
    }
    controlCameraPointer(pCamera, delta, rotate, move, zoom);
    pBuffer->mPrevPos = pos;
}

/**
 * Moves a look-at camera from a pointer movement delta.
 * @param pCamera camera to control
 * @param rDelta pointer movement
 * @param rotate rotation factor
 * @param move translation factor
 * @param zoom zoom factor
 */
void DevTools::controlCameraPointer(sead::LookAtCamera* pCamera, const sead::Vector2f& rDelta,
                                    f32 rotate, f32 move, f32 zoom)
{
    sead::Vector2f rotateStick(0.0f, 0.0f);
    sead::Vector2f moveStick(0.0f, 0.0f);
    if (rotate > 0.0f)
    {
        rotateStick.x = rDelta.x * -0.1f * rotate;
        rotateStick.y = rDelta.y * -0.1f * rotate;
    }
    if (move > 0.0f)
    {
        moveStick.set(rDelta.x * -0.08f * move, rDelta.y * -0.08f * move);
    }
    f32 zoomValue = 0.0f;
    if (zoom > 0.0f)
    {
        zoomValue = (rDelta.y - rDelta.x) * 0.5f * zoom;
    }
    controlCamera(pCamera, moveStick, rotateStick, zoomValue, 0.0f, 0.0f, 0.0f, false,
                  cCameraControlType_0);
}

/**
 * Draws a filled fan with the immediate drawing shader.
 * @param pDrawContext draw context
 * @param rViewMtx view matrix
 * @param rProjMtx projection matrix
 * @param rColor fan color
 * @param start start of the fan as a fraction of a full turn
 * @param end end of the fan as a fraction of a full turn
 * @param divNum number of divisions per full turn
 * @param offset angle offset in radians
 */
void DevTools::drawFan(DrawContext* pDrawContext, const sead::Matrix34f& rViewMtx,
                       const sead::Matrix44f& rProjMtx, const sead::Color4f& rColor, f32 start,
                       f32 end, u32 divNum, f32 offset)
{
    u32 num = 0;
    drawFan_(pDrawContext, &num, rViewMtx, rProjMtx, rColor, start, end, divNum, offset);

    IndexStream stream;
    stream.setPrimitiveType(NVN_DRAW_PRIMITIVE_TRIANGLE_FAN);
    drawArrays(pDrawContext, stream, num + 2);
}

/**
 * Draws a wireframe fan with the immediate drawing shader.
 * @param pDrawContext draw context
 * @param rViewMtx view matrix
 * @param rProjMtx projection matrix
 * @param rColor fan color
 * @param start start of the fan as a fraction of a full turn
 * @param end end of the fan as a fraction of a full turn
 * @param divNum number of divisions per full turn
 * @param offset angle offset in radians
 */
void DevTools::drawWireFan(DrawContext* pDrawContext, const sead::Matrix34f& rViewMtx,
                           const sead::Matrix44f& rProjMtx, const sead::Color4f& rColor,
                           f32 start, f32 end, u32 divNum, f32 offset)
{
    u32 num = 0;
    drawFan_(pDrawContext, &num, rViewMtx, rProjMtx, rColor, start, end, divNum, offset);

    IndexStream stream;
    stream.setPrimitiveType(NVN_DRAW_PRIMITIVE_LINE_STRIP);
    drawArrays(pDrawContext, stream, num + 2);
}

/**
 * Sets up the fan shader and returns the number of fan segments to draw.
 * @param pDrawContext draw context
 * @param pNum receives the number of segments
 * @param rViewMtx view matrix
 * @param rProjMtx projection matrix
 * @param rColor fan color
 * @param start start of the fan as a fraction of a full turn
 * @param end end of the fan as a fraction of a full turn
 * @param divNum number of divisions per full turn
 * @param offset angle offset in radians
 */
void DevTools::drawFan_(DrawContext* pDrawContext, u32* pNum, const sead::Matrix34f& rViewMtx,
                        const sead::Matrix44f& rProjMtx, const sead::Color4f& rColor, f32 start,
                        f32 end, u32 divNum, f32 offset)
{
    const ShaderProgram* pProgram = getProgram(detail::ShaderHolder::cDrawFan);

    sead::Vector4f param;
    sead::Matrix44f viewProj;
    viewProj.setMul(rProjMtx, rViewMtx);

    f32 div = divNum;
    u32 num = (end - start) * div + (div + -1.0f) / (div * div);
    if (num == 0)
    {
        num = 1;
    }
    *pNum = num;

    param.x = start * sead::Mathf::pi2() + offset;
    param.y = end * sead::Mathf::pi2() + offset;
    param.z = param.y - param.x;
    param.w = param.z / num;

    pProgram->activate(pDrawContext, true);
    setUniform(pDrawContext, pProgram->getUniformLocation(0), viewProj);
    setUniform(pDrawContext, pProgram->getUniformLocation(1), param);
    setUniform(pDrawContext, pProgram->getUniformLocation(2), rColor);
    VertexAttribute::disableAttributeAll(pDrawContext);
}

/**
 * Draws a wireframe sphere representing a point light.
 * @param pDrawContext draw context
 * @param rPos light position
 * @param radius light radius
 * @param rColor draw color
 * @param rViewMtx view matrix
 * @param rProjMtx projection matrix
 */
void DevTools::drawPointLight(DrawContext* pDrawContext, const sead::Vector3f& rPos, f32 radius,
                              const sead::Color4f& rColor, const sead::Matrix34f& rViewMtx,
                              const sead::Matrix44f& rProjMtx)
{
    const VertexAttributeHolder* pAttributeHolder = VertexAttributeHolder::instance();
    const PrimitiveShape* pShape = PrimitiveShape::instance();
    const ShaderProgram* pProgram = getProgram(detail::ShaderHolder::cDevUtil);

    f32 diameter = radius * 2;
    sead::Matrix34f modelMtx;
    modelMtx.makeT(rPos);
    modelMtx.scaleBases(diameter, diameter, diameter);

    pProgram->activate(pDrawContext, true);
    pAttributeHolder->getVertexAttribute(VertexAttributeHolder::cAttribute_SphereNormal)
        .activate(pDrawContext);
    setUniformToDevToolsShader_(pDrawContext, modelMtx, rViewMtx, rProjMtx, rColor, rColor,
                                sead::Vector3f::ey);
    drawIndexStream(pDrawContext, pShape->getSphereIndexStream(PrimitiveShape::cQuality_Low,
                                                               PrimitiveShape::cDrawType_Line));
}

/**
 * Sets the transform, color and lighting uniforms of the development utility shader.
 * @param pDrawContext draw context
 * @param rModelMtx model matrix
 * @param rViewMtx view matrix
 * @param rProjMtx projection matrix
 * @param rColor diffuse color
 * @param rAmbient ambient color
 * @param rLightDir light direction in world space
 */
void DevTools::setUniformToDevToolsShader_(DrawContext* pDrawContext,
                                           const sead::Matrix34f& rModelMtx,
                                           const sead::Matrix34f& rViewMtx,
                                           const sead::Matrix44f& rProjMtx,
                                           const sead::Color4f& rColor,
                                           const sead::Color4f& rAmbient,
                                           const sead::Vector3f& rLightDir)
{
    sead::Matrix34f modelViewMtx;
    modelViewMtx.setMul(rViewMtx, rModelMtx);
    sead::Matrix44f mvp;
    mvp.setMul(rProjMtx, modelViewMtx);

    sead::Vector3f lightDir;
    lightDir.setRotated(rViewMtx, rLightDir);
    lightDir.normalize();

    const ShaderProgram* pProgram = getProgram(detail::ShaderHolder::cDevUtil);
    setUniform(pDrawContext, pProgram->getUniformLocation(1), modelViewMtx);
    setUniform(pDrawContext, pProgram->getUniformLocation(0), mvp);
    setUniform(pDrawContext, pProgram->getUniformLocation(2), rColor);
    setUniform(pDrawContext, pProgram->getUniformLocation(3), rAmbient);
    setUniform(pDrawContext, pProgram->getUniformLocation(4), sead::Color4f::cBlack);
    setUniform(pDrawContext, pProgram->getUniformLocation(5), lightDir);
}

/**
 * Draws a grid on the XZ plane centered at the origin with the immediate drawing shader.
 * @param pDrawContext draw context
 * @param sizeX grid size along X
 * @param sizeZ grid size along Z
 * @param divX number of divisions along X
 * @param divZ number of divisions along Z
 * @param rColor line color
 * @param width line width
 */
void DevTools::drawGridImm(DrawContext* pDrawContext, f32 sizeX, f32 sizeZ, u32 divX, u32 divZ,
                           const sead::Color4f& rColor, f32 width)
{
    f32 stepX = sizeX / divX;
    f32 stepZ = sizeZ / divZ;
    sead::Vector3f start = sead::Vector3f::zero;
    sead::Vector3f end = sead::Vector3f::zero;

    for (u32 i = 0; i <= divX; i++)
    {
        f32 x = stepX * i - sizeX * 0.5f;
        end.x = x;
        start.x = x;
        end.z = sizeZ * 0.5f;
        start.z = -sizeZ * 0.5f;
        drawLineImm(pDrawContext, start, end, rColor, width);
    }

    for (u32 i = 0; i <= divZ; i++)
    {
        f32 z = stepZ * i - sizeZ * 0.5f;
        start.z = z;
        end.z = z;
        start.x = -(sizeX * 0.5f);
        end.x = sizeX * 0.5f;
        drawLineImm(pDrawContext, start, end, rColor, width);
    }
}

/**
 * Draws the three axes of a matrix as red, green and blue lines with the immediate drawing shader.
 * @param pDrawContext draw context
 * @param rMtx matrix whose axes and translation are drawn
 * @param length axis length
 * @param width line width
 * @param alpha line alpha
 */
void DevTools::drawAxisImm(DrawContext* pDrawContext, const sead::Matrix34f& rMtx, f32 length,
                           f32 width, f32 alpha)
{
    sead::Vector3f origin;
    rMtx.getTranslation(origin);
    sead::Color4f red = sead::Color4f::cRed;
    sead::Color4f green = sead::Color4f::cGreen;
    sead::Color4f blue = sead::Color4f::cBlue;
    blue.a = alpha;
    green.a = alpha;
    red.a = alpha;
    sead::Vector3f end;

    end = rMtx.getBase(0) * length + origin;
    drawLineImm(pDrawContext, origin, end, red, width);

    end = rMtx.getBase(1) * length + origin;
    drawLineImm(pDrawContext, origin, end, green, width);

    end = rMtx.getBase(2) * length + origin;
    drawLineImm(pDrawContext, origin, end, blue, width);
}

/**
 * Draws the cursor texture at a position in normalized device coordinates.
 * @param pDrawContext draw context
 * @param rScreenSize screen size in pixels
 * @param rPos cursor position in normalized device coordinates
 * @param scale cursor scale
 */
void DevTools::drawCursor(DrawContext* pDrawContext, const sead::Vector2f& rScreenSize,
                          const sead::Vector2f& rPos, f32 scale)
{
    const TextureSampler* pSampler =
        detail::PrivateResource::instance()->getCursorTextureSampler();
    const TextureData& rTexture = pSampler->getTextureData();
    f32 width = rTexture.getWidth(0);
    f32 height = rTexture.getHeight(0);

    f32 invHalfWidth = 1.0f / (rScreenSize.x * 0.5f);
    f32 invHalfHeight = 1.0f / (rScreenSize.y * 0.5f);
    f32 offsetX = (width * 0.5f - 5.0f) * scale;
    f32 offsetY = (height * 0.5f - 3.0f) * scale;

    sead::Matrix44f projMtx = sead::Matrix44f::ident;
    sead::Matrix34f modelMtx;
    modelMtx.makeST(sead::Vector3f(width * scale * invHalfWidth, invHalfHeight * (height * scale),
                                   1.0f),
                    sead::Vector3f(rPos.x + invHalfWidth * offsetX,
                                   rPos.y - invHalfHeight * offsetY, 0.0f));
    drawTexture(pDrawContext, *pSampler, modelMtx, projMtx, sead::Color4f::cWhite);
}

/**
 * Draws a wireframe circle of diameter one transformed by a matrix with the immediate drawing shader.
 * @param pDrawContext draw context
 * @param rMtx transform of the circle on the XY plane
 * @param rColor line color
 * @param width line width
 * @param divNum number of line segments
 */
void DevTools::drawWireCircleImm(DrawContext* pDrawContext, const sead::Matrix34f& rMtx,
                                 const sead::Color4f& rColor, f32 width, u32 divNum)
{
    sead::Vector3f prev;
    prev.setMul(rMtx, sead::Vector3f(0.5f, 0.0f, 0.0f));

    for (u32 i = 0; i < divNum; i++)
    {
        u32 idx = (i + 1) * (0xFFFFFFFF / divNum);
        sead::Vector3f cur;
        cur.setMul(rMtx, sead::Vector3f(sead::Mathf::cosIdx(idx) * 0.5f,
                                        sead::Mathf::sinIdx(idx) * 0.5f, 0.0f));
        drawLineImm(pDrawContext, prev, cur, rColor, width);
        prev = cur;
    }
}

/**
 * Draws the twelve edges of a bounding box with the immediate drawing shader.
 * @param pDrawContext draw context
 * @param rBox bounding box
 * @param rColor line color
 * @param width line width
 */
void DevTools::drawBoundBoxImm(DrawContext* pDrawContext, const sead::BoundBox3f& rBox,
                               const sead::Color4f& rColor, f32 width)
{
    sead::Vector3f min = rBox.getMin();
    sead::Vector3f max = rBox.getMax();
    sead::Vector3f start;
    sead::Vector3f end;

    start.set(min.x, min.y, min.z);
    end.set(max.x, min.y, min.z);
    drawLineImm(pDrawContext, start, end, rColor, width);
    start.set(min.x, max.y, min.z);
    end.set(max.x, max.y, min.z);
    drawLineImm(pDrawContext, start, end, rColor, width);
    start.set(min.x, min.y, max.z);
    end.set(max.x, min.y, max.z);
    drawLineImm(pDrawContext, start, end, rColor, width);
    start.set(min.x, max.y, max.z);
    end.set(max.x, max.y, max.z);
    drawLineImm(pDrawContext, start, end, rColor, width);
    start.set(min.x, min.y, min.z);
    end.set(min.x, max.y, min.z);
    drawLineImm(pDrawContext, start, end, rColor, width);
    start.set(min.x, min.y, max.z);
    end.set(min.x, max.y, max.z);
    drawLineImm(pDrawContext, start, end, rColor, width);
    start.set(min.x, min.y, min.z);
    end.set(min.x, min.y, max.z);
    drawLineImm(pDrawContext, start, end, rColor, width);
    start.set(min.x, max.y, min.z);
    end.set(min.x, max.y, max.z);
    drawLineImm(pDrawContext, start, end, rColor, width);
    start.set(max.x, min.y, min.z);
    end.set(max.x, max.y, min.z);
    drawLineImm(pDrawContext, start, end, rColor, width);
    start.set(max.x, min.y, max.z);
    end.set(max.x, max.y, max.z);
    drawLineImm(pDrawContext, start, end, rColor, width);
    start.set(max.x, min.y, min.z);
    end.set(max.x, min.y, max.z);
    drawLineImm(pDrawContext, start, end, rColor, width);
    start.set(max.x, max.y, min.z);
    end.set(max.x, max.y, max.z);
    drawLineImm(pDrawContext, start, end, rColor, width);
}

/**
 * Visualizes a depth texture over a quad placed by model and projection matrices.
 * @param pDrawContext draw context
 * @param rTexture depth texture
 * @param index texture array index
 * @param rModelMtx model matrix of the quad
 * @param rProjMtx projection matrix of the quad
 * @param rViewMtx view matrix used to render the depth
 * @param rDepthProjMtx projection matrix used to render the depth
 */
void DevTools::drawVisualizedDepth(DrawContext* pDrawContext, const TextureData& rTexture,
                                   s32 index, const sead::Matrix34f& rModelMtx,
                                   const sead::Matrix44f& rProjMtx,
                                   const sead::Matrix34f& rViewMtx,
                                   const sead::Matrix44f& rDepthProjMtx)
{
    const sead::Matrix44f bias(0.5f, 0.0f, 0.0f, 0.5f, 0.0f, -0.5f, 0.0f, 0.5f, 0.0f, 0.0f, 0.5f,
                               0.5f, 0.0f, 0.0f, 0.0f, 1.0f);
    sead::Matrix44f mtx;
    mulMtx44(&mtx, bias, rProjMtx);
    mtx.setMul(mtx, rModelMtx);
    drawVisualizedDepth(pDrawContext, rTexture, index, mtx, rViewMtx, rDepthProjMtx);
}

/**
 * Visualizes a depth texture as instanced cubes transformed into the depth render space.
 * @param pDrawContext draw context
 * @param rTexture depth texture
 * @param index texture array index
 * @param rMtx transform from the visualized volume to clip space
 * @param rViewMtx view matrix used to render the depth
 * @param rDepthProjMtx projection matrix used to render the depth
 */
void DevTools::drawVisualizedDepth(DrawContext* pDrawContext, const TextureData& rTexture,
                                   s32 index, const sead::Matrix44f& rMtx,
                                   const sead::Matrix34f& rViewMtx,
                                   const sead::Matrix44f& rDepthProjMtx)
{
    const ShaderProgram* pProgram =
        getProgramUnsafe(detail::ShaderHolder::cDepthShadowDebug)
            ->getVariation(rTexture.getTextureType() == NVN_TEXTURE_TARGET_2D_ARRAY ? 1 : 0);
    pProgram->activate(pDrawContext, true);

    sead::Matrix34f scaleMtx;
    scaleMtx.makeS(sead::Vector3f::ones * 2.0f);
    sead::Matrix44f invMtx;
    invMtx.setInverse(rMtx);
    sead::Matrix44f depthMtx;
    depthMtx.setMul(rDepthProjMtx, rViewMtx);
    sead::Matrix44f mtx;
    mulMtx44(&mtx, depthMtx, invMtx);
    mtx.setMul(mtx, scaleMtx);
    setUniform(pDrawContext, pProgram->getUniformLocation(0), mtx);

    VertexAttributeHolder::instance()
        ->getVertexAttribute(VertexAttributeHolder::cAttribute_CubeNormal)
        .activate(pDrawContext);

    TextureSampler sampler(rTexture);
    sampler.setFilterDirect(0, 0, 1);
    s32 width = sampler.getTextureData().getMipWidth(0);
    s32 height = sampler.getTextureData().getMipHeight(0);
    {
        sead::Vector4f texelSize(1.0f / width, 1.0f / height, 1.0f / (width - 1),
                                 1.0f / (height - 1));
        setUniform(pDrawContext, pProgram->getUniformLocation(2), texelSize);
    }
    sampler.activate(pDrawContext, pProgram->getSamplerLocation(0), -1, false);
    setUniform(pDrawContext, pProgram->getUniformLocation(3), index);

    {
        sead::GraphicsContext graphicsContext;
        graphicsContext.setCullingMode(2);
        graphicsContext.setAlphaTestEnable(false);
        graphicsContext.setBlendEnable(false);
        graphicsContext.apply(pDrawContext);
    }

    for (s32 z = 0; z < height; z++)
    {
        setUniform(pDrawContext, pProgram->getUniformLocation(4), z);
        drawIndexStreamInstanced(pDrawContext, PrimitiveShape::instance()->getCubeIndexStream(),
                                 width);
    }
}

/**
 * Colors the depth buffer of a render buffer by depth using up to 32 sorted gradation stops.
 * @param pDrawContext draw context
 * @param rRenderBuffer render buffer whose depth target is visualized
 * @param num number of gradation stops
 * @param pDepth depth of each stop
 * @param pColor color of each stop
 * @param near near clip distance
 * @param far far clip distance
 */
void DevTools::drawDepthGradation(DrawContext* pDrawContext, const RenderBuffer& rRenderBuffer,
                                  u32 num, const f32* pDepth, const sead::Color4f* pColor,
                                  f32 near, f32 far)
{
    const RenderTargetDepth* pDepthTarget = rRenderBuffer.getRenderTargetDepth();
    if (pDepthTarget == nullptr)
    {
        return;
    }

    pDepthTarget->expandHiZBuffer(pDrawContext);

    TextureSampler sampler;
    sampler.applyTextureData(*rRenderBuffer.getRenderTargetDepth());

    sead::GraphicsContext graphicsContext;
    graphicsContext.setDepthEnable(false, false);
    graphicsContext.setBlendEnable(true);
    graphicsContext.apply(pDrawContext);

    sead::Vector4f depths[32];
    sead::Color4f colors[32];
    for (u32 i = 0; i < num; i++)
    {
        s32 rank = 0;
        for (u32 j = 0; j < num; j++)
        {
            if (j != i && pDepth[j] < pDepth[i])
            {
                rank++;
            }
        }
        depths[rank].x = pDepth[i];
        colors[rank] = pColor[i];
    }

    const ShaderProgram* pProgram = getProgram(detail::ShaderHolder::cDepthVisualize);
    pProgram->activate(pDrawContext, true);
    setUniform(pDrawContext, pProgram->getUniformLocation(0), near);
    setUniform(pDrawContext, pProgram->getUniformLocation(1), far);
    setUniform(pDrawContext, pProgram->getUniformLocation(4), num);
    if (pProgram->getUniformLocation(2).isValid())
    {
        pProgram->getUniformLocation(2).setUniformNVN(pDrawContext, num * 4, depths);
    }
    if (pProgram->getUniformLocation(3).isValid())
    {
        pProgram->getUniformLocation(3).setUniformNVN(pDrawContext, num * 4, colors);
    }
    sampler.activate(pDrawContext, pProgram->getSamplerLocation(0), -1, false);

    VertexAttributeHolder::instance()
        ->getVertexAttribute(VertexAttributeHolder::cAttribute_QuadTriangleTexCoord)
        .activate(pDrawContext);
    drawIndexStream(pDrawContext, PrimitiveShape::instance()->getQuadTriangleIndexStream());
}

/**
 * Draws the wireframe of a frustum given by a camera matrix and a projection matrix.
 * @param pDrawContext draw context
 * @param rCameraMtx camera world matrix
 * @param rFrustumProjMtx projection matrix of the frustum
 * @param rViewMtx view matrix used for drawing
 * @param rProjMtx projection matrix used for drawing
 * @param isFill unused
 * @param rColor line color
 */
void DevTools::drawFrustum_(DrawContext* pDrawContext, const sead::Matrix34f& rCameraMtx,
                            const sead::Matrix44f& rFrustumProjMtx,
                            const sead::Matrix34f& rViewMtx, const sead::Matrix44f& rProjMtx,
                            bool isFill, const sead::Color4f& rColor)
{
    sead::Matrix44f invProjMtx;
    invProjMtx.setInverse(rFrustumProjMtx);

    sead::Vector3f pos[8] = {{-1.0f, -1.0f, -1.0f}, {1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, -1.0f},
                             {-1.0f, 1.0f, -1.0f},  {-1.0f, -1.0f, 1.0f}, {1.0f, -1.0f, 1.0f},
                             {1.0f, 1.0f, 1.0f},    {-1.0f, 1.0f, 1.0f}};

    for (s32 i = 0; i < 4; i++)
    {
        pos[i].setMul(invProjMtx, pos[i]);
        pos[i].setMul(rCameraMtx, pos[i]);
        pos[i + 4].setMul(invProjMtx, pos[i + 4]);
        pos[i + 4].setMul(rCameraMtx, pos[i + 4]);
    }

    beginDrawImm(pDrawContext, rViewMtx, rProjMtx);
    drawLineImm(pDrawContext, pos[0], pos[1], rColor, 1.0f);
    drawLineImm(pDrawContext, pos[1], pos[2], rColor, 1.0f);
    drawLineImm(pDrawContext, pos[2], pos[3], rColor, 1.0f);
    drawLineImm(pDrawContext, pos[3], pos[0], rColor, 1.0f);
    drawLineImm(pDrawContext, pos[4], pos[5], rColor, 1.0f);
    drawLineImm(pDrawContext, pos[5], pos[6], rColor, 1.0f);
    drawLineImm(pDrawContext, pos[6], pos[7], rColor, 1.0f);
    drawLineImm(pDrawContext, pos[7], pos[4], rColor, 1.0f);
    drawLineImm(pDrawContext, pos[0], pos[4], rColor, 1.0f);
    drawLineImm(pDrawContext, pos[1], pos[5], rColor, 1.0f);
    drawLineImm(pDrawContext, pos[2], pos[6], rColor, 1.0f);
    drawLineImm(pDrawContext, pos[3], pos[7], rColor, 1.0f);
}

/**
 * Draws a shaded arrow made of a cylinder body and a cone head.
 * @param pDrawContext draw context
 * @param rStart start of the arrow
 * @param rEnd tip of the arrow
 * @param rColor0 first draw color
 * @param rColor1 second draw color
 * @param width thickness of the arrow body, the head being twice as large
 * @param rViewMtx view matrix
 * @param rProjMtx projection matrix
 */
void DevTools::drawArrow(DrawContext* pDrawContext, const sead::Vector3f& rStart,
                         const sead::Vector3f& rEnd, const sead::Color4f& rColor0,
                         const sead::Color4f& rColor1, f32 width, const sead::Matrix34f& rViewMtx,
                         const sead::Matrix44f& rProjMtx)
{
    const VertexAttributeHolder* pAttributeHolder = VertexAttributeHolder::instance();
    const PrimitiveShape* pShape = PrimitiveShape::instance();
    const ShaderProgram* pProgram = getProgram(detail::ShaderHolder::cDevUtil);

    sead::Vector3f dir = rEnd - rStart;
    const f32 bodyLength = std::fmax(dir.length() - width, 0.0f);
    dir.normalize();

    sead::Quatf rotation;
    sead::Matrix34f rotationMtx;
    if (rotation.makeVectorRotation(sead::Vector3f::ey, dir))
    {
        rotationMtx.fromQuat(rotation);
    }
    else
    {
        rotation.setAxisRadian(sead::Vector3f::ey, sead::Mathf::pi());
        rotationMtx.fromQuat(rotation);
    }

    const sead::Vector3f headPos = rStart + dir * bodyLength;
    pProgram->activate(pDrawContext, true);

    {
        sead::Matrix34f headMtx = rotationMtx;
        const f32 headSize = width * 2;
        headMtx.scaleBases(headSize, headSize, headSize);
        headMtx.setTranslation((headPos + rEnd) * 0.5f);
        setUniformToDevToolsShader_(pDrawContext, headMtx, rViewMtx, rProjMtx, rColor0, rColor1,
                                    -sead::Vector3f::ones);
        pAttributeHolder->getVertexAttribute(VertexAttributeHolder::cAttribute_ConeNormal)
            .activate(pDrawContext);
        drawIndexStream(pDrawContext,
                        pShape->getConeTriangleIndexStream(PrimitiveShape::cQuality_Middle));
    }

    sead::Matrix34f bodyMtx = rotationMtx;
    bodyMtx.scaleBases(width, bodyLength, width);
    bodyMtx.setTranslation((rStart + headPos) * 0.5f);
    setUniformToDevToolsShader_(pDrawContext, bodyMtx, rViewMtx, rProjMtx, rColor0, rColor1,
                                -sead::Vector3f::ones);
    pAttributeHolder->getVertexAttribute(VertexAttributeHolder::cAttribute_CylinderNormal)
        .activate(pDrawContext);
    drawIndexStream(pDrawContext,
                    pShape->getCylinderTriangleIndexStream(PrimitiveShape::cQuality_Middle));
}

}  // namespace agl::utl
