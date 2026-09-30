#pragma once

#include <basis/seadTypes.h>

namespace agl {
class DrawContext;
}

namespace nn::g3d {
class MaterialObj;
}

namespace sead {
class GraphicsContext;
class GraphicsContextMRT;
class Color4f;
}

namespace al {
void setPolygonOffsetToContext(agl::DrawContext* pDrawContext, sead::GraphicsContext* pContext,
                               const nn::g3d::MaterialObj* pMaterial, float scale);
void setPolygonCtrlToContext(sead::GraphicsContext* pContext, const nn::g3d::MaterialObj* pMaterial);
void setDepthCtrlToContext(sead::GraphicsContext* pContext, const nn::g3d::MaterialObj* pMaterial);
void setBlendCtrlToContext(sead::GraphicsContext* pContext, const nn::g3d::MaterialObj* pMaterial,
                           bool);
void setAlphaTestToContext(sead::GraphicsContext* pContext, const nn::g3d::MaterialObj* pMaterial);
bool isUsingReverseProjection();
bool isUseBlend(const nn::g3d::MaterialObj* pMaterial);
bool isXluBlend(const nn::g3d::MaterialObj* pMaterial);
const char* getBlendMode(const nn::g3d::MaterialObj* pMaterial);
u8 getBlendFunc(const nn::g3d::MaterialObj* pMaterial, bool isSrc, bool isAlpha);
u8 getBlendEquation(const nn::g3d::MaterialObj* pMaterial, bool isAlpha);
void getConstantColor(sead::Color4f* pColor, const nn::g3d::MaterialObj* pMaterial);
bool getAlphaTestEnable(const nn::g3d::MaterialObj* pMaterial);
void setPolygonCtrlToContext(sead::GraphicsContextMRT* pContext,
                             const nn::g3d::MaterialObj* pMaterial);
void setDepthCtrlToContext(sead::GraphicsContextMRT* pContext,
                           const nn::g3d::MaterialObj* pMaterial);
void setAlphaTestToContext(sead::GraphicsContextMRT* pContext,
                           const nn::g3d::MaterialObj* pMaterial);
}  // namespace al
