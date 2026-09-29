#pragma once

#include <gfx/seadColor.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace agl
{

class DrawContext;

namespace sdw
{

class ShadowUtil
{
public:
    static void calcViewDir(sead::Vector3f* pDir, sead::Matrix34f viewMtx);
    static void calcViewPos(sead::Vector3f* pPos, sead::Matrix34f viewMtx);
    static void calcViewMatrix(sead::Matrix34f* pViewMtx, const sead::Vector3f& rPos,
                               const sead::Vector3f& rDir, const sead::Vector3f& rUp);
    static void calcNearFar(f32* pNear, f32* pFar, const sead::Matrix44f& rProjMtx);
    static void drawFrustum(DrawContext* pDrawContext, const sead::Matrix34f& rFrustumViewMtx,
                            const sead::Matrix44f& rFrustumProjMtx, const sead::Matrix34f& rViewMtx,
                            const sead::Matrix44f& rProjMtx, const sead::Color4f& rColor);
};

}  // namespace sdw
}  // namespace agl
