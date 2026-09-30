#include "detail/aglSeadUtil.h"

#include <gfx/seadProjection.h>
#include <math/seadMathCalcCommon.h>

namespace agl::detail {

/**
 * Gets the clip planes, aspect ratio, vertical field of view and offset of a projection.
 * @param rProjection projection
 * @param pNear near clip distance output, may be nullptr
 * @param pFar far clip distance output, may be nullptr
 * @param pAspect aspect ratio output, may be nullptr
 * @param pFovy vertical field of view output, may be nullptr
 * @param pOffset offset output, may be nullptr
 * @return whether the projection is a perspective or frustum projection
 */
bool SeadUtil::getNearFarAspectFovy(const sead::Projection& rProjection, f32* pNear, f32* pFar,
                                    f32* pAspect, f32* pFovy, sead::Vector2f* pOffset)
{
    if (sead::IsDerivedFrom<sead::PerspectiveProjection>(&rProjection))
    {
        const auto* pPerspective = sead::DynamicCast<const sead::PerspectiveProjection>(&rProjection);

        if (pNear)
        {
            *pNear = pPerspective->getNear();
        }

        if (pFar)
        {
            *pFar = pPerspective->getFar();
        }

        if (pAspect)
        {
            *pAspect = pPerspective->getAspect();
        }

        if (pFovy)
        {
            *pFovy = pPerspective->getFovy();
        }

        if (pOffset)
        {
            *pOffset = pPerspective->getOffsetDirect();
        }

        return true;
    }

    if (sead::IsDerivedFrom<sead::FrustumProjection>(&rProjection))
    {
        const auto* pFrustum = sead::DynamicCast<const sead::FrustumProjection>(&rProjection);

        if (pNear)
        {
            *pNear = pFrustum->getNear();
        }

        if (pFar)
        {
            *pFar = pFrustum->getFar();
        }

        if (pAspect)
        {
            *pAspect = pFrustum->getAspect();
        }

        if (pFovy)
        {
            *pFovy = pFrustum->getFovy();
        }

        if (pOffset)
        {
            pFrustum->getOffset(pOffset);
        }

        return true;
    }

    if (sead::IsDerivedFrom<sead::OrthoProjection>(&rProjection))
    {
        const auto* pOrtho = sead::DynamicCast<const sead::OrthoProjection>(&rProjection);

        if (pNear)
        {
            *pNear = pOrtho->getNear();
        }

        if (pFar)
        {
            *pFar = pOrtho->getFar();
        }

        if (pAspect)
        {
            f32 height = sead::Mathf::abs(pOrtho->getTop() - pOrtho->getBottom());

            if (height > 0.0f)
            {
                f32 width = sead::Mathf::abs(pOrtho->getLeft() - pOrtho->getRight());
                *pAspect = sead::Mathf::abs(width / height);
            }
            else
            {
                *pAspect = 1.0f;
            }
        }

        if (pFovy)
        {
            *pFovy = 0.0f;
        }

        if (pOffset)
        {
            *pOffset = sead::Vector2f::zero;
        }

        return false;
    }

    if (pNear)
    {
        *pNear = 0.0f;
    }

    if (pFar)
    {
        *pFar = 0.0f;
    }

    if (pAspect)
    {
        *pAspect = 1.0f;
    }

    if (pFovy)
    {
        *pFovy = 0.0f;
    }

    if (pOffset)
    {
        *pOffset = sead::Vector2f::zero;
    }

    return false;
}

/**
 * Sets the clip planes, aspect ratio, vertical field of view and offset of a projection.
 * @param pProjection projection
 * @param near near clip distance
 * @param far far clip distance
 * @param aspect aspect ratio
 * @param fovy vertical field of view
 * @param rOffset offset
 * @param keepWidth whether a frustum or ortho projection keeps its width when changing the aspect
 * @return whether the projection is a perspective or frustum projection
 */
bool SeadUtil::setNearFarAspectFovy(sead::Projection* pProjection, f32 near, f32 far, f32 aspect,
                                    f32 fovy, const sead::Vector2f& rOffset, bool keepWidth)
{
    if (sead::IsDerivedFrom<sead::PerspectiveProjection>(pProjection))
    {
        auto* pPerspective = sead::DynamicCast<sead::PerspectiveProjection>(pProjection);
        pPerspective->setNear(near);
        pPerspective->setFar(far);
        pPerspective->setAspect(aspect);
        pPerspective->setFovy(fovy);
        pPerspective->setOffset(rOffset);
        return true;
    }

    if (sead::IsDerivedFrom<sead::FrustumProjection>(pProjection))
    {
        auto* pFrustum = sead::DynamicCast<sead::FrustumProjection>(pProjection);
        f32 width = sead::Mathf::abs(pFrustum->getLeft() - pFrustum->getRight());
        f32 height = sead::Mathf::abs(pFrustum->getTop() - pFrustum->getBottom());
        pFrustum->setNear(near);
        pFrustum->setFar(far);

        if (keepWidth)
        {
            f32 scale = width / aspect / height;
            pFrustum->setTop(pFrustum->getTop() * scale);
            pFrustum->setBottom(pFrustum->getBottom() * scale);
        }
        else
        {
            f32 scale = height * aspect / width;
            pFrustum->setLeft(pFrustum->getLeft() * scale);
            pFrustum->setRight(pFrustum->getRight() * scale);
        }

        return true;
    }

    if (sead::IsDerivedFrom<sead::OrthoProjection>(pProjection))
    {
        auto* pOrtho = sead::DynamicCast<sead::OrthoProjection>(pProjection);
        f32 width = sead::Mathf::abs(pOrtho->getLeft() - pOrtho->getRight());
        f32 height = sead::Mathf::abs(pOrtho->getTop() - pOrtho->getBottom());
        pOrtho->setNear(near);
        pOrtho->setFar(far);

        if (keepWidth)
        {
            f32 scale = width / aspect / height;
            pOrtho->setTop(pOrtho->getTop() * scale);
            pOrtho->setBottom(pOrtho->getBottom() * scale);
        }
        else
        {
            f32 scale = height * aspect / width;
            pOrtho->setLeft(pOrtho->getLeft() * scale);
            pOrtho->setRight(pOrtho->getRight() * scale);
        }
    }

    return false;
}

}  // namespace agl::detail
