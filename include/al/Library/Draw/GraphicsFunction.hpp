#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace agl {
class DrawContext;
class RenderBuffer;
}  // namespace agl

namespace nn::g3d {
class MaterialObj;
class ViewVolume;
}  // namespace nn::g3d

namespace sead {
class GraphicsContext;
class GraphicsContextMRT;
class Color4f;
}  // namespace sead

namespace al {
void calcFovByProjection(sead::Vector2f* pFov, const sead::Matrix44f& rProjMtx);
void setDepthFuncNearDraw(sead::GraphicsContext* pContext);
void setDepthFuncFarDraw(sead::GraphicsContext* pContext);
void setDepthFuncNearDraw(sead::GraphicsContextMRT* pContext);
void setDepthFuncFarDraw(sead::GraphicsContextMRT* pContext);
bool isUsingReverseProjection();
bool isDepthFuncReverse();
f32 getDepthClearValue();
void calcViewVolume(nn::g3d::ViewVolume* pViewVolume, const sead::Matrix34f& rViewMtx,
                    const sead::Matrix44f& rProjMtx);
void calcAndExpandViewVolume(nn::g3d::ViewVolume* pViewVolume, const sead::Matrix34f& rViewMtx,
                             const sead::Matrix44f& rProjMtx, const sead::Vector3f& rDir,
                             f32 expand);
bool isUseBlend(const nn::g3d::MaterialObj* pMaterial);
bool isXluBlend(const nn::g3d::MaterialObj* pMaterial);
bool isUseXluZPrepass(const nn::g3d::MaterialObj* pMaterial);
const char* getBlendMode(const nn::g3d::MaterialObj* pMaterial);
u8 getBlendFunc(const nn::g3d::MaterialObj* pMaterial, bool isSrc, bool isAlpha);
u8 getBlendEquation(const nn::g3d::MaterialObj* pMaterial, bool isAlpha);
bool getConstantColor(sead::Color4f* pColor, const nn::g3d::MaterialObj* pMaterial);
bool getDepthTestEnable(const nn::g3d::MaterialObj* pMaterial);
bool getDepthWriteEnable(const nn::g3d::MaterialObj* pMaterial);
u8 getDepthCtrlFunc(const nn::g3d::MaterialObj* pMaterial);
bool getAlphaTestEnable(const nn::g3d::MaterialObj* pMaterial);
f32 getAlphaTestValue(const nn::g3d::MaterialObj* pMaterial);
u8 getAlphaTestFunc(const nn::g3d::MaterialObj* pMaterial);
u8 getCullingMode(const nn::g3d::MaterialObj* pMaterial);
void setPolygonOffsetToContext(agl::DrawContext* pDrawContext, sead::GraphicsContext* pContext,
                               const nn::g3d::MaterialObj* pMaterial, f32 scale);
void setPolygonCtrlToContext(sead::GraphicsContext* pContext,
                             const nn::g3d::MaterialObj* pMaterial);
void setDepthCtrlToContext(sead::GraphicsContext* pContext, const nn::g3d::MaterialObj* pMaterial);
void setBlendCtrlToContext(sead::GraphicsContext* pContext, const nn::g3d::MaterialObj* pMaterial,
                           bool isForceBlend);
void setAlphaTestToContext(sead::GraphicsContext* pContext, const nn::g3d::MaterialObj* pMaterial);
void setPolygonOffsetToContext(agl::DrawContext* pDrawContext,
                               sead::GraphicsContextMRT* pContext,
                               const nn::g3d::MaterialObj* pMaterial, f32 scale);
void setPolygonCtrlToContext(sead::GraphicsContextMRT* pContext,
                             const nn::g3d::MaterialObj* pMaterial);
void setDepthCtrlToContext(sead::GraphicsContextMRT* pContext,
                           const nn::g3d::MaterialObj* pMaterial);
void setBlendCtrlToContext(sead::GraphicsContextMRT* pContext,
                           const nn::g3d::MaterialObj* pMaterial);
void setAlphaTestToContext(sead::GraphicsContextMRT* pContext,
                           const nn::g3d::MaterialObj* pMaterial);
void copyRenderBuffer(agl::DrawContext* pDrawContext, const agl::RenderBuffer& rDst,
                      const agl::RenderBuffer& rSrc);
void setContextMRT(sead::GraphicsContextMRT* pContext);
void setContextMRTBlendBcLbuf(sead::GraphicsContextMRT* pContext, bool isAlphaTest);
void setContextMRTAddBcLbuf(sead::GraphicsContextMRT* pContext, bool isAlphaTest);
void setContextMRTBlendBcNrmLbuf(sead::GraphicsContextMRT* pContext, bool isAlphaTest);
void setContextMRTBlendAll(sead::GraphicsContextMRT* pContext, bool isAlphaTest);
void setContextMRTBlendLbuf(sead::GraphicsContextMRT* pContext, bool isAlphaTest);
void setContextMRTAddLbuf(sead::GraphicsContextMRT* pContext, bool isAlphaTest);
void setContextMRTMulAddLbuf(sead::GraphicsContextMRT* pContext, bool isAlphaTest);
void setContextMRTMulLbuf(sead::GraphicsContextMRT* pContext, bool isAlphaTest);
void setContextMRTMulBc(sead::GraphicsContextMRT* pContext, bool isAlphaTest);
void setContextMRTFootPrint(sead::GraphicsContextMRT* pContext);
void setContextMRTAlphaMask(sead::GraphicsContextMRT* pContext);
void setContextMRTMiiFaceXlu(sead::GraphicsContextMRT* pContext);
void setContextMRTOnlyDepth(sead::GraphicsContextMRT* pContext);
void setContextMRTSilhouette(sead::GraphicsContextMRT* pContext);
}  // namespace al
