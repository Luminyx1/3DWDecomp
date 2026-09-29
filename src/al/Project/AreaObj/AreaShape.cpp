#include "Project/AreaObj/AreaShape.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Project/Matrix/MatrixUtil.hpp"

namespace al {
    /** @brief Constructs a shape with unit scale and no base matrix. */
    AreaShape::AreaShape() : mBaseMtx(nullptr), mScale(1.0f, 1.0f, 1.0f) {}

    /**
     * @brief Sets the matrix the shape is placed with.
     * @param pBaseMtx The base matrix.
     */
    void AreaShape::setBaseMtxPtr(const sead::Matrix34f* pBaseMtx) {
        mBaseMtx = pBaseMtx;
    }

    /**
     * @brief Sets the scale of the shape.
     * @param rScale The scale.
     */
    void AreaShape::setScale(const sead::Vector3f& rScale) {
        mScale = rScale;
    }

    /**
     * @brief Transforms a world position into the shape's unscaled local space.
     * @param pLocalPos Receives the local position.
     * @param rWorldPos The world position.
     * @return Whether the scale is valid.
     */
    bool AreaShape::calcLocalPos(sead::Vector3f* pLocalPos, const sead::Vector3f& rWorldPos) const {
        if (isNearZero(mScale.x, 0.001f) || isNearZero(mScale.y, 0.001f) || isNearZero(mScale.z, 0.001f)) {
            return false;
        }

        if (mBaseMtx != nullptr) {
            calcMtxLocalTrans(pLocalPos, *mBaseMtx, rWorldPos);
        } else {
            pLocalPos->set(rWorldPos);
        }

        pLocalPos->x = pLocalPos->x / mScale.x;
        pLocalPos->y = pLocalPos->y / mScale.y;
        pLocalPos->z = pLocalPos->z / mScale.z;

        return true;
    }

    /**
     * @brief Transforms a position from the shape's unscaled local space into world space.
     * @param pWorldPos Receives the world position.
     * @param rLocalPos The local position.
     * @return Whether the scale is valid.
     */
    bool AreaShape::calcWorldPos(sead::Vector3f* pWorldPos, const sead::Vector3f& rLocalPos) const {
        if (isNearZero(mScale.x, 0.001f) || isNearZero(mScale.y, 0.001f) || isNearZero(mScale.z, 0.001f)) {
            return false;
        }

        pWorldPos->x = rLocalPos.x * mScale.x;
        pWorldPos->y = rLocalPos.y * mScale.y;
        pWorldPos->z = rLocalPos.z * mScale.z;

        if (mBaseMtx != nullptr) {
            pWorldPos->setMul(*mBaseMtx, *pWorldPos);
        }

        return true;
    }

    /**
     * @brief Transforms a direction from the shape's unscaled local space into a normalized world direction.
     * @param pWorldDir Receives the world direction.
     * @param rLocalDir The local direction.
     * @return Whether the scale is valid.
     */
    bool AreaShape::calcWorldDir(sead::Vector3f* pWorldDir, const sead::Vector3f& rLocalDir) const {
        if (isNearZero(mScale.x, 0.001f) || isNearZero(mScale.y, 0.001f) || isNearZero(mScale.z, 0.001f)) {
            return false;
        }

        pWorldDir->x = rLocalDir.x * mScale.x;
        pWorldDir->y = rLocalDir.y * mScale.y;
        pWorldDir->z = rLocalDir.z * mScale.z;

        if (mBaseMtx != nullptr) {
            pWorldDir->setRotated(*mBaseMtx, *pWorldDir);
        }

        normalizeOrZero(pWorldDir);
        return true;
    }

    /**
     * @brief Gets the position of the shape.
     * @param pTrans Receives the position.
     */
    void AreaShape::calcTrans(sead::Vector3f* pTrans) const {
        if (mBaseMtx != nullptr) {
            mBaseMtx->getTranslation(*pTrans);
        } else {
            pTrans->set(sead::Vector3f::zero);
        }
    }
};
