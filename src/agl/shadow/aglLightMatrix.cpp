#include "shadow/aglLightMatrix.h"

#include <gfx/seadProjection.h>
#include <math/seadMathCalcCommon.h>

#include "shadow/aglShadowUtil.h"

namespace agl::sdw
{

namespace
{

inline sead::Vector3f getViewDir(const sead::Matrix34f& rViewMtx)
{
    sead::Vector3f dir;
    ShadowUtil::calcViewDir(&dir, rViewMtx);
    return dir;
}

inline sead::Vector3f getViewPos(const sead::Matrix34f& rViewMtx)
{
    sead::Vector3f pos;
    ShadowUtil::calcViewPos(&pos, rViewMtx);
    return pos;
}

}  // namespace

/**
 * Sets up a directional light looking along a direction.
 * @param rDir light direction
 */
void LightMatrix::update(const sead::Vector3f& rDir)
{
    sead::Vector3f dir = rDir;
    dir.normalize();
    sead::Vector3f up;
    up.setCross(dir, sead::Vector3f::ex);
    if (up.normalize() < sead::Mathf::epsilon() || sead::Mathf::abs(dir.dot(up)) > 0.9999999f)
    {
        up.setCross(dir, sead::Vector3f::ez);
    }

    ShadowUtil::calcViewMatrix(&mViewMtx, sead::Vector3f::zero, rDir, up);

    mProjMtx.makeIdentity();
    mProjMtx(2, 2) = -1.0f;
    mIsDirectional = true;
}

/**
 * Sets up a perspective light.
 * @param rPos light position
 * @param rDir light direction
 * @param rUp up vector
 * @param near near distance
 * @param far far distance
 * @param fovy vertical field of view in radians
 * @param aspect aspect ratio
 */
void LightMatrix::update(const sead::Vector3f& rPos, const sead::Vector3f& rDir,
                         const sead::Vector3f& rUp, f32 near, f32 far, f32 fovy, f32 aspect)
{
    ShadowUtil::calcViewMatrix(&mViewMtx, rPos, rDir, rUp);

    const f32 absNear = sead::Mathf::abs(near);
    const f32 absFar = sead::Mathf::abs(far);
    const f32 absFovy = sead::Mathf::abs(fovy);
    const f32 absAspect = sead::Mathf::abs(aspect);
    const f32 n = sead::Mathf::max(sead::Mathf::min(absNear, absFar), 0.001f);
    const f32 f = sead::Mathf::max(sead::Mathf::max(absNear, absFar), n + 0.001f);
    const f32 a = sead::Mathf::clampMin(absAspect, 0.001f);
    {
        sead::PerspectiveProjection projection(n, f,
                                               sead::Mathf::clamp(absFovy, 0.001f, 3.1405928f), a);
        mProjMtx = projection.getProjectionMatrix();
    }

    mIsDirectional = false;
}

/**
 * Sets the light matrices directly.
 * @param rViewMtx light view matrix
 * @param rProjMtx light projection matrix
 */
void LightMatrix::update(const sead::Matrix34f& rViewMtx, const sead::Matrix44f& rProjMtx)
{
    mViewMtx = rViewMtx;
    mProjMtx = rProjMtx;
    mIsDirectional = false;
}

/**
 * Computes the light view and projection matrices for a camera.
 * @param pViewMtx output light view matrix (may be null)
 * @param pProjMtx output light projection matrix (may be null)
 * @param rCameraViewMtx camera view matrix
 */
void LightMatrix::calcLightSpace(sead::Matrix34f* pViewMtx, sead::Matrix44f* pProjMtx,
                                 const sead::Matrix34f& rCameraViewMtx) const
{
    if (pViewMtx)
    {
        if (mIsDirectional)
        {
            const sead::Vector3f up = getViewDir(rCameraViewMtx);
            const sead::Vector3f dir = getViewDir(mViewMtx);
            const sead::Vector3f pos = getViewPos(mViewMtx);
            ShadowUtil::calcViewMatrix(pViewMtx, pos, dir, up);
        }
        else
        {
            *pViewMtx = mViewMtx;
        }
    }

    if (pProjMtx)
    {
        *pProjMtx = mProjMtx;
    }
}

/**
 * Computes the light view and projection matrices for an up vector.
 * @param pViewMtx output light view matrix (may be null)
 * @param pProjMtx output light projection matrix (may be null)
 * @param rUp up vector of the light view
 */
void LightMatrix::calcLightSpace(sead::Matrix34f* pViewMtx, sead::Matrix44f* pProjMtx,
                                 const sead::Vector3f& rUp) const
{
    if (pViewMtx)
    {
        if (mIsDirectional)
        {
            const sead::Vector3f dir = getViewDir(mViewMtx);
            const sead::Vector3f pos = getViewPos(mViewMtx);
            ShadowUtil::calcViewMatrix(pViewMtx, pos, dir, rUp);
        }
        else
        {
            *pViewMtx = mViewMtx;
        }
    }

    if (pProjMtx)
    {
        *pProjMtx = mProjMtx;
    }
}

/**
 * Constructs a directional light pointing down.
 */
LightMatrix::LightMatrix() : mIsDirectional(true)
{
    update(-sead::Vector3f::ey);
}

/**
 * Constructs a directional light.
 * @param rDir light direction
 */
LightMatrix::LightMatrix(const sead::Vector3f& rDir) : mIsDirectional(false)
{
    update(rDir);
}

/**
 * Constructs a perspective light.
 * @param rPos light position
 * @param rDir light direction
 * @param rUp up vector
 * @param near near distance
 * @param far far distance
 * @param fovy vertical field of view in radians
 * @param aspect aspect ratio
 */
LightMatrix::LightMatrix(const sead::Vector3f& rPos, const sead::Vector3f& rDir,
                         const sead::Vector3f& rUp, f32 near, f32 far, f32 fovy, f32 aspect)
    : mIsDirectional(false)
{
    update(rPos, rDir, rUp, near, far, fovy, aspect);
}

/**
 * Constructs a light from explicit matrices.
 * @param rViewMtx light view matrix
 * @param rProjMtx light projection matrix
 */
LightMatrix::LightMatrix(const sead::Matrix34f& rViewMtx, const sead::Matrix44f& rProjMtx)
    : mViewMtx(rViewMtx), mProjMtx(rProjMtx), mIsDirectional(false)
{
}

}  // namespace agl::sdw
