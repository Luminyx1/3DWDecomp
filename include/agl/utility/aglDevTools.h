#pragma once

#include <gfx/seadColor.h>
#include <hostio/seadHostIOReflexible.h>
#include <math/seadBoundBox.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>
#include <prim/seadSafeString.h>

#include "common/aglTextureEnum.h"
#include "utility/aglImageFilter2D.h"

namespace sead {
class Camera;
class Controller;
class LookAtCamera;
class Projection;
class Viewport;
}  // namespace sead

namespace agl {

class DrawContext;
class RenderBuffer;
class ShaderProgram;
class TextureData;
class TextureSampler;

namespace utl {

namespace DevTools {

enum CameraControlType {
    cCameraControlType_0 = 0,
    cCameraControlType_1 = 1,
};

class PoiningControlBuffer
{
public:
    sead::Vector2f mPrevPos;
    u32 mIsActive;
};

void setMeterScale(f32 scale);
f32 getMeterScale();
f32 calcScale(f32 meter);
f32 calcMeter(f32 scale);
sead::FixedSafeString<256> getStringMinMax(f32 min, f32 max);
void setFrameSpeed(f32 speed);
f32 getFrameSpeed();
void setCameraOperationSpeed(f32 speed);
f32 getCameraOperationSpeed();
void genMessage(sead::hostio::Context* pContext);

void drawCamera(DrawContext* pDrawContext, const sead::Camera& rCamera,
                const sead::Matrix34f& rViewMtx, const sead::Matrix44f& rProjMtx, bool isFill,
                const sead::Color4f& rColor, f32 size);
void drawCamera(DrawContext* pDrawContext, const sead::Matrix34f& rCameraMtx,
                const sead::Vector3f& rTarget, const sead::Matrix34f& rViewMtx,
                const sead::Matrix44f& rProjMtx, bool isFill, const sead::Color4f& rColor,
                f32 size);
void drawCamera_(DrawContext* pDrawContext, const sead::Matrix34f& rCameraMtx,
                 const sead::Vector3f& rTarget, const sead::Matrix34f& rViewMtx,
                 const sead::Matrix44f& rProjMtx, bool isFill, const sead::Color4f& rColor,
                 f32 size);
void drawFrustum(DrawContext* pDrawContext, const sead::Camera& rCamera,
                 const sead::Projection& rProjection, const sead::Matrix34f& rViewMtx,
                 const sead::Matrix44f& rProjMtx, bool isFill, const sead::Color4f& rColor);
void drawFrustum_(DrawContext* pDrawContext, const sead::Matrix34f& rCameraMtx,
                  const sead::Matrix44f& rFrustumProjMtx, const sead::Matrix34f& rViewMtx,
                  const sead::Matrix44f& rProjMtx, bool isFill, const sead::Color4f& rColor);
void drawFrustum(DrawContext* pDrawContext, const sead::Matrix34f& rCameraMtx,
                 const sead::Projection& rProjection, const sead::Matrix34f& rViewMtx,
                 const sead::Matrix44f& rProjMtx, bool isFill, const sead::Color4f& rColor);
void drawCameraAndFrustum(DrawContext* pDrawContext, const sead::Camera& rCamera,
                          const sead::Projection& rProjection, const sead::Matrix34f& rViewMtx,
                          const sead::Matrix44f& rProjMtx, bool isFill,
                          const sead::Color4f& rCameraColor, const sead::Color4f& rFrustumColor,
                          f32 size);
void beginDrawImm(DrawContext* pDrawContext, const sead::Matrix34f& rViewMtx,
                  const sead::Matrix44f& rProjMtx);
void drawLineImm(DrawContext* pDrawContext, const sead::Vector3f& rStart,
                 const sead::Vector3f& rEnd, const sead::Color4f& rColor, f32 width);
void setUniformToDevToolsShader_(DrawContext* pDrawContext, const sead::Matrix34f& rModelMtx,
                                 const sead::Matrix34f& rViewMtx, const sead::Matrix44f& rProjMtx,
                                 const sead::Color4f& rColor, const sead::Color4f& rAmbient,
                                 const sead::Vector3f& rLightDir);
void drawAxisImm(DrawContext* pDrawContext, const sead::Matrix34f& rMtx, f32 length, f32 width,
                 f32 alpha);
void drawDirectionalLight(DrawContext* pDrawContext, const sead::Vector3f& rDir,
                          const sead::Color4f& rColor0, const sead::Color4f& rColor1,
                          const sead::Color4f& rColor2, const sead::Matrix34f& rViewMtx,
                          const sead::Matrix44f& rProjMtx, f32 size, bool isFill);
void drawPointLight(DrawContext* pDrawContext, const sead::Vector3f& rPos, f32 radius,
                    const sead::Color4f& rColor, const sead::Matrix34f& rViewMtx,
                    const sead::Matrix44f& rProjMtx);
void drawSpotLight(DrawContext* pDrawContext, const sead::Vector3f& rPos,
                   const sead::Vector3f& rDir, const sead::Color4f& rColor, f32 angle,
                   f32 length, const sead::Matrix34f& rViewMtx, const sead::Matrix44f& rProjMtx);
void drawProjLight(DrawContext* pDrawContext, const sead::Vector3f& rPos,
                   const sead::Vector3f& rDir, const sead::Vector3f& rUp,
                   const sead::Color4f& rColor, f32 a, f32 b, f32 c, f32 d, f32 e, f32 f,
                   const sead::Vector3f& rOffset, bool isOrtho, const sead::Matrix34f& rViewMtx,
                   const sead::Matrix44f& rProjMtx);
void drawCapsule(DrawContext* pDrawContext, const sead::Vector3f& rStart,
                 const sead::Vector3f& rEnd, f32 radius, const sead::Color4f& rColor,
                 const sead::Matrix34f& rViewMtx, const sead::Matrix44f& rProjMtx, bool isFill);
void drawArrow(DrawContext* pDrawContext, const sead::Vector3f& rStart, const sead::Vector3f& rEnd,
               const sead::Color4f& rColor0, const sead::Color4f& rColor1, f32 width,
               const sead::Matrix34f& rViewMtx, const sead::Matrix44f& rProjMtx);
void drawCursor(DrawContext* pDrawContext, const sead::Vector2f& rScreenSize,
                const sead::Vector2f& rPos, f32 scale);
void drawTexture(DrawContext* pDrawContext, const TextureSampler& rSampler,
                 const sead::Matrix34f& rModelMtx, const sead::Matrix44f& rProjMtx,
                 const sead::Color4f& rColor);
void drawDepthGradation(DrawContext* pDrawContext, const RenderBuffer& rRenderBuffer, u32 num,
                        const f32* pDepth, const sead::Color4f* pColor, f32 near, f32 far);
void controlCamera(sead::LookAtCamera* pCamera, const sead::Controller& rController, f32 speed,
                   CameraControlType type);
void controlCamera(sead::LookAtCamera* pCamera, const sead::Vector2f& rLeftStick,
                   const sead::Vector2f& rRightStick, f32 zoom, f32 moveUD, f32 moveLR,
                   f32 speed, bool isEnable, CameraControlType type);
void controlCameraPointer(sead::LookAtCamera* pCamera, PoiningControlBuffer* pBuffer,
                          const sead::Viewport& rViewport);
void controlCameraPointer(sead::LookAtCamera* pCamera, PoiningControlBuffer* pBuffer, f32 width,
                          f32 height);
void controlCameraPointer(sead::LookAtCamera* pCamera, PoiningControlBuffer* pBuffer,
                          bool isPress, bool isLeft, bool isRight, bool isMiddle,
                          const sead::Vector2f& rPos, bool isInside, f32 width, f32 height);
void controlCameraPointer(sead::LookAtCamera* pCamera, const sead::Vector2f& rDelta, f32 rotate,
                          f32 move, f32 zoom);
void drawFrameBuffer(DrawContext* pDrawContext, const RenderBuffer& rRenderBuffer,
                     const sead::Viewport& rViewport, ImageFilter2D::Channel channel);
void drawVisualizedDepth(DrawContext* pDrawContext, const TextureData& rTexture, s32 index,
                         const sead::Matrix34f& rModelMtx, const sead::Matrix44f& rProjMtx,
                         const sead::Matrix34f& rViewMtx, const sead::Matrix44f& rDepthProjMtx);
void drawVisualizedDepth(DrawContext* pDrawContext, const TextureData& rTexture, s32 index,
                         const sead::Matrix44f& rMtx, const sead::Matrix34f& rViewMtx,
                         const sead::Matrix44f& rDepthProjMtx);
void drawColorQuad(DrawContext* pDrawContext, const sead::Color4f& rColor,
                   const sead::Matrix34f& rModelMtx, const sead::Matrix44f& rProjMtx);
void drawColorQuadTopBottom(DrawContext* pDrawContext, const sead::Color4f& rTopColor,
                            const sead::Color4f& rBottomColor, const sead::Matrix34f& rModelMtx,
                            const sead::Matrix44f& rProjMtx);
void drawTexture_(DrawContext* pDrawContext, const ShaderProgram& rProgram,
                  const TextureSampler& rSampler, const sead::Matrix34f& rModelMtx,
                  const sead::Matrix44f& rProjMtx, const sead::Color4f& rColor);
void drawTextureChannel(DrawContext* pDrawContext, const TextureSampler& rSampler,
                        const sead::Matrix34f& rModelMtx, const sead::Matrix44f& rProjMtx,
                        ImageFilter2D::Channel channel);
void drawTextureGamma(DrawContext* pDrawContext, const TextureSampler& rSampler,
                      const sead::Matrix34f& rModelMtx, const sead::Matrix44f& rProjMtx,
                      f32 gamma);
void drawTextureMipLevel(DrawContext* pDrawContext, const TextureSampler& rSampler,
                         const sead::Matrix34f& rModelMtx, const sead::Matrix44f& rProjMtx,
                         f32 mipLevel);
void drawTexture2DArray(DrawContext* pDrawContext, const TextureSampler& rSampler,
                        const sead::Matrix34f& rModelMtx, const sead::Matrix44f& rProjMtx,
                        s32 slice, f32 mipLevel, const sead::Color4f& rColor);
void drawTexture3D(DrawContext* pDrawContext, const TextureSampler& rSampler,
                   const sead::Matrix34f& rModelMtx, const sead::Matrix44f& rProjMtx, f32 depth,
                   f32 mipLevel);
void drawTextureCubeMap(DrawContext* pDrawContext, const TextureSampler& rSampler,
                        const sead::Matrix34f& rModelMtx, const sead::Matrix44f& rProjMtx,
                        CubeMapFace face, f32 mipLevel);
void drawTextureCubeArray(DrawContext* pDrawContext, const TextureSampler& rSampler,
                          const sead::Matrix34f& rModelMtx, const sead::Matrix44f& rProjMtx,
                          s32 slice, CubeMapFace face, f32 mipLevel);
void drawTextureTexCoord(DrawContext* pDrawContext, const TextureSampler& rSampler,
                         const sead::Matrix34f& rModelMtx, const sead::Matrix44f& rProjMtx,
                         const sead::Vector2f& rTexCoordScale, f32 rotate,
                         const sead::Vector2f& rTexCoordTranslate);
void drawTextureTexCoordMultColor(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                  const sead::Matrix34f& rModelMtx,
                                  const sead::Matrix44f& rProjMtx,
                                  const sead::Vector2f& rTexCoordScale, f32 rotate,
                                  const sead::Vector2f& rTexCoordTranslate,
                                  const sead::Color4f& rColor);
void drawTextureMSAA(DrawContext* pDrawContext, const TextureSampler& rSampler,
                     const sead::Matrix34f& rModelMtx, const sead::Matrix44f& rProjMtx);
void drawNV12Decord(DrawContext* pDrawContext, const TextureSampler& rSamplerY,
                    const TextureSampler& rSamplerUV, const sead::Matrix34f& rModelMtx,
                    const sead::Matrix44f& rProjMtx);
void drawTextureColorMatrix(DrawContext* pDrawContext, const TextureSampler& rSampler,
                            const sead::Matrix34f& rModelMtx, const sead::Matrix44f& rProjMtx,
                            const sead::Matrix44f& rColorMtx, const sead::Vector4f& rColorOffset,
                            f32 mipLevel);
void drawTexture_(DrawContext* pDrawContext, const ShaderProgram& rProgram,
                  const TextureSampler& rSampler, const sead::Matrix34f& rModelMtx,
                  const sead::Matrix44f& rProjMtx, const sead::Matrix44f& rColorMtx,
                  const sead::Vector4f& rColorOffset);
void drawTexture2DArrayColorMatrix(DrawContext* pDrawContext, const TextureSampler& rSampler,
                                   const sead::Matrix34f& rModelMtx,
                                   const sead::Matrix44f& rProjMtx,
                                   const sead::Matrix44f& rColorMtx,
                                   const sead::Vector4f& rColorOffset, s32 slice, f32 mipLevel);
void drawPointImm(DrawContext* pDrawContext, const sead::Vector3f& rPos,
                  const sead::Color4f& rColor, f32 size);
void drawTriangleImm(DrawContext* pDrawContext, const sead::Vector3f& rPos0,
                     const sead::Vector3f& rPos1, const sead::Vector3f& rPos2,
                     const sead::Color4f& rColor);
void drawWireTriangleImm(DrawContext* pDrawContext, const sead::Vector3f& rPos0,
                         const sead::Vector3f& rPos1, const sead::Vector3f& rPos2,
                         const sead::Color4f& rColor, f32 width);
void drawGridImm(DrawContext* pDrawContext, f32 sizeX, f32 sizeZ, u32 divX, u32 divZ,
                 const sead::Color4f& rColor, f32 width);
void drawWireCircleImm(DrawContext* pDrawContext, const sead::Matrix34f& rMtx,
                       const sead::Color4f& rColor, f32 width, u32 divNum);
void drawBoundBoxImm(DrawContext* pDrawContext, const sead::BoundBox3f& rBox,
                     const sead::Color4f& rColor, f32 width);
void drawFan(DrawContext* pDrawContext, const sead::Matrix34f& rViewMtx,
             const sead::Matrix44f& rProjMtx, const sead::Color4f& rColor, f32 start, f32 end,
             u32 divNum, f32 offset);
void drawFan_(DrawContext* pDrawContext, u32* pNum, const sead::Matrix34f& rViewMtx,
              const sead::Matrix44f& rProjMtx, const sead::Color4f& rColor, f32 start, f32 end,
              u32 divNum, f32 offset);
void drawWireFan(DrawContext* pDrawContext, const sead::Matrix34f& rViewMtx,
                 const sead::Matrix44f& rProjMtx, const sead::Color4f& rColor, f32 start, f32 end,
                 u32 divNum, f32 offset);
void setStickReverse(bool reverse);
bool isStickReverse();
void setRotateLRReverse(bool reverse);
bool isRotateLRReverse();
void setRotateUDReverse(bool reverse);
bool isRotateUDReverse();

}  // namespace DevTools

}  // namespace utl

}  // namespace agl
