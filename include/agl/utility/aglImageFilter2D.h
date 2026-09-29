#pragma once

#include <math/seadVector.h>

#include "common/aglTextureEnum.h"

namespace sead {
class Color4f;
class Viewport;
}  // namespace sead

namespace agl {

class DrawContext;
class ShaderProgram;
class TextureSampler;

namespace utl {

class ImageFilter2D {
public:
    enum Channel {
        cChannel_Red = 0,
        cChannel_Green = 1,
        cChannel_Blue = 2,
        cChannel_Alpha = 3,
        cChannel_Depth = 4,
        cChannel_RGBA = 5,
    };

    enum ReduceScale {
        cReduceScale_1 = 0,
        cReduceScale_2 = 1,
        cReduceScale_4 = 2,
        cReduceScale_8 = 3,
        cReduceScale_16 = 4,
    };

    enum BlurType {
        cBlurType_0 = 0,
    };

    enum GaussianKernel {
        cGaussianKernel_0 = 0,
    };

    enum TextureCompressType {
        cTextureCompressType_0 = 0,
    };

    static void drawTexture(DrawContext* pDrawContext, const TextureSampler& rSampler,
                            const sead::Viewport& rViewport, const sead::Vector2f& rScale,
                            const sead::Vector2f& rTranslate);
    static void draw(DrawContext* pDrawContext, const ShaderProgram& rProgram,
                     const TextureSampler& rSampler, const sead::Viewport& rViewport,
                     const sead::Vector2f& rScale, const sead::Vector2f& rTranslate);
    static void drawTextureQuadTriangle(DrawContext* pDrawContext, const TextureSampler& rSampler);
    static void drawQuadTriangle(DrawContext* pDrawContext, const ShaderProgram& rProgram,
                                 const TextureSampler& rSampler);
    static void drawDepthQuadTriangle(DrawContext* pDrawContext, const TextureSampler& rSampler);
    static void drawTextureChannel(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                   const sead::Viewport& rViewport, Channel channel,
                                   const sead::Vector2f& rScale, const sead::Vector2f& rTranslate);
    static void drawTextureMipLevel(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                    const sead::Viewport& rViewport, f32 mipLevel,
                                    const sead::Vector2f& rScale,
                                    const sead::Vector2f& rTranslate);
    static void drawTexture2DArray(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                   const sead::Viewport& rViewport, s32 slice,
                                   const sead::Vector2f& rScale, const sead::Vector2f& rTranslate,
                                   f32 mipLevel);
    static void drawTexture3D(DrawContext* pDrawContext, const TextureSampler& rSampler,
                              const sead::Viewport& rViewport, f32 depth,
                              const sead::Vector2f& rScale, const sead::Vector2f& rTranslate,
                              f32 mipLevel);
    static void drawTextureCubeMap(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                   const sead::Viewport& rViewport, CubeMapFace face,
                                   const sead::Vector2f& rScale, const sead::Vector2f& rTranslate,
                                   f32 mipLevel);
    static void drawTextureCubeArray(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                     const sead::Viewport& rViewport, s32 slice, CubeMapFace face,
                                     const sead::Vector2f& rScale,
                                     const sead::Vector2f& rTranslate, f32 mipLevel);
    static void drawTextureTexCoord(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                    const sead::Viewport& rViewport,
                                    const sead::Vector2f& rTexCoordScale, f32 rotate,
                                    const sead::Vector2f& rTexCoordTranslate,
                                    const sead::Vector2f& rScale,
                                    const sead::Vector2f& rTranslate);
    static void drawTextureMSAA(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                const sead::Viewport& rViewport, const sead::Vector2f& rScale,
                                const sead::Vector2f& rTranslate);
    static void drawReduce(DrawContext* pDrawContext, const TextureSampler& rSampler,
                           const sead::Viewport& rViewport, ReduceScale scale, f32 mipLevel,
                           const sead::Vector2f& rTranslate);
    static ShaderProgram* getReduceProgram_(ReduceScale scale);
    static void drawBlur(DrawContext* pDrawContext, const TextureSampler& rSampler,
                         const sead::Viewport& rViewport, BlurType type,
                         const sead::Vector2f& rStep, const sead::Vector2f& rScale,
                         const sead::Vector2f& rTranslate);
    static const ShaderProgram* getBlurProgram_(BlurType type);
    static void drawGaussian(DrawContext* pDrawContext, const TextureSampler& rSampler,
                             const sead::Viewport& rViewport, GaussianKernel kernel,
                             bool isVertical, bool isHalf, const sead::Vector2f& rTranslate);
    static void drawCubic(DrawContext* pDrawContext, const TextureSampler& rSampler,
                          const sead::Viewport& rViewport, const sead::Vector2f& rTexCoordScale,
                          f32 rotate, const sead::Vector2f& rTexCoordTranslate,
                          const sead::Vector2f& rScale, const sead::Vector2f& rTranslate);
    static void drawAlphaModifiedTexture(DrawContext* pDrawContext,
                                         const TextureSampler& rSampler,
                                         const sead::Viewport& rViewport,
                                         const sead::Vector2f& rScale,
                                         const sead::Vector2f& rTranslate);
    static void drawColorDrift(DrawContext* pDrawContext, const TextureSampler& rSampler,
                               const sead::Viewport& rViewport, const sead::Vector2f& rDriftR,
                               const sead::Vector2f& rDriftG, const sead::Vector2f& rDriftB,
                               const sead::Vector2f& rScale, const sead::Vector2f& rTranslate);
    static void drawNV12Decord(DrawContext* pDrawContext, const TextureSampler& rSamplerY,
                               const TextureSampler& rSamplerUV, const sead::Viewport& rViewport,
                               const sead::Vector2f& rScale, const sead::Vector2f& rTranslate,
                               bool isBT709);
    static void drawNV12DecordQuadTriangle(DrawContext* pDrawContext,
                                           const TextureSampler& rSamplerY,
                                           const TextureSampler& rSamplerUV, bool isBT709);
    static void drawCubemapGaussian(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                    u32 mipLevel, u32 face, BlurType type, f32 sigma);
    static void drawCubemapGaussian(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                    u32 mipLevel, u32 slice, u32 face, BlurType type, f32 sigma);
    static void drawCubemapIrradiance(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                      u32 mipLevel, u32 face, const sead::Vector3f& rParam);
    static void drawCubemapIrradiance(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                      u32 mipLevel, u32 slice, u32 face,
                                      const sead::Vector3f& rParam);
    static void drawCubemapSH(DrawContext* pDrawContext, const TextureSampler& rSampler, f32 a,
                              const sead::Vector2f& rB, f32 c, const TextureSampler* pSampler,
                              const sead::Vector4f& rD, const sead::Vector4f& rE);
    static void drawCubemapSHPoint(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                   f32 a, const sead::Vector3f& rB, f32 c,
                                   const TextureSampler* pSampler, const sead::Vector4f& rD,
                                   const sead::Vector4f& rE);
    static void drawCubemapSHOcclusion(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                       f32 a, const sead::Vector2f& rB, const sead::Vector3f& rC,
                                       f32 d);
    static void drawFXAA(DrawContext* pDrawContext, const TextureSampler& rSampler);
    static void drawFXAA(DrawContext* pDrawContext, const TextureSampler& rSampler,
                         const TextureSampler& rLumaSampler);
    static void drawTextureGamma(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                 const sead::Viewport& rViewport, f32 gamma,
                                 const sead::Vector2f& rScale, const sead::Vector2f& rTranslate);
    static void drawTextureGammaQuadTriangle(DrawContext* pDrawContext,
                                             const TextureSampler& rSampler, f32 gamma);
    static void drawLinearDepth(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                const sead::Viewport& rViewport, f32 near, f32 far,
                                const sead::Vector2f& rScale, const sead::Vector2f& rTranslate);
    static void drawLinearDepthArray(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                     const sead::Viewport& rViewport, s32 slice, f32 near,
                                     f32 far, const sead::Vector2f& rScale,
                                     const sead::Vector2f& rTranslate);
    static void drawUint(DrawContext* pDrawContext, const TextureSampler& rSampler,
                         const sead::Viewport& rViewport, const sead::Vector4f& rScale,
                         const sead::Vector2f& rDrawScale, const sead::Vector2f& rTranslate);
    static void drawUintArray(DrawContext* pDrawContext, const TextureSampler& rSampler,
                              const sead::Viewport& rViewport, s32 slice,
                              const sead::Vector4f& rScale, const sead::Vector2f& rDrawScale,
                              const sead::Vector2f& rTranslate);
    static void drawLinearDepthQuadTriangle(DrawContext* pDrawContext,
                                            const TextureSampler& rSampler, f32 near, f32 far);
    static void drawColorQuadTriangle(DrawContext* pDrawContext, const sead::Color4f& rColor,
                                      f32 depth);
    static void drawDepthMaskQuadTriangle(DrawContext* pDrawContext,
                                          const TextureSampler& rSampler);
    static void drawLuminanceQuadTriangle(DrawContext* pDrawContext,
                                          const TextureSampler& rSampler,
                                          const sead::Vector3f& rWeight);
    static void drawNormalMap(DrawContext* pDrawContext, const TextureSampler& rSampler, f32 a,
                              f32 b);
    static void drawBC(DrawContext* pDrawContext, const TextureSampler& rSampler, u32 mipLevel,
                       u32 slice, TextureCompressType type);
    static void drawBCHQ(DrawContext* pDrawContext, const TextureSampler& rSampler, u32 mipLevel,
                         u32 slice, TextureCompressType type);
    static void draw2DArrayColorQuadTriangle(DrawContext* pDrawContext,
                                             const TextureSampler& rSampler, s32 slice);
    static void draw2DArrayDepthQuadTriangle(DrawContext* pDrawContext,
                                             const TextureSampler& rSampler, s32 slice);
    static void draw2DMinMaxQuadTriangle(DrawContext* pDrawContext,
                                         const TextureSampler& rSampler, bool isMin,
                                         bool isDepth);
    static void draw2DArrayMinMaxQuadTriangle(DrawContext* pDrawContext,
                                              const TextureSampler& rSampler, bool isMin,
                                              bool isDepth, s32 slice);
};

}  // namespace utl
}  // namespace agl
