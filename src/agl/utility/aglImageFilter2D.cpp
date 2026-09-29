#include "utility/aglImageFilter2D.h"

#include <gfx/seadColor.h>
#include <gfx/seadProjection.h>
#include <gfx/seadViewport.h>
#include <math/seadMatrix.h>

#include "common/aglDrawContext.h"
#include "common/aglShaderProgram.h"
#include "common/aglTextureSampler.h"
#include "detail/aglShaderHolder.h"
#include "postfx/aglPostFxUtil.h"
#include "utility/aglPrimitiveShape.h"
#include "utility/aglVertexAttributeHolder.h"

namespace agl::utl {

namespace {

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

inline void setUniform(DrawContext* pDrawContext, const UniformLocation& rLocation, u32 value)
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

inline u32 getWidth(const TextureData& rTexture)
{
    u32 width = rTexture.getWidth();
    return width > 1 ? width : 1;
}

inline u32 getHeight(const TextureData& rTexture)
{
    s32 min = rTexture.getMinHeight_();
    s32 height = rTexture.getHeight();
    return height < min ? min : height;
}

inline u32 getMipWidth(const TextureData& rTexture, s32 mipLevel)
{
    s32 width = rTexture.getWidth() >> mipLevel;
    return width > 1 ? width : 1;
}

inline u32 getMipHeight(const TextureData& rTexture, s32 mipLevel)
{
    s32 min = rTexture.getMinHeight_();
    s32 height = rTexture.getHeight() >> mipLevel;
    return min > height ? min : height;
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

inline void drawQuad(DrawContext* pDrawContext)
{
    VertexAttributeHolder::instance()
        ->getVertexAttribute(VertexAttributeHolder::cAttribute_Quad)
        .activate(pDrawContext);
    drawIndexStream(pDrawContext, PrimitiveShape::instance()->getQuadIndexStream());
}

const f32 cFxaaLumaWeight[4] = {0.299f, 0.587f, 0.114f, 0.0f};
const f32 cFxaaParam[4] = {0.2f, 0.1f, 1.0f, 0.0f};
const sead::Vector2f cBlurDirection[] = {sead::Vector2f(1.0f, 0.0f), sead::Vector2f(0.0f, 1.0f)};

inline void drawQuadTriangleQuadIndex(DrawContext* pDrawContext)
{
    VertexAttributeHolder::instance()
        ->getVertexAttribute(VertexAttributeHolder::cAttribute_QuadTriangleTexCoord)
        .activate(pDrawContext);
    drawIndexStream(pDrawContext, PrimitiveShape::instance()->getQuadIndexStream());
}

inline ShaderProgram* getProgram(s32 type)
{
    return detail::ShaderHolder::instance()->getShaderProgram(type);
}

inline ShaderProgram* getProgramUnsafe(s32 type)
{
    return detail::ShaderHolder::instance()->getShaderProgramUnsafe(type);
}

inline void drawQuadTriangle_(DrawContext* pDrawContext)
{
    VertexAttributeHolder::instance()
        ->getVertexAttribute(VertexAttributeHolder::cAttribute_QuadTriangleTexCoord)
        .activate(pDrawContext);
    drawIndexStream(pDrawContext,
                                 PrimitiveShape::instance()->getQuadTriangleIndexStream());
}

[[gnu::always_inline]] inline void drawBC_(DrawContext* pDrawContext, const TextureSampler& rSampler, u32 mipLevel,
                    u32 slice, ImageFilter2D::TextureCompressType type, s32 programType)
{
    const TextureData& rTexture = rSampler.getTextureData();
    f32 width = getMipWidth(rTexture, mipLevel);
    f32 height = getMipHeight(rTexture, mipLevel);

    s32 textureType = -1;
    switch (rTexture.getSurface().mTarget)
    {
    case NVN_TEXTURE_TARGET_2D:
        textureType = 0;
        break;
    case NVN_TEXTURE_TARGET_2D_ARRAY:
        textureType = 1;
        break;
    case NVN_TEXTURE_TARGET_3D:
        textureType = 2;
        break;
    case NVN_TEXTURE_TARGET_CUBEMAP:
        textureType = getSlice(rTexture) > 6 ? 4 : 3;
        break;
    default:
        break;
    }

    sead::Vector4f invSize(1.0f / width, 1.0f / height, 0.0f, 0.0f);
    f32 layer;
    u32 face;
    if (textureType == 3 || textureType == 4)
    {
        layer = slice / 6;
        face = slice % 6;
    }
    else
    {
        layer = slice;
        if (textureType == 2)
        {
            layer /= f32(getSlice(rTexture));
        }
        face = 0;
    }

    const ShaderProgram* pProgram =
        getProgramUnsafe(programType)->getVariation(textureType + type * 5);
    pProgram->activate(pDrawContext, true);
    setUniform(pDrawContext, pProgram->getUniformLocation(0), invSize);
    setUniform(pDrawContext, pProgram->getUniformLocation(1), f32(mipLevel));
    setUniform(pDrawContext, pProgram->getUniformLocation(2), layer);
    setUniform(pDrawContext, pProgram->getUniformLocation(3), face);
    rSampler.activate(pDrawContext, pProgram->getSamplerLocation(0), -1, false);
    drawQuadTriangle_(pDrawContext);
}

}  // namespace

/**
 * Draws a texture with the plain texture shader.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 * @param rViewport viewport to draw into
 * @param rScale scale of the drawn texture
 * @param rTranslate translation of the drawn texture
 */
void ImageFilter2D::drawTexture(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                const sead::Viewport& rViewport, const sead::Vector2f& rScale,
                                const sead::Vector2f& rTranslate)
{
    const ShaderProgram* pProgram =
        getProgramUnsafe(detail::ShaderHolder::cTexture)->getVariation(0);
    pProgram->activate(pDrawContext, true);
    draw(pDrawContext, *pProgram, rSampler, rViewport, rScale, rTranslate);
}

/**
 * Draws a texture as a quad with an activated shader program.
 * @param pDrawContext draw context
 * @param rProgram activated shader program
 * @param rSampler texture to draw
 * @param rViewport viewport to draw into
 * @param rScale scale of the drawn texture
 * @param rTranslate translation of the drawn texture
 */
void ImageFilter2D::draw(DrawContext* pDrawContext, const ShaderProgram& rProgram,
                         const TextureSampler& rSampler, const sead::Viewport& rViewport,
                         const sead::Vector2f& rScale, const sead::Vector2f& rTranslate)
{
    const TextureData& rTexture = rSampler.getTextureData();
    {
        sead::Vector4f texSize(getWidth(rTexture), getHeight(rTexture),
                               1.0f / getWidth(rTexture), 1.0f / getHeight(rTexture));
        setUniform(pDrawContext, rProgram.getUniformLocation(1), texSize);
    }

    sead::OrthoProjection projection(0.0f, 1.0f, rViewport);

    const sead::Vector2f& rMin = rViewport.getMin();
    const sead::Vector2f& rMax = rViewport.getMax();
    sead::Vector2f center = (rMin + rMax) * 0.5f;
    sead::Vector2f pos = rTranslate - (center - rMin);
    f32 x = pos.x;
    f32 y = pos.y;

    f32 width = rScale.x * getWidth(rTexture);
    f32 height = rScale.y * getHeight(rTexture);
    sead::Matrix34f mtx(width, 0.0f, 0.0f, x + rScale.x * getWidth(rTexture) * 0.5f, 0.0f, height,
                        0.0f, -(y + rScale.y * getHeight(rTexture) * 0.5f), 0.0f, 0.0f, 0.0f,
                        0.0f);

    sead::Matrix44f mvp;
    mvp.setMul(projection.getProjectionMatrix(), mtx);

    rProgram.validate_();
    rSampler.activate(pDrawContext, rProgram.getSamplerLocation(0), -1, false);
    setUniform(pDrawContext, rProgram.getUniformLocation(0), mvp);
    drawQuad(pDrawContext);
}

/**
 * Draws a texture over the whole render target with the plain texture shader.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 */
void ImageFilter2D::drawTextureQuadTriangle(DrawContext* pDrawContext,
                                            const TextureSampler& rSampler)
{
    const ShaderProgram* pProgram =
        getProgramUnsafe(detail::ShaderHolder::cTexture)->getVariation(1);
    pProgram->activate(pDrawContext, true);
    drawQuadTriangle(pDrawContext, *pProgram, rSampler);
}

/**
 * Draws a texture over the whole render target with an activated shader program.
 * @param pDrawContext draw context
 * @param rProgram activated shader program
 * @param rSampler texture to draw
 */
void ImageFilter2D::drawQuadTriangle(DrawContext* pDrawContext, const ShaderProgram& rProgram,
                                     const TextureSampler& rSampler)
{
    const TextureData& rTexture = rSampler.getTextureData();
    sead::Vector4f texSize(getWidth(rTexture), getHeight(rTexture),
                           1.0f / getWidth(rTexture), 1.0f / getHeight(rTexture));
    setUniform(pDrawContext, rProgram.getUniformLocation(1), texSize);

    rProgram.validate_();
    rSampler.activate(pDrawContext, rProgram.getSamplerLocation(0), -1, false);
    drawQuadTriangle_(pDrawContext);
}

/**
 * Draws a depth texture over the whole render target.
 * @param pDrawContext draw context
 * @param rSampler depth texture to draw
 */
void ImageFilter2D::drawDepthQuadTriangle(DrawContext* pDrawContext,
                                          const TextureSampler& rSampler)
{
    const ShaderProgram* pProgram = getProgram(detail::ShaderHolder::cDepth2d);
    pProgram->activate(pDrawContext, true);
    drawQuadTriangle(pDrawContext, *pProgram, rSampler);
}

/**
 * Draws a single channel of a texture.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 * @param rViewport viewport to draw into
 * @param channel channel to draw
 * @param rScale scale of the drawn texture
 * @param rTranslate translation of the drawn texture
 */
void ImageFilter2D::drawTextureChannel(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                       const sead::Viewport& rViewport, Channel channel,
                                       const sead::Vector2f& rScale,
                                       const sead::Vector2f& rTranslate)
{
    s32 type;
    switch (channel)
    {
    case cChannel_Red:
        type = detail::ShaderHolder::cRed;
        break;
    case cChannel_Green:
        type = detail::ShaderHolder::cGreen;
        break;
    case cChannel_Blue:
        type = detail::ShaderHolder::cBlue;
        break;
    case cChannel_Alpha:
        type = detail::ShaderHolder::cAlpha;
        break;
    case cChannel_Depth:
        type = detail::ShaderHolder::cDepthRaw;
        break;
    default:
        type = detail::ShaderHolder::cTexture;
        break;
    }

    const ShaderProgram* pProgram = getProgram(type);
    pProgram->activate(pDrawContext, true);
    draw(pDrawContext, *pProgram, rSampler, rViewport, rScale, rTranslate);
}

/**
 * Draws one mip level of a texture.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 * @param rViewport viewport to draw into
 * @param mipLevel mip level to draw
 * @param rScale scale of the drawn texture
 * @param rTranslate translation of the drawn texture
 */
void ImageFilter2D::drawTextureMipLevel(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                        const sead::Viewport& rViewport, f32 mipLevel,
                                        const sead::Vector2f& rScale,
                                        const sead::Vector2f& rTranslate)
{
    const ShaderProgram* pProgram =
        getProgramUnsafe(detail::ShaderHolder::cTexture)->getVariation(2);
    pProgram->activate(pDrawContext, true);
    setUniform(pDrawContext, pProgram->getUniformLocation(2), mipLevel);
    draw(pDrawContext, *pProgram, rSampler, rViewport, rScale, rTranslate);
}

/**
 * Draws one slice of a 2D array texture.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 * @param rViewport viewport to draw into
 * @param slice slice to draw
 * @param rScale scale of the drawn texture
 * @param rTranslate translation of the drawn texture
 * @param mipLevel mip level to draw, or -1 for the default level
 */
void ImageFilter2D::drawTexture2DArray(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                       const sead::Viewport& rViewport, s32 slice,
                                       const sead::Vector2f& rScale,
                                       const sead::Vector2f& rTranslate, f32 mipLevel)
{
    const ShaderProgram* pProgram = getProgramUnsafe(detail::ShaderHolder::cTexture2dArray)
                                        ->getVariation(mipLevel != -1.0f ? 1 : 0);
    pProgram->activate(pDrawContext, true);
    setUniform(pDrawContext, pProgram->getUniformLocation(3), f32(slice));
    setUniform(pDrawContext, pProgram->getUniformLocation(2), mipLevel);
    draw(pDrawContext, *pProgram, rSampler, rViewport, rScale, rTranslate);
}

/**
 * Draws one depth slice of a 3D texture.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 * @param rViewport viewport to draw into
 * @param depth depth coordinate to draw
 * @param rScale scale of the drawn texture
 * @param rTranslate translation of the drawn texture
 * @param mipLevel mip level to draw, or -1 for the default level
 */
void ImageFilter2D::drawTexture3D(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                  const sead::Viewport& rViewport, f32 depth,
                                  const sead::Vector2f& rScale, const sead::Vector2f& rTranslate,
                                  f32 mipLevel)
{
    const ShaderProgram* pProgram = getProgramUnsafe(detail::ShaderHolder::cTexture3d)
                                        ->getVariation(mipLevel != -1.0f ? 1 : 0);
    pProgram->activate(pDrawContext, true);
    setUniform(pDrawContext, pProgram->getUniformLocation(3), depth);
    setUniform(pDrawContext, pProgram->getUniformLocation(2), mipLevel);
    draw(pDrawContext, *pProgram, rSampler, rViewport, rScale, rTranslate);
}

/**
 * Draws one face of a cube map texture.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 * @param rViewport viewport to draw into
 * @param face face to draw
 * @param rScale scale of the drawn texture
 * @param rTranslate translation of the drawn texture
 * @param mipLevel mip level to draw, or -1 for the default level
 */
void ImageFilter2D::drawTextureCubeMap(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                       const sead::Viewport& rViewport, CubeMapFace face,
                                       const sead::Vector2f& rScale,
                                       const sead::Vector2f& rTranslate, f32 mipLevel)
{
    drawTextureCubeArray(pDrawContext, rSampler, rViewport, 0, face, rScale, rTranslate,
                         mipLevel);
}

/**
 * Draws one face of one cube of a cube map array texture.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 * @param rViewport viewport to draw into
 * @param slice cube to draw
 * @param face face to draw
 * @param rScale scale of the drawn texture
 * @param rTranslate translation of the drawn texture
 * @param mipLevel mip level to draw, or -1 for the default level
 */
void ImageFilter2D::drawTextureCubeArray(DrawContext* pDrawContext,
                                         const TextureSampler& rSampler,
                                         const sead::Viewport& rViewport, s32 slice,
                                         CubeMapFace face, const sead::Vector2f& rScale,
                                         const sead::Vector2f& rTranslate, f32 mipLevel)
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
    draw(pDrawContext, *pProgram, rSampler, rViewport, rScale, rTranslate);
}

/**
 * Draws a texture with transformed texture coordinates.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 * @param rViewport viewport to draw into
 * @param rTexCoordScale texture coordinate scale
 * @param rotate texture coordinate rotation
 * @param rTexCoordTranslate texture coordinate translation
 * @param rScale scale of the drawn texture
 * @param rTranslate translation of the drawn texture
 */
void ImageFilter2D::drawTextureTexCoord(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                        const sead::Viewport& rViewport,
                                        const sead::Vector2f& rTexCoordScale, f32 rotate,
                                        const sead::Vector2f& rTexCoordTranslate,
                                        const sead::Vector2f& rScale,
                                        const sead::Vector2f& rTranslate)
{
    const ShaderProgram* pProgram = getProgram(detail::ShaderHolder::cTextureTexcoord);
    pProgram->activate(pDrawContext, true);
    setUniform(pDrawContext, pProgram->getUniformLocation(4), rTexCoordScale);
    setUniform(pDrawContext, pProgram->getUniformLocation(5), rotate);
    setUniform(pDrawContext, pProgram->getUniformLocation(6), rTexCoordTranslate);
    draw(pDrawContext, *pProgram, rSampler, rViewport, rScale, rTranslate);
}

/**
 * Draws a multisampled texture with the shader matching its sample count.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 * @param rViewport viewport to draw into
 * @param rScale scale of the drawn texture
 * @param rTranslate translation of the drawn texture
 */
void ImageFilter2D::drawTextureMSAA(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                    const sead::Viewport& rViewport, const sead::Vector2f& rScale,
                                    const sead::Vector2f& rTranslate)
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
    draw(pDrawContext, *pProgram, rSampler, rViewport, rScale, rTranslate);
}

/**
 * Draws a texture reduced by a power of two.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 * @param rViewport viewport to draw into
 * @param scale reduction scale
 * @param mipLevel mip level to sample
 * @param rTranslate translation of the drawn texture
 */
void ImageFilter2D::drawReduce(DrawContext* pDrawContext, const TextureSampler& rSampler,
                               const sead::Viewport& rViewport, ReduceScale scale, f32 mipLevel,
                               const sead::Vector2f& rTranslate)
{
    const ShaderProgram* pProgram = getReduceProgram_(scale);
    pProgram->activate(pDrawContext, true);
    setUniform(pDrawContext, pProgram->getUniformLocation(11), mipLevel);
    f32 reduce = 1.0f / (1 << scale);
    sead::Vector2f drawScale(reduce * sead::Vector2f::ones.x, reduce * sead::Vector2f::ones.y);
    draw(pDrawContext, *pProgram, rSampler, rViewport, drawScale, rTranslate);
}

/**
 * Returns the reduction shader for a reduction scale.
 * @param scale reduction scale
 * @return the matching shader program
 */
ShaderProgram* ImageFilter2D::getReduceProgram_(ReduceScale scale)
{
    s32 type;
    switch (scale)
    {
    case cReduceScale_2:
        type = detail::ShaderHolder::cReduce2;
        break;
    case cReduceScale_4:
        type = detail::ShaderHolder::cReduce4;
        break;
    case cReduceScale_8:
        type = detail::ShaderHolder::cReduce8;
        break;
    case cReduceScale_16:
        type = detail::ShaderHolder::cReduce16;
        break;
    default:
        type = detail::ShaderHolder::cTexture;
        break;
    }
    return getProgram(type);
}

/**
 * Draws a texture with a blur filter.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 * @param rViewport viewport to draw into
 * @param type blur type
 * @param rStep blur sampling step
 * @param rScale scale of the drawn texture
 * @param rTranslate translation of the drawn texture
 */
void ImageFilter2D::drawBlur(DrawContext* pDrawContext, const TextureSampler& rSampler,
                             const sead::Viewport& rViewport, BlurType type,
                             const sead::Vector2f& rStep, const sead::Vector2f& rScale,
                             const sead::Vector2f& rTranslate)
{
    const ShaderProgram* pProgram = getBlurProgram_(type);
    pProgram->activate(pDrawContext, true);
    setUniform(pDrawContext, pProgram->getUniformLocation(18), rStep);
    draw(pDrawContext, *pProgram, rSampler, rViewport, rScale, rTranslate);
}

/**
 * Returns the blur shader variation for a blur type.
 * @param type blur type
 * @return the matching shader program
 */
const ShaderProgram* ImageFilter2D::getBlurProgram_(BlurType type)
{
    return getProgramUnsafe(detail::ShaderHolder::cImageFilterBlur)->getVariation(type);
}

/**
 * Draws a texture with a gaussian filter.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 * @param rViewport viewport to draw into
 * @param kernel gaussian kernel size
 * @param isVertical whether to filter vertically
 * @param isHalf whether to use the half-size variation
 * @param rTranslate translation of the drawn texture
 */
void ImageFilter2D::drawGaussian(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                 const sead::Viewport& rViewport, GaussianKernel kernel,
                                 bool isVertical, bool isHalf, const sead::Vector2f& rTranslate)
{
    const ShaderProgram* pProgram =
        getProgramUnsafe(detail::ShaderHolder::cImageFilterGaussian)
            ->getVariation(kernel << 2 | isVertical << 1 | isHalf);
    pProgram->activate(pDrawContext, true);
    draw(pDrawContext, *pProgram, rSampler, rViewport, sead::Vector2f::ones, rTranslate);
}

/**
 * Draws a texture with cubic filtering and transformed texture coordinates.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 * @param rViewport viewport to draw into
 * @param rTexCoordScale texture coordinate scale
 * @param rotate texture coordinate rotation
 * @param rTexCoordTranslate texture coordinate translation
 * @param rScale scale of the drawn texture
 * @param rTranslate translation of the drawn texture
 */
void ImageFilter2D::drawCubic(DrawContext* pDrawContext, const TextureSampler& rSampler,
                              const sead::Viewport& rViewport,
                              const sead::Vector2f& rTexCoordScale, f32 rotate,
                              const sead::Vector2f& rTexCoordTranslate,
                              const sead::Vector2f& rScale, const sead::Vector2f& rTranslate)
{
    const ShaderProgram* pProgram = getProgram(detail::ShaderHolder::cImageFilterCubic);
    pProgram->activate(pDrawContext, true);
    setUniform(pDrawContext, pProgram->getUniformLocation(4), rTexCoordScale);
    setUniform(pDrawContext, pProgram->getUniformLocation(5), rotate);
    setUniform(pDrawContext, pProgram->getUniformLocation(6), rTexCoordTranslate);
    draw(pDrawContext, *pProgram, rSampler, rViewport, rScale, rTranslate);
}

/**
 * Draws a texture with its alpha channel modified.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 * @param rViewport viewport to draw into
 * @param rScale scale of the drawn texture
 * @param rTranslate translation of the drawn texture
 */
void ImageFilter2D::drawAlphaModifiedTexture(DrawContext* pDrawContext,
                                             const TextureSampler& rSampler,
                                             const sead::Viewport& rViewport,
                                             const sead::Vector2f& rScale,
                                             const sead::Vector2f& rTranslate)
{
    const ShaderProgram* pProgram = getProgram(detail::ShaderHolder::cAlphaModify);
    pProgram->activate(pDrawContext, true);
    draw(pDrawContext, *pProgram, rSampler, rViewport, rScale, rTranslate);
}

/**
 * Draws a texture with each color channel shifted separately.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 * @param rViewport viewport to draw into
 * @param rDriftR red channel offset
 * @param rDriftG green channel offset
 * @param rDriftB blue channel offset
 * @param rScale scale of the drawn texture
 * @param rTranslate translation of the drawn texture
 */
void ImageFilter2D::drawColorDrift(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                   const sead::Viewport& rViewport, const sead::Vector2f& rDriftR,
                                   const sead::Vector2f& rDriftG, const sead::Vector2f& rDriftB,
                                   const sead::Vector2f& rScale, const sead::Vector2f& rTranslate)
{
    const ShaderProgram* pProgram = getProgram(detail::ShaderHolder::cTextureColorDrift);
    pProgram->activate(pDrawContext, true);
    setUniform(pDrawContext, pProgram->getUniformLocation(12), rDriftR);
    setUniform(pDrawContext, pProgram->getUniformLocation(13), rDriftG);
    setUniform(pDrawContext, pProgram->getUniformLocation(14), rDriftB);
    draw(pDrawContext, *pProgram, rSampler, rViewport, rScale, rTranslate);
}

/**
 * Draws a YUV NV12 texture pair converted to RGB.
 * @param pDrawContext draw context
 * @param rSamplerY luma texture
 * @param rSamplerUV chroma texture
 * @param rViewport viewport to draw into
 * @param rScale scale of the drawn texture
 * @param rTranslate translation of the drawn texture
 * @param isBT601 whether to use the BT.601 video range conversion
 */
void ImageFilter2D::drawNV12Decord(DrawContext* pDrawContext, const TextureSampler& rSamplerY,
                                   const TextureSampler& rSamplerUV,
                                   const sead::Viewport& rViewport, const sead::Vector2f& rScale,
                                   const sead::Vector2f& rTranslate, bool isBT601)
{
    const ShaderProgram* pProgram =
        getProgramUnsafe(detail::ShaderHolder::cNv12decode)->getVariation(isBT601 ? 2 : 0);
    pProgram->activate(pDrawContext, true);
    if (isBT601)
    {
        sead::Vector4f coefficient(1.596f, -0.813f, -0.391f, 2.018f);
        setUniform(pDrawContext, pProgram->getUniformLocation(22), coefficient);
    }
    else
    {
        sead::Vector4f coefficient(1.402f, -0.714f, -0.344f, 1.722f);
        setUniform(pDrawContext, pProgram->getUniformLocation(22), coefficient);
    }
    pProgram->validate_();
    rSamplerUV.activate(pDrawContext, pProgram->getSamplerLocation(1), -1, false);
    draw(pDrawContext, *pProgram, rSamplerY, rViewport, rScale, rTranslate);
}

/**
 * Draws a YUV NV12 texture pair converted to RGB over the whole render target.
 * @param pDrawContext draw context
 * @param rSamplerY luma texture
 * @param rSamplerUV chroma texture
 * @param isBT601 whether to use the BT.601 video range conversion
 */
void ImageFilter2D::drawNV12DecordQuadTriangle(DrawContext* pDrawContext,
                                               const TextureSampler& rSamplerY,
                                               const TextureSampler& rSamplerUV, bool isBT601)
{
    s32 variation = isBT601 << 1 | 1;
    const ShaderProgram* pProgram =
        getProgramUnsafe(detail::ShaderHolder::cNv12decode)->getVariation(variation);
    pProgram->activate(pDrawContext, true);
    if (isBT601)
    {
        sead::Vector4f coefficient(1.596f, -0.813f, -0.391f, 2.018f);
        setUniform(pDrawContext, pProgram->getUniformLocation(22), coefficient);
    }
    else
    {
        sead::Vector4f coefficient(1.402f, -0.714f, -0.344f, 1.722f);
        setUniform(pDrawContext, pProgram->getUniformLocation(22), coefficient);
    }
    pProgram->validate_();
    rSamplerUV.activate(pDrawContext, pProgram->getSamplerLocation(1), -1, false);
    drawQuadTriangle(pDrawContext, *pProgram, rSamplerY);
}

/**
 * Draws one blur pass of a cube map face.
 * @param pDrawContext draw context
 * @param rSampler cube map to blur
 * @param mipLevel mip level to draw
 * @param face face to draw
 * @param type blur direction
 * @param sigma gaussian sigma
 */
void ImageFilter2D::drawCubemapGaussian(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                        u32 mipLevel, u32 face, BlurType type, f32 sigma)
{
    const ShaderProgram* pProgram =
        getProgramUnsafe(detail::ShaderHolder::cCubemapGaussian)->getVariation(0);
    u32 size = rSampler.getTextureData().getMipWidth(mipLevel);
    pProgram->activate(pDrawContext, true);
    pProgram->validate_();
    rSampler.activate(pDrawContext, pProgram->getSamplerLocation(0), -1, false);
    setUniform(pDrawContext, pProgram->getUniformLocation(3), f32(mipLevel));
    setUniform(pDrawContext, pProgram->getUniformLocation(1), face);
    f32 texSize = size;
    setUniform(pDrawContext, pProgram->getUniformLocation(5), sigma);
    setUniform(pDrawContext, pProgram->getUniformLocation(4), texSize);
    setUniform(pDrawContext, pProgram->getUniformLocation(6), cBlurDirection[type - 2]);
    drawQuad(pDrawContext);
}

/**
 * Draws one blur pass of a cube map array face.
 * @param pDrawContext draw context
 * @param rSampler cube map array to blur
 * @param slice cube to draw
 * @param mipLevel mip level to draw
 * @param face face to draw
 * @param type blur direction
 * @param sigma gaussian sigma
 */
void ImageFilter2D::drawCubemapGaussian(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                        u32 slice, u32 mipLevel, u32 face, BlurType type,
                                        f32 sigma)
{
    const ShaderProgram* pProgram =
        getProgramUnsafe(detail::ShaderHolder::cCubemapGaussian)->getVariation(1);
    u32 size = rSampler.getTextureData().getMipWidth(mipLevel);
    pProgram->activate(pDrawContext, true);
    pProgram->validate_();
    rSampler.activate(pDrawContext, pProgram->getSamplerLocation(0), -1, false);
    setUniform(pDrawContext, pProgram->getUniformLocation(3), f32(mipLevel));
    setUniform(pDrawContext, pProgram->getUniformLocation(1), face);
    setUniform(pDrawContext, pProgram->getUniformLocation(2), slice);
    f32 texSize = size;
    setUniform(pDrawContext, pProgram->getUniformLocation(5), sigma);
    setUniform(pDrawContext, pProgram->getUniformLocation(4), texSize);
    setUniform(pDrawContext, pProgram->getUniformLocation(6), cBlurDirection[type - 2]);
    drawQuad(pDrawContext);
}

/**
 * Draws the irradiance convolution of a cube map face.
 * @param pDrawContext draw context
 * @param rSampler cube map to convolve
 * @param mipLevel mip level to draw
 * @param face face to draw
 * @param rParam convolution parameters
 */
void ImageFilter2D::drawCubemapIrradiance(DrawContext* pDrawContext,
                                          const TextureSampler& rSampler, u32 mipLevel, u32 face,
                                          const sead::Vector3f& rParam)
{
    const ShaderProgram* pProgram =
        getProgramUnsafe(detail::ShaderHolder::cCubemapIrradiance)->getVariation(0);
    u32 size = rSampler.getTextureData().getMipWidth(mipLevel);
    pProgram->activate(pDrawContext, true);
    pProgram->validate_();
    rSampler.activate(pDrawContext, pProgram->getSamplerLocation(0), -1, false);
    setUniform(pDrawContext, pProgram->getUniformLocation(1), face);
    setUniform(pDrawContext, pProgram->getUniformLocation(3), rParam);
    f32 texSize = size;
    setUniform(pDrawContext, pProgram->getUniformLocation(2), 0.0f);
    setUniform(pDrawContext, pProgram->getUniformLocation(5), texSize);
    setUniform(pDrawContext, pProgram->getUniformLocation(4), f32(mipLevel));
    drawQuad(pDrawContext);
}

/**
 * Draws the irradiance convolution of a cube map array face.
 * @param pDrawContext draw context
 * @param rSampler cube map array to convolve
 * @param mipLevel mip level to draw
 * @param slice cube to draw
 * @param face face to draw
 * @param rParam convolution parameters
 */
void ImageFilter2D::drawCubemapIrradiance(DrawContext* pDrawContext,
                                          const TextureSampler& rSampler, u32 mipLevel, u32 slice,
                                          u32 face, const sead::Vector3f& rParam)
{
    const ShaderProgram* pProgram =
        getProgramUnsafe(detail::ShaderHolder::cCubemapIrradiance)->getVariation(1);
    u32 size = rSampler.getTextureData().getMipWidth(mipLevel);
    pProgram->activate(pDrawContext, true);
    pProgram->validate_();
    rSampler.activate(pDrawContext, pProgram->getSamplerLocation(0), -1, false);
    setUniform(pDrawContext, pProgram->getUniformLocation(1), face);
    setUniform(pDrawContext, pProgram->getUniformLocation(3), rParam);
    f32 sliceF = slice;
    f32 texSize = size;
    setUniform(pDrawContext, pProgram->getUniformLocation(2), sliceF);
    setUniform(pDrawContext, pProgram->getUniformLocation(5), texSize);
    setUniform(pDrawContext, pProgram->getUniformLocation(4), f32(mipLevel));
    drawQuad(pDrawContext);
}

/**
 * Projects a cube map onto spherical harmonics.
 * @param pDrawContext draw context
 * @param rSampler cube map to project
 * @param a first shader parameter
 * @param rStep sampling step in radians
 * @param weight total sample weight
 * @param pSampler optional second texture
 * @param rD first parameter of the second texture
 * @param rE second parameter of the second texture
 */
void ImageFilter2D::drawCubemapSH(DrawContext* pDrawContext, const TextureSampler& rSampler, f32 a,
                                  const sead::Vector2f& rStep, f32 weight,
                                  const TextureSampler* pSampler, const sead::Vector4f& rD,
                                  const sead::Vector4f& rE)
{
    const ShaderProgram* pBaseProgram = getProgramUnsafe(detail::ShaderHolder::cCubemapSh);
    const ShaderProgram* pProgram =
        pBaseProgram->getVariation(pBaseProgram->getVariationMacroStride(0) * (pSampler != nullptr));
    pProgram->activate(pDrawContext, true);
    pProgram->validate_();
    rSampler.activate(pDrawContext, pProgram->getSamplerLocation(0), -1, false);
    if (pSampler != nullptr)
    {
        pProgram->validate_();
        pSampler->activate(pDrawContext, pProgram->getSamplerLocation(1), -1, false);
    }

    sead::Vector3f step(rStep.x, rStep.y,
                        weight / u32(sead::Mathf::ceil(sead::Mathf::pi() / rStep.x) *
                                     sead::Mathf::floor(sead::Mathf::pi2() / rStep.y)));
    sead::Vector2f param(a, 0.0f);
    setUniform(pDrawContext, pProgram->getUniformLocation(0), step);
    setUniform(pDrawContext, pProgram->getUniformLocation(1), param);
    if (pSampler != nullptr)
    {
        setUniform(pDrawContext, pProgram->getUniformLocation(2), rD);
        setUniform(pDrawContext, pProgram->getUniformLocation(3), rE);
    }
    drawQuadTriangleQuadIndex(pDrawContext);
}

/**
 * Projects a cube map onto spherical harmonics around a point.
 * @param pDrawContext draw context
 * @param rSampler cube map to project
 * @param a first shader parameter
 * @param rPos projection point
 * @param weight total sample weight
 * @param pSampler optional second texture
 * @param rD first parameter of the second texture
 * @param rE second parameter of the second texture
 */
void ImageFilter2D::drawCubemapSHPoint(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                       f32 a, const sead::Vector3f& rPos, f32 weight,
                                       const TextureSampler* pSampler, const sead::Vector4f& rD,
                                       const sead::Vector4f& rE)
{
    const ShaderProgram* pBaseProgram = getProgramUnsafe(detail::ShaderHolder::cCubemapShPoint);
    const ShaderProgram* pProgram =
        pBaseProgram->getVariation(pBaseProgram->getVariationMacroStride(0) * (pSampler != nullptr));
    pProgram->activate(pDrawContext, true);
    pProgram->validate_();
    rSampler.activate(pDrawContext, pProgram->getSamplerLocation(0), -1, false);
    if (pSampler != nullptr)
    {
        pProgram->validate_();
        pSampler->activate(pDrawContext, pProgram->getSamplerLocation(1), -1, false);
    }

    sead::Vector3f param(0.0f, 0.0f, weight);
    setUniform(pDrawContext, pProgram->getUniformLocation(0), param);
    setUniform(pDrawContext, pProgram->getUniformLocation(1), a);
    setUniform(pDrawContext, pProgram->getUniformLocation(4), rPos);
    if (pSampler != nullptr)
    {
        setUniform(pDrawContext, pProgram->getUniformLocation(2), rD);
        setUniform(pDrawContext, pProgram->getUniformLocation(3), rE);
    }
    drawQuadTriangleQuadIndex(pDrawContext);
}

/**
 * Projects the occlusion of a cube map onto spherical harmonics.
 * @param pDrawContext draw context
 * @param rSampler cube map to project
 * @param a first shader parameter
 * @param rStep sampling step in radians
 * @param rRange depth range and third parameter
 * @param weight total sample weight
 */
void ImageFilter2D::drawCubemapSHOcclusion(DrawContext* pDrawContext,
                                           const TextureSampler& rSampler, f32 a,
                                           const sead::Vector2f& rStep,
                                           const sead::Vector3f& rRange, f32 weight)
{
    const ShaderProgram* pProgram = getProgram(detail::ShaderHolder::cCubemapShOcclusion);
    pProgram->activate(pDrawContext, true);
    pProgram->validate_();
    rSampler.activate(pDrawContext, pProgram->getSamplerLocation(0), -1, false);

    sead::Vector3f step(rStep.x, rStep.y,
                        weight / u32(sead::Mathf::ceil(sead::Mathf::pi() / rStep.x) *
                                     sead::Mathf::floor(sead::Mathf::pi2() / rStep.y)));
    sead::Vector2f param(a, a);
    setUniform(pDrawContext, pProgram->getUniformLocation(0), step);
    setUniform(pDrawContext, pProgram->getUniformLocation(1), param);
    f32 range = rRange.y - rRange.x;
    sead::Vector4f depth(rRange.x, range, 1.0f / range, rRange.x / range);
    setUniform(pDrawContext, pProgram->getUniformLocation(5), depth);
    sead::Vector4f param2(rRange.z, 0.0f, 0.0f, 0.0f);
    setUniform(pDrawContext, pProgram->getUniformLocation(6), param2);
    drawQuadTriangleQuadIndex(pDrawContext);
}

/**
 * Draws a texture over the whole render target with FXAA.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 */
void ImageFilter2D::drawFXAA(DrawContext* pDrawContext, const TextureSampler& rSampler)
{
    const TextureData& rTexture = rSampler.getTextureData();
    f32 width = getWidth(rTexture);
    f32 height = getHeight(rTexture);
    sead::Vector2f invSize(1.0f / width, 1.0f / height);
    sead::Vector2f halfSize(invSize.x * 0.5f, invSize.y * 0.5f);
    sead::Vector4f param0;
    sead::Vector4f param1;
    param0.z = halfSize.x;
    param0.w = halfSize.y;
    param1.x = halfSize.x;
    param1.y = halfSize.y;
    param0.x = invSize.x;
    param0.y = invSize.y;
    param1.z = invSize.x * 0.15f;
    param1.w = invSize.y * 0.15f;

    const ShaderProgram* pProgram = getProgram(detail::ShaderHolder::cFxaa);
    pProgram->activate(pDrawContext, true);
    setUniform(pDrawContext, pProgram->getUniformLocation(1), param0);
    setUniform(pDrawContext, pProgram->getUniformLocation(2), param1);
    setUniform(pDrawContext, pProgram->getUniformLocation(3),
               *reinterpret_cast<const sead::Vector4f*>(cFxaaLumaWeight));
    setUniform(pDrawContext, pProgram->getUniformLocation(4),
               *reinterpret_cast<const sead::Vector4f*>(cFxaaParam));
    rSampler.activate(pDrawContext, pProgram->getSamplerLocation(0), -1, false);
    drawQuadTriangle_(pDrawContext);
}

/**
 * Draws a texture over the whole render target with FXAA using a separate luma texture.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 * @param rLumaSampler luma texture
 */
void ImageFilter2D::drawFXAA(DrawContext* pDrawContext, const TextureSampler& rSampler,
                             const TextureSampler& rLumaSampler)
{
    const TextureData& rTexture = rSampler.getTextureData();
    f32 width = getWidth(rTexture);
    f32 height = getHeight(rTexture);
    sead::Vector2f invSize(1.0f / width, 1.0f / height);
    sead::Vector2f halfSize(invSize.x * 0.5f, invSize.y * 0.5f);
    sead::Vector4f param0;
    sead::Vector4f param1;
    param0.z = halfSize.x;
    param0.w = halfSize.y;
    param1.x = halfSize.x;
    param1.y = halfSize.y;
    param0.x = invSize.x;
    param0.y = invSize.y;
    param1.z = invSize.x * 0.15f;
    param1.w = invSize.y * 0.15f;

    const ShaderProgram* pProgram = getProgram(detail::ShaderHolder::cFxaa);
    pProgram->activate(pDrawContext, true);
    setUniform(pDrawContext, pProgram->getUniformLocation(1), param0);
    setUniform(pDrawContext, pProgram->getUniformLocation(2), param1);
    setUniform(pDrawContext, pProgram->getUniformLocation(3),
               *reinterpret_cast<const sead::Vector4f*>(cFxaaLumaWeight));
    rSampler.activate(pDrawContext, pProgram->getSamplerLocation(0), -1, false);
    rLumaSampler.activate(pDrawContext, pProgram->getSamplerLocation(1), -1, false);
    drawQuadTriangle_(pDrawContext);
}

/**
 * Draws a texture with gamma correction.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 * @param rViewport viewport to draw into
 * @param gamma gamma value
 * @param rScale scale of the drawn texture
 * @param rTranslate translation of the drawn texture
 */
void ImageFilter2D::drawTextureGamma(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                     const sead::Viewport& rViewport, f32 gamma,
                                     const sead::Vector2f& rScale,
                                     const sead::Vector2f& rTranslate)
{
    if (gamma == 1.0f)
    {
        drawTexture(pDrawContext, rSampler, rViewport, rScale, rTranslate);
        return;
    }

    const ShaderProgram* pProgram =
        getProgramUnsafe(detail::ShaderHolder::cTextureGamma)->getVariation(0);
    pProgram->activate(pDrawContext, true);
    setUniform(pDrawContext, pProgram->getUniformLocation(23), gamma);
    draw(pDrawContext, *pProgram, rSampler, rViewport, rScale, rTranslate);
}

/**
 * Draws a texture with gamma correction over the whole render target.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 * @param gamma gamma value
 */
void ImageFilter2D::drawTextureGammaQuadTriangle(DrawContext* pDrawContext,
                                                 const TextureSampler& rSampler, f32 gamma)
{
    if (gamma == 1.0f)
    {
        drawTextureQuadTriangle(pDrawContext, rSampler);
        return;
    }

    const ShaderProgram* pProgram =
        getProgramUnsafe(detail::ShaderHolder::cTextureGamma)->getVariation(1);
    pProgram->activate(pDrawContext, true);
    setUniform(pDrawContext, pProgram->getUniformLocation(23), gamma);
    drawQuadTriangle(pDrawContext, *pProgram, rSampler);
}

/**
 * Draws a depth texture converted to linear depth.
 * @param pDrawContext draw context
 * @param rSampler depth texture to draw
 * @param rViewport viewport to draw into
 * @param near near clip distance
 * @param far far clip distance
 * @param rScale scale of the drawn texture
 * @param rTranslate translation of the drawn texture
 */
void ImageFilter2D::drawLinearDepth(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                    const sead::Viewport& rViewport, f32 near, f32 far,
                                    const sead::Vector2f& rScale,
                                    const sead::Vector2f& rTranslate)
{
    const ShaderProgram* pProgram =
        getProgramUnsafe(detail::ShaderHolder::cDepthLinear)->getVariation(0);
    pProgram->activate(pDrawContext, true);
    setUniform(pDrawContext, pProgram->getUniformLocation(19), near);
    setUniform(pDrawContext, pProgram->getUniformLocation(20), 1.0f - near / far);
    setUniform(pDrawContext, pProgram->getUniformLocation(21), 1.0f / (far - near));
    draw(pDrawContext, *pProgram, rSampler, rViewport, rScale, rTranslate);
}

/**
 * Draws one slice of a depth array texture converted to linear depth.
 * @param pDrawContext draw context
 * @param rSampler depth texture to draw
 * @param rViewport viewport to draw into
 * @param slice slice to draw
 * @param near near clip distance
 * @param far far clip distance
 * @param rScale scale of the drawn texture
 * @param rTranslate translation of the drawn texture
 */
void ImageFilter2D::drawLinearDepthArray(DrawContext* pDrawContext,
                                         const TextureSampler& rSampler,
                                         const sead::Viewport& rViewport, s32 slice, f32 near,
                                         f32 far, const sead::Vector2f& rScale,
                                         const sead::Vector2f& rTranslate)
{
    const ShaderProgram* pProgram =
        getProgramUnsafe(detail::ShaderHolder::cDepthLinearArray)->getVariation(0);
    pProgram->activate(pDrawContext, true);
    setUniform(pDrawContext, pProgram->getUniformLocation(19), near);
    setUniform(pDrawContext, pProgram->getUniformLocation(20), 1.0f - near / far);
    setUniform(pDrawContext, pProgram->getUniformLocation(21), 1.0f / (far - near));
    setUniform(pDrawContext, pProgram->getUniformLocation(3), f32(slice));
    draw(pDrawContext, *pProgram, rSampler, rViewport, rScale, rTranslate);
}

/**
 * Draws an unsigned integer texture.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 * @param rViewport viewport to draw into
 * @param rScale value scale per channel
 * @param rDrawScale scale of the drawn texture
 * @param rTranslate translation of the drawn texture
 */
void ImageFilter2D::drawUint(DrawContext* pDrawContext, const TextureSampler& rSampler,
                             const sead::Viewport& rViewport, const sead::Vector4f& rScale,
                             const sead::Vector2f& rDrawScale, const sead::Vector2f& rTranslate)
{
    const ShaderProgram* pProgram = getProgramUnsafe(detail::ShaderHolder::cUint)->getVariation(0);
    pProgram->activate(pDrawContext, true);
    setUniform(pDrawContext, pProgram->getUniformLocation(25), rScale);
    draw(pDrawContext, *pProgram, rSampler, rViewport, rDrawScale, rTranslate);
}

/**
 * Draws one slice of an unsigned integer array texture.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 * @param rViewport viewport to draw into
 * @param slice slice to draw
 * @param rScale value scale per channel
 * @param rDrawScale scale of the drawn texture
 * @param rTranslate translation of the drawn texture
 */
void ImageFilter2D::drawUintArray(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                  const sead::Viewport& rViewport, s32 slice,
                                  const sead::Vector4f& rScale, const sead::Vector2f& rDrawScale,
                                  const sead::Vector2f& rTranslate)
{
    const ShaderProgram* pProgram =
        getProgramUnsafe(detail::ShaderHolder::cUintArray)->getVariation(0);
    pProgram->activate(pDrawContext, true);
    setUniform(pDrawContext, pProgram->getUniformLocation(25), rScale);
    setUniform(pDrawContext, pProgram->getUniformLocation(3), f32(slice));
    draw(pDrawContext, *pProgram, rSampler, rViewport, rDrawScale, rTranslate);
}

/**
 * Draws a depth texture converted to linear depth over the whole render target.
 * @param pDrawContext draw context
 * @param rSampler depth texture to draw
 * @param near near clip distance
 * @param far far clip distance
 */
void ImageFilter2D::drawLinearDepthQuadTriangle(DrawContext* pDrawContext,
                                                const TextureSampler& rSampler, f32 near, f32 far)
{
    const ShaderProgram* pProgram =
        getProgramUnsafe(detail::ShaderHolder::cDepthLinear)->getVariation(1);
    pProgram->activate(pDrawContext, true);
    setUniform(pDrawContext, pProgram->getUniformLocation(19), near);
    setUniform(pDrawContext, pProgram->getUniformLocation(20), 1.0f - near / far);
    setUniform(pDrawContext, pProgram->getUniformLocation(21), 1.0f / (far - near));
    drawQuadTriangle(pDrawContext, *pProgram, rSampler);
}

/**
 * Fills the whole render target with a color.
 * @param pDrawContext draw context
 * @param rColor fill color
 * @param depth depth to write
 */
void ImageFilter2D::drawColorQuadTriangle(DrawContext* pDrawContext, const sead::Color4f& rColor,
                                          f32 depth)
{
    const ShaderProgram* pProgram =
        getProgramUnsafe(detail::ShaderHolder::cColorQuad)->getVariation(1);
    pProgram->activate(pDrawContext, true);
    setUniform(pDrawContext, pProgram->getUniformLocation(24), depth);
    setUniform(pDrawContext, pProgram->getUniformLocation(8), rColor);
    drawQuadTriangle_(pDrawContext);
}

/**
 * Draws a depth mask of a texture over the whole render target.
 * @param pDrawContext draw context
 * @param rSampler depth texture
 */
void ImageFilter2D::drawDepthMaskQuadTriangle(DrawContext* pDrawContext,
                                              const TextureSampler& rSampler)
{
    const ShaderProgram* pProgram =
        getProgramUnsafe(detail::ShaderHolder::cDepthMask)->getVariation(1);
    pProgram->activate(pDrawContext, true);
    drawQuadTriangle(pDrawContext, *pProgram, rSampler);
}

/**
 * Draws the luminance of a texture over the whole render target.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 * @param rWeight luminance weight per channel
 */
void ImageFilter2D::drawLuminanceQuadTriangle(DrawContext* pDrawContext,
                                              const TextureSampler& rSampler,
                                              const sead::Vector3f& rWeight)
{
    const ShaderProgram* pProgram =
        getProgramUnsafe(detail::ShaderHolder::cLuminance)->getVariation(1);
    pProgram->activate(pDrawContext, true);
    sead::Vector4f weight(rWeight.x, rWeight.y, rWeight.z, 1.0f);
    setUniform(pDrawContext, pProgram->getUniformLocation(8), weight);
    drawQuadTriangle(pDrawContext, *pProgram, rSampler);
}

/**
 * Converts a height map to a normal map over the whole render target.
 * @param pDrawContext draw context
 * @param rSampler height map
 * @param a first scale
 * @param b second scale
 */
void ImageFilter2D::drawNormalMap(DrawContext* pDrawContext, const TextureSampler& rSampler, f32 a,
                                  f32 b)
{
    const ShaderProgram* pProgram =
        getProgramUnsafe(detail::ShaderHolder::cNormalMap)->getVariation(0);
    pProgram->activate(pDrawContext, true);
    sead::Vector4f param(a, b, 0.0f, 1.0f);
    setUniform(pDrawContext, pProgram->getUniformLocation(0), param);
    pProgram->validate_();
    rSampler.activate(pDrawContext, pProgram->getSamplerLocation(0), -1, false);
    drawQuadTriangle_(pDrawContext);
}

/**
 * Compresses a texture into a block compressed format.
 * @param pDrawContext draw context
 * @param rSampler texture to compress
 * @param mipLevel mip level to compress
 * @param slice slice to compress
 * @param type compression format
 */
void ImageFilter2D::drawBC(DrawContext* pDrawContext, const TextureSampler& rSampler, u32 mipLevel,
                           u32 slice, TextureCompressType type)
{
    drawBC_(pDrawContext, rSampler, mipLevel, slice, type, detail::ShaderHolder::cTextureCompress);
}

/**
 * Compresses a texture into a block compressed format with the high quality shader.
 * @param pDrawContext draw context
 * @param rSampler texture to compress
 * @param mipLevel mip level to compress
 * @param slice slice to compress
 * @param type compression format
 */
void ImageFilter2D::drawBCHQ(DrawContext* pDrawContext, const TextureSampler& rSampler,
                             u32 mipLevel, u32 slice, TextureCompressType type)
{
    drawBC_(pDrawContext, rSampler, mipLevel, slice, type,
            detail::ShaderHolder::cTextureCompressHq);
}

/**
 * Draws one slice of a 2D array color texture over the whole render target.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 * @param slice slice to draw
 */
void ImageFilter2D::draw2DArrayColorQuadTriangle(DrawContext* pDrawContext,
                                                 const TextureSampler& rSampler, s32 slice)
{
    const ShaderProgram* pProgram = getProgram(detail::ShaderHolder::cColor2dArrayQuad);
    pProgram->activate(pDrawContext, true);
    setUniform(pDrawContext, pProgram->getUniformLocation(3), f32(slice));
    drawQuadTriangle(pDrawContext, *pProgram, rSampler);
}

/**
 * Draws one slice of a 2D array depth texture over the whole render target.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 * @param slice slice to draw
 */
void ImageFilter2D::draw2DArrayDepthQuadTriangle(DrawContext* pDrawContext,
                                                 const TextureSampler& rSampler, s32 slice)
{
    const ShaderProgram* pProgram = getProgram(detail::ShaderHolder::cDepth2dArrayQuad);
    pProgram->activate(pDrawContext, true);
    setUniform(pDrawContext, pProgram->getUniformLocation(3), f32(slice));
    drawQuadTriangle(pDrawContext, *pProgram, rSampler);
}

/**
 * Draws the minimum or maximum filter of a texture over the whole render target.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 * @param isMin whether to take the minimum instead of the maximum
 * @param isDepth whether the texture is a depth texture
 */
void ImageFilter2D::draw2DMinMaxQuadTriangle(DrawContext* pDrawContext,
                                             const TextureSampler& rSampler, bool isMin,
                                             bool isDepth)
{
    s32 depthType = isMin ? detail::ShaderHolder::cDepth2dMin : detail::ShaderHolder::cDepth2dMax;
    s32 colorType = isMin ? detail::ShaderHolder::cColor2dMin : detail::ShaderHolder::cColor2dMax;
    const ShaderProgram* pProgram = getProgram(isDepth ? depthType : colorType);
    pProgram->activate(pDrawContext, true);
    drawQuadTriangle(pDrawContext, *pProgram, rSampler);
}

/**
 * Draws the minimum or maximum filter of one array slice over the whole render target.
 * @param pDrawContext draw context
 * @param rSampler texture to draw
 * @param isMin whether to take the minimum instead of the maximum
 * @param isDepth whether the texture is a depth texture
 * @param slice slice to draw
 */
void ImageFilter2D::draw2DArrayMinMaxQuadTriangle(DrawContext* pDrawContext,
                                                  const TextureSampler& rSampler, bool isMin,
                                                  bool isDepth, s32 slice)
{
    s32 depthType =
        isMin ? detail::ShaderHolder::cDepth2dArrayMin : detail::ShaderHolder::cDepth2dArrayMax;
    s32 colorType =
        isMin ? detail::ShaderHolder::cColor2dArrayMin : detail::ShaderHolder::cColor2dArrayMax;
    const ShaderProgram* pProgram = getProgram(isDepth ? depthType : colorType);
    pProgram->activate(pDrawContext, true);
    setUniform(pDrawContext, pProgram->getUniformLocation(3), f32(slice));
    drawQuadTriangle(pDrawContext, *pProgram, rSampler);
}

}  // namespace agl::utl
