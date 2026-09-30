#include "shadow/aglShadowUtil.h"

#include <gfx/seadCamera.h>
#include <gfx/seadGraphicsContext.h>
#include <math/seadMatrixCalcCommon.h>

#include "common/aglDrawContext.h"
#include "utility/aglDevTools.h"

namespace agl::sdw
{

/**
 * Computes the normalized view direction of a view matrix.
 * @param pDir output direction
 * @param viewMtx view matrix
 */
void ShadowUtil::calcViewDir(sead::Vector3f* pDir, sead::Matrix34f viewMtx)
{
    pDir->set(-viewMtx(2, 0), -viewMtx(2, 1), -viewMtx(2, 2));
    pDir->normalize();
}

/**
 * Computes the world position of the eye of a view matrix.
 * @param pPos output position
 * @param viewMtx view matrix
 */
void ShadowUtil::calcViewPos(sead::Vector3f* pPos, sead::Matrix34f viewMtx)
{
    pPos->set(-viewMtx(0, 0) * viewMtx(0, 3) - viewMtx(1, 0) * viewMtx(1, 3) -
                  viewMtx(2, 0) * viewMtx(2, 3),
              -viewMtx(0, 3) * viewMtx(0, 1) - viewMtx(1, 3) * viewMtx(1, 1) -
                  viewMtx(2, 3) * viewMtx(2, 1),
              -viewMtx(0, 3) * viewMtx(0, 2) - viewMtx(1, 3) * viewMtx(1, 2) -
                  viewMtx(2, 3) * viewMtx(2, 2));
}

/**
 * Builds a look-at view matrix from a position, direction and up vector.
 * @param pViewMtx output view matrix
 * @param rPos eye position
 * @param rDir view direction
 * @param rUp up vector
 */
void ShadowUtil::calcViewMatrix(sead::Matrix34f* pViewMtx, const sead::Vector3f& rPos,
                                const sead::Vector3f& rDir, const sead::Vector3f& rUp)
{
    sead::LookAtCamera camera;
    camera.setPos(rPos);
    camera.setAt(rPos + rDir);
    camera.setUp(rUp);
    camera.normalizeUp();
    camera.updateViewMatrix();
    *pViewMtx = camera.getMatrix();
}

/**
 * Extracts the near and far clip distances from a projection matrix.
 * @param pNear output near distance (may be null)
 * @param pFar output far distance (may be null)
 * @param rProjMtx projection matrix
 */
void ShadowUtil::calcNearFar(f32* pNear, f32* pFar, const sead::Matrix44f& rProjMtx)
{
    sead::Matrix44f inv;
    sead::Matrix44CalcCommon<f32>::inverse(inv, rProjMtx);
    if (pNear)
    {
        *pNear = -(inv(2, 3) - inv(2, 2)) / (inv(3, 3) - inv(3, 2));
    }

    if (pFar)
    {
        *pFar = -(inv(2, 2) + inv(2, 3)) / (inv(3, 2) + inv(3, 3));
    }
}

/**
 * Draws the wireframe of a view frustum.
 * @param pDrawContext draw context
 * @param rFrustumViewMtx view matrix of the frustum to draw
 * @param rFrustumProjMtx projection matrix of the frustum to draw
 * @param rViewMtx view matrix used for drawing
 * @param rProjMtx projection matrix used for drawing
 * @param rColor line color
 */
void ShadowUtil::drawFrustum(DrawContext* pDrawContext, const sead::Matrix34f& rFrustumViewMtx,
                             const sead::Matrix44f& rFrustumProjMtx,
                             const sead::Matrix34f& rViewMtx, const sead::Matrix44f& rProjMtx,
                             const sead::Color4f& rColor)
{
    sead::GraphicsContext context;
    context.apply(pDrawContext);
    utl::DevTools::beginDrawImm(pDrawContext, rViewMtx, rProjMtx);

    sead::Matrix44f viewProj;
    sead::Matrix44CalcCommon<f32>::multiply(viewProj, rFrustumProjMtx, rFrustumViewMtx);
    sead::Vector3f points[8] = {
        {-1.0f, 1.0f, -1.0f}, {-1.0f, -1.0f, -1.0f}, {1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, -1.0f},
        {-1.0f, 1.0f, 0.99f}, {-1.0f, -1.0f, 0.99f}, {1.0f, -1.0f, 0.99f}, {1.0f, 1.0f, 0.99f},
    };

    sead::Matrix44f inv;
    sead::Matrix44CalcCommon<f32>::inverse(inv, viewProj);

    for (s32 i = 0; i < 8; i++)
    {
        const sead::Vector3f p = points[i];
        const f32 w = inv(3, 0) * p.x + inv(3, 1) * p.y + inv(3, 2) * p.z + inv(3, 3);
        points[i].x =
            inv(0, 0) / w * p.x + inv(0, 1) / w * p.y + inv(0, 2) / w * p.z + inv(0, 3) / w;
        points[i].y =
            inv(1, 0) / w * p.x + inv(1, 1) / w * p.y + inv(1, 2) / w * p.z + inv(1, 3) / w;
        points[i].z =
            inv(2, 0) / w * p.x + inv(2, 1) / w * p.y + inv(2, 2) / w * p.z + inv(2, 3) / w;
    }

    for (s32 i = 0; i < 4; i++)
    {
        const s32 next = (i + 1) % 4;
        utl::DevTools::drawLineImm(pDrawContext, points[i], points[i + 4], rColor, 1.0f);
        utl::DevTools::drawLineImm(pDrawContext, points[i], points[next], sead::Color4f::cGreen,
                                   1.0f);
        utl::DevTools::drawLineImm(pDrawContext, points[i + 4], points[next + 4], rColor, 1.0f);
    }
}

}  // namespace agl::sdw
