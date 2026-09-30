#include "Project/AreaObj/AreaShape.hpp"

#include "Library/Math/MathUtil.hpp"
#include "Library/Math/MatrixUtil.hpp"

namespace al {
/**
 * Constructs a shape with unit scale and no base matrix.
 */
AreaShape::AreaShape() = default;

/**
 * Sets the matrix the shape is placed with.
 * @param pMtx base matrix
 */
void AreaShape::setBaseMtxPtr(const sead::Matrix34f* pMtx) {
    mBaseMtx = pMtx;
}

/**
 * Sets the scale of the shape.
 * @param rScale scale
 */
void AreaShape::setScale(const sead::Vector3f& rScale) {
    mScale = rScale;
}

/**
 * Transforms a world position into unscaled shape space.
 * @param pLocalPos output local position
 * @param rPos world position
 * @return false if the scale is zero on an axis
 */
bool AreaShape::calcLocalPos(sead::Vector3f* pLocalPos, const sead::Vector3f& rPos) const {
    if (isNearZero(mScale.x, 0.001f) || isNearZero(mScale.y, 0.001f) ||
        isNearZero(mScale.z, 0.001f)) {
        return false;
    }

    if (mBaseMtx) {
        calcMtxLocalTrans(pLocalPos, *mBaseMtx, rPos);
    } else {
        pLocalPos->set(rPos);
    }
    pLocalPos->x = pLocalPos->x / mScale.x;
    pLocalPos->y = pLocalPos->y / mScale.y;
    pLocalPos->z = pLocalPos->z / mScale.z;
    return true;
}

/**
 * Transforms an unscaled shape space position into world space.
 * @param pWorldPos output world position
 * @param rPos local position
 * @return false if the scale is zero on an axis
 */
bool AreaShape::calcWorldPos(sead::Vector3f* pWorldPos, const sead::Vector3f& rPos) const {
    if (isNearZero(mScale.x, 0.001f) || isNearZero(mScale.y, 0.001f) ||
        isNearZero(mScale.z, 0.001f)) {
        return false;
    }

    pWorldPos->x = rPos.x * mScale.x;
    pWorldPos->y = rPos.y * mScale.y;
    pWorldPos->z = rPos.z * mScale.z;
    if (mBaseMtx) {
        pWorldPos->setMul(*mBaseMtx, *pWorldPos);
    }
    return true;
}

/**
 * Transforms an unscaled shape space direction into a normalized world direction.
 * @param pWorldDir output world direction
 * @param rDir local direction
 * @return false if the scale is zero on an axis
 */
bool AreaShape::calcWorldDir(sead::Vector3f* pWorldDir, const sead::Vector3f& rDir) const {
    if (isNearZero(mScale.x, 0.001f) || isNearZero(mScale.y, 0.001f) ||
        isNearZero(mScale.z, 0.001f)) {
        return false;
    }

    pWorldDir->x = rDir.x * mScale.x;
    pWorldDir->y = rDir.y * mScale.y;
    pWorldDir->z = rDir.z * mScale.z;
    if (mBaseMtx) {
        pWorldDir->setRotated(*mBaseMtx, *pWorldDir);
    }
    normalizeOrZero(pWorldDir);
    return true;
}

/**
 * Gets the translation of the shape.
 * @param pTrans output translation
 */
void AreaShape::calcTrans(sead::Vector3f* pTrans) const {
    if (mBaseMtx) {
        mBaseMtx->getTranslation(*pTrans);
    } else {
        pTrans->set(sead::Vector3f::zero);
    }
}
}  // namespace al
