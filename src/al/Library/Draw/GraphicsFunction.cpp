#include "Library/Draw/GraphicsFunction.hpp"

#include <attributes.h>
#include <common/aglDrawContext.h>
#include <common/aglRenderBuffer.h>
#include <common/aglRenderTarget.h>
#include <common/aglTextureSampler.h>
#include <driver/aglGraphicsDriverMgr.h>
#include <gfx/seadColor.h>
#include <gfx/seadGraphicsContext.h>
#include <gfx/seadGraphicsContextMRT.h>
#include <gfx/seadViewport.h>
#include <math/seadMathCalcCommon.h>
#include <nn/g3d/g3d_Resources.h>
#include <nn/g3d/g3d_ViewVolume.h>
#include <nn/util/util_VectorApi.h>
#include <utility/aglImageFilter2D.h>

#include "Library/Math/MathUtil.hpp"
#include "Library/Model/Function/alModelFunction.hpp"
#include "Library/Projection/Projection.hpp"
#include "Library/Shader/DeferredRendering/SamplerLocation.hpp"
#include "Project/Base/StringUtil.hpp"

extern template void sead::Matrix44CalcCommon<f32>::inverse(Base& o, const Base& n);
extern template bool sead::Matrix34CalcCommon<f32>::inverse(Base& o, const Base& n);

/**
 * Looks up a shader option value by name.
 * @param pName Name of the shader option.
 * @return The option value string, or nullptr if the option does not exist.
 */
NOINLINE inline const char*
nn::g3d::ResShaderAssign::FindShaderOption(const char* pName) const {
    const nn::util::ResDic* dictionary = pOptionDic.Get();

    if (dictionary == nullptr) {
        return nullptr;
    }

    int index = dictionary->FindIndex(pName);

    if (index == nn::util::ResDic::Npos) {
        return nullptr;
    }

    return pOptionArray.Get()[index].Get()->GetData();
}

/**
 * Looks up a render info entry by name.
 * @param pName Name of the render info.
 * @return The render info, or nullptr if it does not exist.
 */
NOINLINE inline const nn::g3d::ResRenderInfo*
nn::g3d::ResMaterial::FindRenderInfo(const char* pName) const {
    const nn::util::ResDic* dictionary = ToData().pRenderInfoDic.Get();

    if (dictionary == nullptr) {
        return nullptr;
    }

    int index = dictionary->FindIndex(pName);

    if (index == nn::util::ResDic::Npos) {
        return nullptr;
    }

    return &ToData().pRenderInfoArray.Get()[index];
}

namespace al {

/**
 * Calculates the horizontal and vertical field of view described by a projection matrix.
 * @param pFov Receives the horizontal (x) and vertical (y) field of view in radians.
 * @param rProjMtx Matrix that maps projected points back to view space.
 */
void calcFovByProjection(sead::Vector2f* pFov, const sead::Matrix44f& rProjMtx) {
    sead::Vector4f points[6] = {
        {1.0f, 1.0f, -1.0f, 1.0f},  {1.0f, 1.0f, 1.0f, 1.0f},   {-1.0f, 1.0f, 1.0f, 1.0f},
        {-1.0f, 1.0f, -1.0f, 1.0f}, {1.0f, -1.0f, -1.0f, 1.0f}, {1.0f, -1.0f, 1.0f, 1.0f},
    };
    sead::Vector3f viewPoints[6];

#pragma clang loop unroll(disable)
    for (s32 i = 0; i < 6; i++) {
        const sead::Vector4f& point = points[i];
        f32 x = point.x * rProjMtx.m[0][0] + point.y * rProjMtx.m[0][1] +
                point.z * rProjMtx.m[0][2] + point.w * rProjMtx.m[0][3];
        f32 y = point.x * rProjMtx.m[1][0] + point.y * rProjMtx.m[1][1] +
                point.z * rProjMtx.m[1][2] + point.w * rProjMtx.m[1][3];
        f32 z = point.x * rProjMtx.m[2][0] + point.y * rProjMtx.m[2][1] +
                point.z * rProjMtx.m[2][2] + point.w * rProjMtx.m[2][3];
        f32 w = point.x * rProjMtx.m[3][0] + point.y * rProjMtx.m[3][1] +
                point.z * rProjMtx.m[3][2] + point.w * rProjMtx.m[3][3];
        f32 invW = 1.0f / w;
        viewPoints[i].x = x * invW;
        viewPoints[i].y = y * invW;
        viewPoints[i].z = z * invW;
    }

    sead::Vector3f diffRight = viewPoints[1] - viewPoints[0];
    sead::Vector3f diffLeft = viewPoints[2] - viewPoints[3];
    sead::Vector3f diffBottom = viewPoints[5] - viewPoints[4];
    sead::Vector3f right(diffRight.x, 0.0f, diffRight.z);
    sead::Vector3f left(diffLeft.x, 0.0f, diffLeft.z);
    sead::Vector3f top(0.0f, diffRight.y, diffRight.z);
    sead::Vector3f bottom(0.0f, diffBottom.y, diffBottom.z);
    tryNormalizeOrZero(&right);
    tryNormalizeOrZero(&left);
    tryNormalizeOrZero(&top);
    tryNormalizeOrZero(&bottom);

    f32 dot = right.dot(left);
    sead::Vector3f cross;
    cross.setCross(right, left);
    pFov->x = sead::Mathf::atan2(cross.length(), dot);

    dot = top.dot(bottom);
    cross.setCross(top, bottom);
    pFov->y = sead::Mathf::atan2(cross.length(), dot);
}

/**
 * Sets the depth function so that nearer fragments pass the depth test.
 * @param pContext Graphics context to modify.
 */
void setDepthFuncNearDraw(sead::GraphicsContext* pContext) {
    pContext->setDepthFunc(isProjectionReverse() ? NVN_DEPTH_FUNC_GEQUAL : NVN_DEPTH_FUNC_LEQUAL);
}

/**
 * Sets the depth function so that farther fragments pass the depth test.
 * @param pContext Graphics context to modify.
 */
void setDepthFuncFarDraw(sead::GraphicsContext* pContext) {
    pContext->setDepthFunc(isProjectionReverse() ? NVN_DEPTH_FUNC_LEQUAL : NVN_DEPTH_FUNC_GEQUAL);
}

/**
 * Sets the depth function so that nearer fragments pass the depth test.
 * @param pContext Graphics context to modify.
 */
void setDepthFuncNearDraw(sead::GraphicsContextMRT* pContext) {
    pContext->setDepthFunc(isProjectionReverse() ? NVN_DEPTH_FUNC_GEQUAL : NVN_DEPTH_FUNC_LEQUAL);
}

/**
 * Sets the depth function so that farther fragments pass the depth test.
 * @param pContext Graphics context to modify.
 */
void setDepthFuncFarDraw(sead::GraphicsContextMRT* pContext) {
    pContext->setDepthFunc(isProjectionReverse() ? NVN_DEPTH_FUNC_LEQUAL : NVN_DEPTH_FUNC_GEQUAL);
}

/**
 * @return Whether the reverse depth projection is in use.
 */
bool isUsingReverseProjection() {
    return isProjectionReverse();
}

/**
 * @return Whether depth comparisons are reversed.
 */
bool isDepthFuncReverse() {
    return isProjectionReverse();
}

/**
 * @return The value the depth buffer is cleared to.
 */
f32 getDepthClearValue() {
    return isProjectionReverse() ? 0.0f : 1.0f;
}

/**
 * Calculates the view volume of a camera.
 * @param pViewVolume Receives the view volume.
 * @param rViewMtx View matrix of the camera.
 * @param rProjMtx Projection matrix of the camera.
 */
void calcViewVolume(nn::g3d::ViewVolume* pViewVolume, const sead::Matrix34f& rViewMtx,
                    const sead::Matrix44f& rProjMtx) {
    static const sead::Vector4f sCorners[8] = {
        {-1.0f, 1.0f, -1.0f, 1.0f}, {1.0f, 1.0f, -1.0f, 1.0f},  {1.0f, -1.0f, -1.0f, 1.0f},
        {-1.0f, -1.0f, -1.0f, 1.0f}, {-1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f},
        {1.0f, -1.0f, 1.0f, 1.0f},  {-1.0f, -1.0f, 1.0f, 1.0f},
    };
    static nn::util::Vector3fType sPoints[8];

    sead::Matrix44f viewProjMtx;
    viewProjMtx.setMul(rProjMtx, rViewMtx);
    sead::Matrix44f invViewProjMtx;
    invViewProjMtx.setInverse(viewProjMtx);

    for (s32 i = 0; i < 8; i++) {
        sead::Vector4f corner = sCorners[i];
        f32 x = corner.x * invViewProjMtx.m[0][0] + corner.y * invViewProjMtx.m[0][1] +
                corner.z * invViewProjMtx.m[0][2] + corner.w * invViewProjMtx.m[0][3];
        f32 y = corner.x * invViewProjMtx.m[1][0] + corner.y * invViewProjMtx.m[1][1] +
                corner.z * invViewProjMtx.m[1][2] + corner.w * invViewProjMtx.m[1][3];
        f32 z = corner.x * invViewProjMtx.m[2][0] + corner.y * invViewProjMtx.m[2][1] +
                corner.z * invViewProjMtx.m[2][2] + corner.w * invViewProjMtx.m[2][3];
        f32 w = corner.x * invViewProjMtx.m[3][0] + corner.y * invViewProjMtx.m[3][1] +
                corner.z * invViewProjMtx.m[3][2] + corner.w * invViewProjMtx.m[3][3];
        f32 invW = 1.0f / w;
        nn::util::VectorSet(&sPoints[i], x * invW, y * invW, z * invW);
    }

    pViewVolume->GetAabb().Set(sPoints, 8);

    sead::Matrix34f invViewMtx;
    invViewMtx.setInverse(rViewMtx);
    nn::util::Vector3fType eye;
    nn::util::VectorSet(&eye, invViewMtx.m[0][3], invViewMtx.m[1][3], invViewMtx.m[2][3]);

    pViewVolume->GetPlane(0).Set(eye, sPoints[3], sPoints[0]);
    pViewVolume->GetPlane(1).Set(eye, sPoints[1], sPoints[2]);
    pViewVolume->GetPlane(2).Set(sPoints[0], sPoints[1], sPoints[2]);
    pViewVolume->GetPlane(3).Set(sPoints[4], sPoints[7], sPoints[6]);
    pViewVolume->GetPlane(4).Set(eye, sPoints[0], sPoints[1]);
    pViewVolume->GetPlane(5).Set(eye, sPoints[2], sPoints[3]);
    pViewVolume->SetPlaneCount(6);
    pViewVolume->SetUseBounds(1);
}

/**
 * Calculates the view volume of a camera and pushes the planes facing a direction outwards.
 * @param pViewVolume Receives the view volume.
 * @param rViewMtx View matrix of the camera.
 * @param rProjMtx Projection matrix of the camera.
 * @param rDir Direction (in world space) to expand the volume towards.
 * @param expand Distance to expand the volume by.
 */
void calcAndExpandViewVolume(nn::g3d::ViewVolume* pViewVolume, const sead::Matrix34f& rViewMtx,
                             const sead::Matrix44f& rProjMtx, const sead::Vector3f& rDir,
                             f32 expand) {
    calcViewVolume(pViewVolume, rViewMtx, rProjMtx);

    sead::Vector3f offset = rDir;
    f32 length = offset.length();

    if (length > 0.0f) {
        offset *= expand / length;
    }

    nn::g3d::ViewVolume expanded;
    sead::Matrix34f expandedViewMtx = rViewMtx;
    expandedViewMtx.m[0][3] -= offset.x;
    expandedViewMtx.m[1][3] -= offset.y;
    expandedViewMtx.m[2][3] -= offset.z;
    calcViewVolume(&expanded, expandedViewMtx, rProjMtx);

    sead::Vector3f dir = rDir;
    f32 dirX = dir.x * rViewMtx.m[0][0] + dir.y * rViewMtx.m[0][1] + dir.z * rViewMtx.m[0][2];
    f32 dirY = dir.x * rViewMtx.m[1][0] + dir.y * rViewMtx.m[1][1] + dir.z * rViewMtx.m[1][2];
    f32 dirZ = dir.x * rViewMtx.m[2][0] + dir.y * rViewMtx.m[2][1] + dir.z * rViewMtx.m[2][2];

    if (dirX < 0.0f) {
        pViewVolume->GetPlane(0) = expanded.GetPlane(0);
    } else if (dirX > 0.0f) {
        pViewVolume->GetPlane(1) = expanded.GetPlane(1);
    }

    if (dirY < 0.0f) {
        pViewVolume->GetPlane(5) = expanded.GetPlane(5);
    } else if (dirY > 0.0f) {
        pViewVolume->GetPlane(4) = expanded.GetPlane(4);
    }

    if (dirZ < 0.0f) {
        pViewVolume->GetPlane(3) = expanded.GetPlane(3);
    } else if (dirZ > 0.0f) {
        pViewVolume->GetPlane(2) = expanded.GetPlane(2);
    }

    pViewVolume->SetUseBounds(0);
}

/**
 * @param pMaterial Material to check.
 * @return Whether the material is rendered with blending.
 */
bool isUseBlend(const nn::g3d::MaterialObj* pMaterial) {
    const nn::g3d::ResMaterial* material = pMaterial->GetResource();
    const char* renderType = material->GetShaderAssign()->FindShaderOption("cRenderType");

    if (renderType == nullptr) {
        return false;
    }

    if (isEqualString("0", renderType)) {
        return false;
    }

    if (isEqualString("1", renderType)) {
        return true;
    }

    const nn::g3d::ResRenderInfo* forwardXlu = material->FindRenderInfo("forward_xlu");

    if (forwardXlu == nullptr) {
        return true;
    }

    const char* forwardXluType = forwardXlu->GetString(0);

    if (isEqualString("3", renderType)) {
        return !isEqualString("Opa", forwardXluType);
    }

    return false;
}

/**
 * @param pMaterial Material to check.
 * @return Whether the material is rendered in the translucent pass.
 */
bool isXluBlend(const nn::g3d::MaterialObj* pMaterial) {
    const char* renderType =
        pMaterial->GetResource()->GetShaderAssign()->FindShaderOption("cRenderType");

    if (renderType == nullptr) {
        return false;
    }

    return isEqualString(renderType, "2") || isEqualString(renderType, "3") ||
           isEqualString(renderType, "4") || isEqualString(renderType, "5");
}

/**
 * @param pMaterial Material to check.
 * @return Whether the material uses a depth prepass in the translucent pass.
 */
bool isUseXluZPrepass(const nn::g3d::MaterialObj* pMaterial) {
    const nn::g3d::ResRenderInfo* info =
        pMaterial->GetResource()->FindRenderInfo("enable_xlu_zprepass");

    if (info == nullptr) {
        return false;
    }

    return isEqualString(info->GetString(0), "true");
}

/**
 * @param pMaterial Material to check.
 * @return "Custom" if the material uses custom blend settings, "Blend" otherwise.
 */
const char* getBlendMode(const nn::g3d::MaterialObj* pMaterial) {
    const nn::g3d::ResRenderInfo* info =
        pMaterial->GetResource()->FindRenderInfo("enable_color_blend_custom");

    if (info != nullptr && isEqualString(info->GetString(0), "true")) {
        return "Custom";
    }

    return "Blend";
}

/**
 * @param pMaterial Material to read.
 * @param isSrc Whether to read the source (true) or destination (false) factor.
 * @param isAlpha Whether to read the alpha (true) or color (false) factor.
 * @return The blend factor of the material.
 */
u8 getBlendFunc(const nn::g3d::MaterialObj* pMaterial, bool isSrc, bool isAlpha) {
    const nn::g3d::ResRenderInfo* info = pMaterial->GetResource()->FindRenderInfo(
        isAlpha ? (isSrc ? "color_blend_alpha_src_func" : "color_blend_alpha_dst_func") :
                  (isSrc ? "color_blend_rgb_src_func" : "color_blend_rgb_dst_func"));

    if (info != nullptr) {
        const char* func = info->GetString(0);

        if (isEqualString(func, "zero")) {
            return NVN_BLEND_FUNC_ZERO;
        }

        if (isEqualString(func, "one")) {
            return NVN_BLEND_FUNC_ONE;
        }

        if (isEqualString(func, "src_color")) {
            return NVN_BLEND_FUNC_SRC_COLOR;
        }

        if (isEqualString(func, "one_minus_src_color")) {
            return NVN_BLEND_FUNC_ONE_MINUS_SRC_COLOR;
        }

        if (isEqualString(func, "src_alpha")) {
            return NVN_BLEND_FUNC_SRC_ALPHA;
        }

        if (isEqualString(func, "one_minus_src_alpha")) {
            return NVN_BLEND_FUNC_ONE_MINUS_SRC_ALPHA;
        }

        if (isEqualString(func, "dst_alpha")) {
            return NVN_BLEND_FUNC_DST_ALPHA;
        }

        if (isEqualString(func, "one_minus_dst_alpha")) {
            return NVN_BLEND_FUNC_ONE_MINUS_DST_ALPHA;
        }

        if (isEqualString(func, "const_color")) {
            return NVN_BLEND_FUNC_CONSTANT_COLOR;
        }

        if (isEqualString(func, "one_minus_const_color")) {
            return NVN_BLEND_FUNC_ONE_MINUS_CONSTANT_COLOR;
        }

        if (isEqualString(func, "const_alpha")) {
            return NVN_BLEND_FUNC_CONSTANT_ALPHA;
        }

        if (isEqualString(func, "one_minus_const_alpha")) {
            return NVN_BLEND_FUNC_ONE_MINUS_CONSTANT_ALPHA;
        }

        if (isEqualString(func, "src_alpha_saturate")) {
            return NVN_BLEND_FUNC_SRC_ALPHA_SATURATE;
        }
    }

    return NVN_BLEND_FUNC_SRC_ALPHA;
}

/**
 * @param pMaterial Material to read.
 * @param isAlpha Whether to read the alpha (true) or color (false) equation.
 * @return The blend equation of the material.
 */
u8 getBlendEquation(const nn::g3d::MaterialObj* pMaterial, bool isAlpha) {
    const nn::g3d::ResRenderInfo* info = pMaterial->GetResource()->FindRenderInfo(
        isAlpha ? "color_blend_alpha_op" : "color_blend_rgb_op");

    if (info != nullptr) {
        const char* op = info->GetString(0);

        if (isEqualString(op, "add")) {
            return NVN_BLEND_EQUATION_ADD;
        }

        if (isEqualString(op, "src_minus_dst")) {
            return NVN_BLEND_EQUATION_SUB;
        }

        if (isEqualString(op, "min")) {
            return NVN_BLEND_EQUATION_MIN;
        }

        if (isEqualString(op, "max")) {
            return NVN_BLEND_EQUATION_MAX;
        }

        if (isEqualString(op, "dst_minus_src")) {
            return NVN_BLEND_EQUATION_REVERSE_SUB;
        }
    }

    return NVN_BLEND_EQUATION_ADD;
}

/**
 * Reads the blend constant color of a material.
 * @param pColor Receives the color. Left untouched if the material has none.
 * @param pMaterial Material to read.
 * @return Whether the material has a blend constant color.
 */
bool getConstantColor(sead::Color4f* pColor, const nn::g3d::MaterialObj* pMaterial) {
    const nn::g3d::ResRenderInfo* info =
        pMaterial->GetResource()->FindRenderInfo("color_blend_const_color");

    if (info == nullptr) {
        return false;
    }

    const f32* color = info->GetFloat();
    *pColor = sead::Color4f(color[0], color[1], color[2], color[3]);
    return true;
}

/**
 * @param pMaterial Material to read.
 * @return Whether the material enables the depth test.
 */
bool getDepthTestEnable(const nn::g3d::MaterialObj* pMaterial) {
    const nn::g3d::ResRenderInfo* info =
        pMaterial->GetResource()->FindRenderInfo("enable_depth_test");

    if (info == nullptr) {
        return true;
    }

    return isEqualString(info->GetString(0), "true");
}

/**
 * @param pMaterial Material to read.
 * @return Whether the material enables depth writes.
 */
bool getDepthWriteEnable(const nn::g3d::MaterialObj* pMaterial) {
    const nn::g3d::ResRenderInfo* info =
        pMaterial->GetResource()->FindRenderInfo("enable_depth_write");

    if (info == nullptr) {
        return true;
    }

    return isEqualString(info->GetString(0), "true");
}

/**
 * @param pMaterial Material to read.
 * @return The depth function of the material, adjusted for reverse depth.
 */
u8 getDepthCtrlFunc(const nn::g3d::MaterialObj* pMaterial) {
    const nn::g3d::ResRenderInfo* info =
        pMaterial->GetResource()->FindRenderInfo("depth_test_func");

    if (info != nullptr) {
        const char* func = info->GetString(0);

        if (isDepthFuncReverse()) {
            if (isEqualString(func, "Lequal")) {
                return NVN_DEPTH_FUNC_GEQUAL;
            }

            if (isEqualString(func, "Less")) {
                return NVN_DEPTH_FUNC_GREATER;
            }

            if (isEqualString(func, "Greater")) {
                return NVN_DEPTH_FUNC_LESS;
            }

            if (isEqualString(func, "Gequal")) {
                return NVN_DEPTH_FUNC_LEQUAL;
            }
        } else {
            if (isEqualString(func, "Lequal")) {
                return NVN_DEPTH_FUNC_LEQUAL;
            }

            if (isEqualString(func, "Less")) {
                return NVN_DEPTH_FUNC_LESS;
            }

            if (isEqualString(func, "Greater")) {
                return NVN_DEPTH_FUNC_GREATER;
            }

            if (isEqualString(func, "Gequal")) {
                return NVN_DEPTH_FUNC_GEQUAL;
            }
        }

        if (isEqualString(func, "Always")) {
            return NVN_DEPTH_FUNC_ALWAYS;
        }

        if (isEqualString(func, "Never")) {
            return NVN_DEPTH_FUNC_NEVER;
        }

        if (isEqualString(func, "Equal")) {
            return NVN_DEPTH_FUNC_EQUAL;
        }

        if (isEqualString(func, "Nequal")) {
            return NVN_DEPTH_FUNC_NOTEQUAL;
        }
    }

    return NVN_DEPTH_FUNC_LEQUAL;
}

/**
 * @param pMaterial Material to read.
 * @return Whether the material uses alpha testing.
 */
bool getAlphaTestEnable(const nn::g3d::MaterialObj* pMaterial) {
    return alModelFunction::isShaderAssignAlphaMask(pMaterial);
}

/**
 * @param pMaterial Material to read.
 * @return The alpha test reference value of the material.
 */
f32 getAlphaTestValue(const nn::g3d::MaterialObj* pMaterial) {
    f32 value = 0.5f;
    s32 index = pMaterial->GetResource()->FindShaderParamIndex("alpha_test_value");

    if (index >= 0) {
        value = *pMaterial->GetShaderParam<f32>(index);
    }

    return value;
}

/**
 * @param pMaterial Material to read.
 * @return The alpha test function of the material.
 */
u8 getAlphaTestFunc(const nn::g3d::MaterialObj* pMaterial) {
    const char* func = alModelFunction::getShaderAssignAlphaFunc(pMaterial);

    if (func == nullptr) {
        return NVN_ALPHA_FUNC_GEQUAL;
    }

    if (isEqualString(func, "0")) {
        return NVN_ALPHA_FUNC_NEVER;
    }

    if (isEqualString(func, "10")) {
        return NVN_ALPHA_FUNC_LESS;
    }

    if (isEqualString(func, "20")) {
        return NVN_ALPHA_FUNC_EQUAL;
    }

    if (isEqualString(func, "30")) {
        return NVN_ALPHA_FUNC_LEQUAL;
    }

    if (isEqualString(func, "40")) {
        return NVN_ALPHA_FUNC_GREATER;
    }

    if (isEqualString(func, "50")) {
        return NVN_ALPHA_FUNC_NOTEQUAL;
    }

    if (isEqualString(func, "60")) {
        return NVN_ALPHA_FUNC_GEQUAL;
    }

    if (isEqualString(func, "70")) {
        return NVN_ALPHA_FUNC_ALWAYS;
    }

    return NVN_ALPHA_FUNC_GEQUAL;
}

/**
 * @param pMaterial Material to read.
 * @return The culling mode of the material.
 */
u8 getCullingMode(const nn::g3d::MaterialObj* pMaterial) {
    const nn::g3d::ResRenderInfo* info = pMaterial->GetResource()->FindRenderInfo("display_face");

    if (info != nullptr) {
        const char* face = info->GetString(0);

        if (isEqualString(face, "back")) {
            return NVN_FACE_FRONT;
        }

        if (isEqualString(face, "front")) {
            return NVN_FACE_BACK;
        }

        if (isEqualString(face, "both")) {
            return NVN_FACE_NONE;
        }

        if (isEqualString(face, "none")) {
            return NVN_FACE_FRONT_AND_BACK;
        }
    }

    return NVN_FACE_BACK;
}

/**
 * Applies the polygon offset settings of a material.
 * @param pDrawContext Draw context to apply the polygon offset to.
 * @param pContext Graphics context to modify.
 * @param pMaterial Material to read.
 * @param scale Polygon offset factor.
 */
void setPolygonOffsetToContext(agl::DrawContext* pDrawContext, sead::GraphicsContext* pContext,
                               const nn::g3d::MaterialObj* pMaterial, f32 scale) {
    const nn::g3d::ResRenderInfo* info =
        pMaterial->GetResource()->FindRenderInfo("enable_polygon_offset");

    if (info != nullptr && isEqualString(info->GetString(0), "true")) {
        const nn::g3d::ResRenderInfo* valueInfo =
            pMaterial->GetResource()->FindRenderInfo("polygon_offset_value");
        f32 value = -1.0f;

        if (valueInfo != nullptr && valueInfo->GetFloat() != nullptr) {
            value = valueInfo->GetFloat()[0];
        }

        pContext->setPolygonOffsetFrontEnable(true);
        agl::driver::GraphicsDriverMgr::instance()->setPolygonOffset(pDrawContext, scale, value);
        return;
    }

    pContext->setPolygonOffsetFrontEnable(false);
}

/**
 * Applies the culling settings of a material.
 * @param pContext Graphics context to modify.
 * @param pMaterial Material to read.
 */
void setPolygonCtrlToContext(sead::GraphicsContext* pContext,
                             const nn::g3d::MaterialObj* pMaterial) {
    pContext->setCullingMode(getCullingMode(pMaterial));
}

/**
 * Applies the depth settings of a material.
 * @param pContext Graphics context to modify.
 * @param pMaterial Material to read.
 */
void setDepthCtrlToContext(sead::GraphicsContext* pContext, const nn::g3d::MaterialObj* pMaterial) {
    pContext->setDepthTestEnable(getDepthTestEnable(pMaterial));
    pContext->setDepthWriteEnable(getDepthWriteEnable(pMaterial));
    pContext->setDepthFunc(getDepthCtrlFunc(pMaterial));

    if (isUseBlend(pMaterial)) {
        isEqualString(getBlendMode(pMaterial), "Custom");
    }
}

/**
 * Applies the blend settings of a material.
 * @param pContext Graphics context to modify.
 * @param pMaterial Material to read.
 * @param isForceBlend Whether to enable blending even if the material does not use it.
 */
void setBlendCtrlToContext(sead::GraphicsContext* pContext, const nn::g3d::MaterialObj* pMaterial,
                           bool isForceBlend) {
    bool isBlend = isForceBlend || isUseBlend(pMaterial);
    pContext->setBlendEnable(isBlend);

    if (!isBlend) {
        return;
    }

    const char* blendMode = getBlendMode(pMaterial);

    if (isEqualString(blendMode, "Custom")) {
        pContext->setBlendFactorSrcRGB(0, getBlendFunc(pMaterial, true, false));
        pContext->setBlendFactorDstRGB(0, getBlendFunc(pMaterial, false, false));
        pContext->setBlendEquationRGB(0, getBlendEquation(pMaterial, false));
        pContext->setBlendFactorSrcA(0, getBlendFunc(pMaterial, true, true));
        pContext->setBlendFactorDstA(0, getBlendFunc(pMaterial, false, true));
        pContext->setBlendEquationA(0, getBlendEquation(pMaterial, true));

        sead::Color4f color(1.0f, 1.0f, 1.0f, 1.0f);
        getConstantColor(&color, pMaterial);
        pContext->setBlendConstantColor(color);
    } else if (isEqualString(blendMode, "Add")) {
        pContext->setBlendFactorSrcRGB(0, NVN_BLEND_FUNC_SRC_ALPHA);
        pContext->setBlendFactorDstRGB(0, NVN_BLEND_FUNC_ONE);
        pContext->setBlendEquationRGB(0, NVN_BLEND_EQUATION_ADD);
        pContext->setBlendFactorSrcA(0, NVN_BLEND_FUNC_ZERO);
        pContext->setBlendFactorDstA(0, NVN_BLEND_FUNC_ONE);
        pContext->setBlendEquationA(0, NVN_BLEND_EQUATION_ADD);
    } else if (isEqualString(blendMode, "Sub")) {
        pContext->setBlendFactorSrcRGB(0, NVN_BLEND_FUNC_SRC_ALPHA);
        pContext->setBlendFactorDstRGB(0, NVN_BLEND_FUNC_ONE);
        pContext->setBlendEquationRGB(0, NVN_BLEND_EQUATION_REVERSE_SUB);
        pContext->setBlendFactorSrcA(0, NVN_BLEND_FUNC_ZERO);
        pContext->setBlendFactorDstA(0, NVN_BLEND_FUNC_ONE);
        pContext->setBlendEquationA(0, NVN_BLEND_EQUATION_ADD);
    } else if (isEqualString(blendMode, "Mul")) {
        pContext->setBlendFactorSrcRGB(0, NVN_BLEND_FUNC_ZERO);
        pContext->setBlendFactorDstRGB(0, NVN_BLEND_FUNC_SRC_COLOR);
        pContext->setBlendEquationRGB(0, NVN_BLEND_EQUATION_ADD);
        pContext->setBlendFactorSrcA(0, NVN_BLEND_FUNC_ZERO);
        pContext->setBlendFactorDstA(0, NVN_BLEND_FUNC_ONE);
        pContext->setBlendEquationA(0, NVN_BLEND_EQUATION_ADD);
    } else if (isEqualString(blendMode, "Blend")) {
        pContext->setBlendFactorSrcRGB(0, getBlendFunc(pMaterial, true, false));
        pContext->setBlendFactorDstRGB(0, getBlendFunc(pMaterial, false, false));
        pContext->setBlendEquationRGB(0, getBlendEquation(pMaterial, false));
        pContext->setBlendFactorSrcA(0, getBlendFunc(pMaterial, true, true));
        pContext->setBlendFactorDstA(0, getBlendFunc(pMaterial, false, true));
        pContext->setBlendEquationA(0, getBlendEquation(pMaterial, true));

        sead::Color4f color(1.0f, 1.0f, 1.0f, 1.0f);
        getConstantColor(&color, pMaterial);
        pContext->setBlendConstantColor(color);
    }
}

/**
 * Applies the alpha test settings of a material.
 * @param pContext Graphics context to modify.
 * @param pMaterial Material to read.
 */
void setAlphaTestToContext(sead::GraphicsContext* pContext, const nn::g3d::MaterialObj* pMaterial) {
    pContext->setAlphaTestEnable(getAlphaTestEnable(pMaterial));
    u8 func = getAlphaTestFunc(pMaterial);
    f32 ref = getAlphaTestValue(pMaterial);
    pContext->setAlphaTestFunc(func);
    pContext->setAlphaTestRef(ref);
}

/**
 * Applies the polygon offset settings of a material.
 * @param pDrawContext Draw context to apply the polygon offset to.
 * @param pContext Graphics context to modify.
 * @param pMaterial Material to read.
 * @param scale Polygon offset factor.
 */
void setPolygonOffsetToContext(agl::DrawContext* pDrawContext,
                               sead::GraphicsContextMRT* pContext,
                               const nn::g3d::MaterialObj* pMaterial, f32 scale) {
    const nn::g3d::ResRenderInfo* info =
        pMaterial->GetResource()->FindRenderInfo("enable_polygon_offset");

    if (info != nullptr && isEqualString(info->GetString(0), "1")) {
        const nn::g3d::ResRenderInfo* valueInfo =
            pMaterial->GetResource()->FindRenderInfo("polygon_offset_value");
        f32 value = -1.0f;

        if (valueInfo != nullptr && valueInfo->GetFloat() != nullptr) {
            value = valueInfo->GetFloat()[0];
        }

        pContext->setPolygonOffsetFrontEnable(true);
        agl::driver::GraphicsDriverMgr::instance()->setPolygonOffset(pDrawContext, scale, value);
        return;
    }

    pContext->setPolygonOffsetFrontEnable(false);
}

/**
 * Applies the culling settings of a material.
 * @param pContext Graphics context to modify.
 * @param pMaterial Material to read.
 */
void setPolygonCtrlToContext(sead::GraphicsContextMRT* pContext,
                             const nn::g3d::MaterialObj* pMaterial) {
    pContext->setCullingMode(getCullingMode(pMaterial));
}

/**
 * Applies the depth settings of a material.
 * @param pContext Graphics context to modify.
 * @param pMaterial Material to read.
 */
void setDepthCtrlToContext(sead::GraphicsContextMRT* pContext,
                           const nn::g3d::MaterialObj* pMaterial) {
    pContext->setDepthTestEnable(getDepthTestEnable(pMaterial));
    pContext->setDepthWriteEnable(getDepthWriteEnable(pMaterial));
    pContext->setDepthFunc(getDepthCtrlFunc(pMaterial));
}

/**
 * Blend settings are not applied to multiple render target contexts.
 * @param pContext Graphics context.
 * @param pMaterial Material.
 */
void setBlendCtrlToContext(sead::GraphicsContextMRT* pContext,
                           const nn::g3d::MaterialObj* pMaterial) {}

/**
 * Applies the alpha test settings of a material.
 * @param pContext Graphics context to modify.
 * @param pMaterial Material to read.
 */
void setAlphaTestToContext(sead::GraphicsContextMRT* pContext,
                           const nn::g3d::MaterialObj* pMaterial) {
    pContext->setAlphaTestEnable(getAlphaTestEnable(pMaterial));
    u8 func = getAlphaTestFunc(pMaterial);
    f32 ref = getAlphaTestValue(pMaterial);
    pContext->setAlphaTestFunc(func);
    pContext->setAlphaTestRef(ref);
}

/**
 * Copies the first color target of a render buffer into another one.
 * @param pDrawContext Draw context to draw with.
 * @param rDst Render buffer to copy into.
 * @param rSrc Render buffer to copy from.
 */
void copyRenderBuffer(agl::DrawContext* pDrawContext, const agl::RenderBuffer& rDst,
                      const agl::RenderBuffer& rSrc) {
    sead::GraphicsContext context;
    context.setColorMask(true, true, true, true);
    context.setDepthEnable(false, false);
    context.setBlendEnable(false);
    context.apply(pDrawContext);

    const agl::TextureData* dstTexture = rDst.getRenderTargetColor();
    const agl::TextureData* srcTexture = rSrc.getRenderTargetColor();
    sead::Vector2f scale(f32(dstTexture->getWidth(0)) / f32(srcTexture->getWidth(0)),
                         f32(dstTexture->getHeight(0)) / f32(srcTexture->getHeight(0)));

    sead::Viewport viewport(rDst);
    viewport.apply(pDrawContext, rDst);
    rDst.bind(pDrawContext);
    tryChangeShaderMode(pDrawContext, agl::ShaderMode(1));

    agl::TextureSampler sampler;
    sampler.applyTextureData(*srcTexture);
    sampler.setWrapDirect(1, 1, 1);
    agl::utl::ImageFilter2D::drawTexture(pDrawContext, sampler, viewport, scale,
                                         sead::Vector2f::zero);
}

/**
 * Sets up a multiple render target context for opaque drawing.
 * @param pContext Graphics context to modify.
 */
void setContextMRT(sead::GraphicsContextMRT* pContext) {
    setDepthFuncNearDraw(pContext);
    pContext->setColorMask(0xfffff);
    pContext->setBlendEnableMask(0);
}

/**
 * Sets up a multiple render target context for blending into the base color and light buffers.
 * @param pContext Graphics context to modify.
 * @param isAlphaTest Whether to enable alpha testing.
 */
void setContextMRTBlendBcLbuf(sead::GraphicsContextMRT* pContext, bool isAlphaTest) {
    pContext->setBlendEnableMask(0x9);
    pContext->setColorMask(0x700f);
    pContext->setAlphaTestEnable(isAlphaTest);
    pContext->setDepthEnable(true, false);
    pContext->setBlendFactorSrcRGB(3, NVN_BLEND_FUNC_SRC_ALPHA);
    pContext->setBlendFactorDstRGB(3, NVN_BLEND_FUNC_ONE_MINUS_SRC_ALPHA);
    pContext->setBlendEquationRGB(3, NVN_BLEND_EQUATION_ADD);
    pContext->setBlendFactorSrcRGB(0, NVN_BLEND_FUNC_SRC_ALPHA);
    pContext->setBlendFactorDstRGB(0, NVN_BLEND_FUNC_ONE_MINUS_SRC_ALPHA);
    pContext->setBlendEquationRGB(0, NVN_BLEND_EQUATION_ADD);
}

/**
 * Sets up a multiple render target context for blending into the base color buffer and adding into
 * the light buffer.
 * @param pContext Graphics context to modify.
 * @param isAlphaTest Whether to enable alpha testing.
 */
void setContextMRTAddBcLbuf(sead::GraphicsContextMRT* pContext, bool isAlphaTest) {
    pContext->setBlendEnableMask(0x9);
    pContext->setColorMask(0x700f);
    pContext->setAlphaTestEnable(isAlphaTest);
    pContext->setDepthEnable(true, false);
    pContext->setBlendFactorSrcRGB(3, NVN_BLEND_FUNC_SRC_ALPHA);
    pContext->setBlendFactorDstRGB(3, NVN_BLEND_FUNC_ONE);
    pContext->setBlendEquationRGB(3, NVN_BLEND_EQUATION_ADD);
    pContext->setBlendFactorSrcRGB(0, NVN_BLEND_FUNC_SRC_ALPHA);
    pContext->setBlendFactorDstRGB(0, NVN_BLEND_FUNC_ONE);
    pContext->setBlendEquationRGB(0, NVN_BLEND_EQUATION_ADD);
}

/**
 * Sets up a multiple render target context for blending into the base color, normal and light
 * buffers.
 * @param pContext Graphics context to modify.
 * @param isAlphaTest Whether to enable alpha testing.
 */
void setContextMRTBlendBcNrmLbuf(sead::GraphicsContextMRT* pContext, bool isAlphaTest) {
    pContext->setBlendEnableMask(0xb);
    pContext->setColorMask(0x70ff);
    pContext->setAlphaTestEnable(isAlphaTest);
    pContext->setDepthEnable(true, false);
    pContext->setBlendFactorSrcRGB(3, NVN_BLEND_FUNC_SRC_ALPHA);
    pContext->setBlendFactorDstRGB(3, NVN_BLEND_FUNC_ONE_MINUS_SRC_ALPHA);
    pContext->setBlendEquationRGB(3, NVN_BLEND_EQUATION_ADD);
    pContext->setBlendFactorSrcRGB(1, NVN_BLEND_FUNC_SRC_ALPHA);
    pContext->setBlendFactorDstRGB(1, NVN_BLEND_FUNC_ONE_MINUS_SRC_ALPHA);
    pContext->setBlendEquationRGB(1, NVN_BLEND_EQUATION_ADD);
    pContext->setBlendFactorSrcRGB(0, NVN_BLEND_FUNC_SRC_ALPHA);
    pContext->setBlendFactorDstRGB(0, NVN_BLEND_FUNC_ONE_MINUS_SRC_ALPHA);
    pContext->setBlendEquationRGB(0, NVN_BLEND_EQUATION_ADD);
}

/**
 * Sets up a multiple render target context for blending into all buffers.
 * @param pContext Graphics context to modify.
 * @param isAlphaTest Whether to enable alpha testing.
 */
void setContextMRTBlendAll(sead::GraphicsContextMRT* pContext, bool isAlphaTest) {
    pContext->setBlendEnableMask(0xb);
    pContext->setColorMask(0xf0ff);
    pContext->setAlphaTestEnable(isAlphaTest);
    pContext->setDepthEnable(true, false);
    pContext->setBlendFactorSrcRGB(3, NVN_BLEND_FUNC_SRC_ALPHA);
    pContext->setBlendFactorDstRGB(3, NVN_BLEND_FUNC_ONE_MINUS_SRC_ALPHA);
    pContext->setBlendEquationRGB(3, NVN_BLEND_EQUATION_ADD);
    pContext->setBlendFactorSrcRGB(1, NVN_BLEND_FUNC_SRC_ALPHA);
    pContext->setBlendFactorDstRGB(1, NVN_BLEND_FUNC_ONE_MINUS_SRC_ALPHA);
    pContext->setBlendEquationRGB(1, NVN_BLEND_EQUATION_ADD);
    pContext->setBlendFactorSrcRGB(0, NVN_BLEND_FUNC_SRC_ALPHA);
    pContext->setBlendFactorDstRGB(0, NVN_BLEND_FUNC_ONE_MINUS_SRC_ALPHA);
    pContext->setBlendEquationRGB(0, NVN_BLEND_EQUATION_ADD);
}

/**
 * Sets up a multiple render target context for blending into the light buffer.
 * @param pContext Graphics context to modify.
 * @param isAlphaTest Whether to enable alpha testing.
 */
void setContextMRTBlendLbuf(sead::GraphicsContextMRT* pContext, bool isAlphaTest) {
    pContext->setAlphaTestFunc(NVN_ALPHA_FUNC_GREATER);
    pContext->setAlphaTestRef(0.5f);
    pContext->setAlphaTestEnable(isAlphaTest);
    pContext->setColorMask(0xf);
    pContext->setBlendEnableMask(0x1);
    pContext->setDepthEnable(true, false);
    pContext->setBlendFactorSrcRGB(0, NVN_BLEND_FUNC_SRC_ALPHA);
    pContext->setBlendFactorDstRGB(0, NVN_BLEND_FUNC_ONE_MINUS_SRC_ALPHA);
    pContext->setBlendEquationRGB(0, NVN_BLEND_EQUATION_ADD);
}

/**
 * Sets up a multiple render target context for adding into the light buffer.
 * @param pContext Graphics context to modify.
 * @param isAlphaTest Whether to enable alpha testing.
 */
void setContextMRTAddLbuf(sead::GraphicsContextMRT* pContext, bool isAlphaTest) {
    pContext->setAlphaTestFunc(NVN_ALPHA_FUNC_GREATER);
    pContext->setAlphaTestRef(0.5f);
    pContext->setAlphaTestEnable(isAlphaTest);
    pContext->setColorMask(0xf);
    pContext->setBlendEnableMask(0x1);
    pContext->setDepthEnable(true, false);
    pContext->setBlendFactorSrcRGB(0, NVN_BLEND_FUNC_SRC_ALPHA);
    pContext->setBlendFactorDstRGB(0, NVN_BLEND_FUNC_ONE);
    pContext->setBlendEquationRGB(0, NVN_BLEND_EQUATION_ADD);
}

/**
 * Sets up a multiple render target context for multiplying and adding into the light buffer.
 * @param pContext Graphics context to modify.
 * @param isAlphaTest Whether to enable alpha testing.
 */
void setContextMRTMulAddLbuf(sead::GraphicsContextMRT* pContext, bool isAlphaTest) {
    pContext->setAlphaTestFunc(NVN_ALPHA_FUNC_GREATER);
    pContext->setAlphaTestRef(0.5f);
    pContext->setAlphaTestEnable(isAlphaTest);
    pContext->setColorMask(0xf);
    pContext->setBlendEnableMask(0x1);
    pContext->setDepthEnable(true, false);
    pContext->setBlendFactorSrcRGB(0, NVN_BLEND_FUNC_DST_COLOR);
    pContext->setBlendFactorDstRGB(0, NVN_BLEND_FUNC_ONE);
    pContext->setBlendEquationRGB(0, NVN_BLEND_EQUATION_ADD);
}

/**
 * Sets up a multiple render target context for multiplying into the light buffer.
 * @param pContext Graphics context to modify.
 * @param isAlphaTest Whether to enable alpha testing.
 */
void setContextMRTMulLbuf(sead::GraphicsContextMRT* pContext, bool isAlphaTest) {
    pContext->setAlphaTestFunc(NVN_ALPHA_FUNC_GREATER);
    pContext->setAlphaTestRef(0.5f);
    pContext->setAlphaTestEnable(isAlphaTest);
    pContext->setColorMask(0xf);
    pContext->setBlendEnableMask(0x1);
    pContext->setDepthEnable(true, false);
    pContext->setBlendFactorSrcRGB(0, NVN_BLEND_FUNC_DST_COLOR);
    pContext->setBlendFactorDstRGB(0, NVN_BLEND_FUNC_ZERO);
    pContext->setBlendEquationRGB(0, NVN_BLEND_EQUATION_ADD);
}

/**
 * Sets up a multiple render target context for multiplying into the base color buffer.
 * @param pContext Graphics context to modify.
 * @param isAlphaTest Whether to enable alpha testing.
 */
void setContextMRTMulBc(sead::GraphicsContextMRT* pContext, bool isAlphaTest) {
    pContext->setBlendEnableMask(0x8);
    pContext->setColorMask(0x7000);
    pContext->setAlphaTestEnable(isAlphaTest);
    pContext->setDepthEnable(true, false);
    pContext->setBlendFactorSrcRGB(3, NVN_BLEND_FUNC_DST_COLOR);
    pContext->setBlendFactorDstRGB(3, NVN_BLEND_FUNC_ZERO);
    pContext->setBlendEquationRGB(3, NVN_BLEND_EQUATION_ADD);
}

/**
 * Sets up a multiple render target context for drawing foot prints.
 * @param pContext Graphics context to modify.
 */
void setContextMRTFootPrint(sead::GraphicsContextMRT* pContext) {
    pContext->setBlendEnableMask(0xb);
    pContext->setAlphaTestEnable(false);
    pContext->setDepthEnable(true, false);
    pContext->setColorMask(0x703f);
    pContext->setBlendFactorSrcRGB(3, NVN_BLEND_FUNC_ZERO);
    pContext->setBlendFactorDstRGB(3, NVN_BLEND_FUNC_ONE_MINUS_SRC_COLOR);
    pContext->setBlendEquationRGB(3, NVN_BLEND_EQUATION_ADD);
    pContext->setBlendFactorSrcRGB(0, NVN_BLEND_FUNC_ZERO);
    pContext->setBlendFactorDstRGB(0, NVN_BLEND_FUNC_ONE_MINUS_SRC_ALPHA);
    pContext->setBlendEquationRGB(0, NVN_BLEND_EQUATION_ADD);
    pContext->setBlendFactorSrcRGB(1, NVN_BLEND_FUNC_SRC_ALPHA);
    pContext->setBlendFactorDstRGB(1, NVN_BLEND_FUNC_ONE_MINUS_SRC_ALPHA);
    pContext->setBlendEquationRGB(1, NVN_BLEND_EQUATION_ADD);
}

/**
 * Sets up a multiple render target context for alpha masked opaque drawing.
 * @param pContext Graphics context to modify.
 */
void setContextMRTAlphaMask(sead::GraphicsContextMRT* pContext) {
    setDepthFuncNearDraw(pContext);
    pContext->setAlphaTestEnable(true);
    pContext->setBlendEnableMask(0);
    pContext->setColorMask(0xfffff);
}

/**
 * Sets up a multiple render target context for drawing translucent Mii faces.
 * @param pContext Graphics context to modify.
 */
void setContextMRTMiiFaceXlu(sead::GraphicsContextMRT* pContext) {
    pContext->setBlendEnableMask(0x9);
    pContext->setColorMask(0xf7fff);
    pContext->setDepthEnable(true, false);
    pContext->setBlendFactorSrcA(3, NVN_BLEND_FUNC_ZERO);
    pContext->setBlendFactorDstA(3, NVN_BLEND_FUNC_ZERO);
    pContext->setBlendEquationA(3, NVN_BLEND_EQUATION_ADD);
    pContext->setBlendFactorSrcRGB(0, NVN_BLEND_FUNC_SRC_ALPHA);
    pContext->setBlendFactorDstRGB(0, NVN_BLEND_FUNC_ONE_MINUS_SRC_ALPHA);
    pContext->setBlendEquationRGB(0, NVN_BLEND_EQUATION_ADD);
}

/**
 * Sets up a multiple render target context that only writes depth.
 * @param pContext Graphics context to modify.
 */
void setContextMRTOnlyDepth(sead::GraphicsContextMRT* pContext) {
    pContext->setDepthEnable(true, true);
    setDepthFuncNearDraw(pContext);
    pContext->setBlendEnableMask(0);
    pContext->setColorMask(0);
}

/**
 * Sets up a multiple render target context for drawing silhouettes.
 * @param pContext Graphics context to modify.
 */
void setContextMRTSilhouette(sead::GraphicsContextMRT* pContext) {
    pContext->setBlendEnableMask(0x9);
    pContext->setColorMask(0x7007);
    pContext->setDepthEnable(true, false);
    setDepthFuncFarDraw(pContext);
    pContext->setBlendFactorSrcRGB(3, NVN_BLEND_FUNC_ONE);
    pContext->setBlendFactorDstRGB(3, NVN_BLEND_FUNC_ZERO);
    pContext->setBlendEquationRGB(3, NVN_BLEND_EQUATION_ADD);
    pContext->setBlendFactorSrcRGB(0, NVN_BLEND_FUNC_ONE);
    pContext->setBlendFactorDstRGB(0, NVN_BLEND_FUNC_ZERO);
    pContext->setBlendEquationRGB(0, NVN_BLEND_EQUATION_ADD);
    pContext->setCullingMode(NVN_FACE_BACK);
}

}  // namespace al
