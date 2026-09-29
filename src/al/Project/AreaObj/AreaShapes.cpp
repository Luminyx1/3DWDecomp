#include "Project/AreaObj/AreaShapeCube.hpp"
#include "Project/AreaObj/AreaShapeRound.hpp"
#include <nerd/nerdMath.h>
#include "Library/Math/MathUtil.hpp"

namespace al {
    /**
     * @brief Constructs a cube shape.
     * @param originType Whether the origin is at the center or the bottom of the cube.
     */
    AreaShapeCube::AreaShapeCube(OriginType originType) : mOriginType(originType) {}

    /**
     * @brief Calculates the bounding box of the cube in local space.
     * @param pBox Receives the bounding box.
     * @return Whether the bounding box could be calculated.
     */
    bool AreaShapeCube::calcLocalBoundingBox(sead::BoundBox3f* pBox) const {
        pBox->setUndef();
        sead::Vector3f min(-500.0f, calcBottom(), -500.0f);
        sead::Vector3f max(500.0f, calcTop(), 500.0f);
        pBox->addPoint(min);
        pBox->addPoint(max);
        return true;
    }

    /**
     * @brief Calculates the bounding box of the cube in world space.
     * @param pBox Receives the bounding box.
     * @return Whether the bounding box could be calculated.
     */
    bool AreaShapeCube::calcWorldBoundingBox(sead::BoundBox3f* pBox) const {
        pBox->setUndef();
        return true;
    }

    /**
     * @brief Checks whether a position is inside the cube.
     * @param rPos The position to check.
     * @return Whether the cube contains the position.
     */
    bool AreaShapeCube::isInVolume(const sead::Vector3f& rPos) const {
        sead::Vector3f localPos = sead::Vector3f::zero;
        calcLocalPos(&localPos, rPos);
        return isInLocalVolume(localPos);
    }

    /**
     * @brief Calculates the nearest point of the shape (unused for cubes).
     * @param pOut Receives the nearest point.
     * @param rPos The position.
     */
    void AreaShapeCube::calcNearPoint(sead::Vector3f* pOut, const sead::Vector3f& rPos) const {}

    /**
     * @brief Calculates the point on the cube's surface nearest to a position.
     * @param pOut Receives the nearest surface point.
     * @param rPos The position.
     * @return Whether a nearest point could be calculated.
     */
    bool AreaShapeCube::calcNearestEdgePoint(sead::Vector3f* pOut, const sead::Vector3f& rPos) const {
        f32 bottom = calcBottom();
        f32 top = calcTop();

        sead::Vector3f localPos(0.0f, 0.0f, 0.0f);
        calcLocalPos(&localPos, rPos);

        if (isInLocalVolume(localPos)) {
            if (isNearZero(localPos, 0.001f)) {
                return false;
            }

            f32 rateX = calcRate01(localPos.x, -500.0f, 500.0f) - 0.5f;
            f32 rateY = calcRate01(localPos.y, bottom, top) - 0.5f;
            f32 rateZ = calcRate01(localPos.z, -500.0f, 500.0f) - 0.5f;

            f32 absX = sead::Mathf::abs(rateX);
            f32 absY = sead::Mathf::abs(rateY);
            f32 absZ = sead::Mathf::abs(rateZ);

            s32 axis;
            if (absX > absY) {
                axis = absX > absZ ? 0 : 2;
            } else {
                axis = absY > absZ ? 1 : 2;
            }

            switch (axis) {
            case 0:
                localPos.x = rateX < 0.0f ? -500.0f : 500.0f;
                break;
            case 1:
                localPos.y = rateY < 0.0f ? bottom : top;
                break;
            case 2:
                localPos.z = rateZ < 0.0f ? -500.0f : 500.0f;
                break;
            }
        } else {
            if (sead::Mathf::abs(localPos.x) >= 500.0f) {
                localPos.x = localPos.x < 0.0f ? -500.0f : 500.0f;
            }

            if (localPos.y <= bottom) {
                localPos.y = bottom;
            } else if (localPos.y >= top) {
                localPos.y = top;
            }

            if (sead::Mathf::abs(localPos.z) >= 500.0f) {
                localPos.z = localPos.z < 0.0f ? -500.0f : 500.0f;
            }
        }

        calcWorldPos(pOut, localPos);
        return true;
    }

    /**
     * @brief Checks whether a position in local space is inside the cube.
     * @param rLocalPos The local position to check.
     * @return Whether the cube contains the position.
     */
    bool AreaShapeCube::isInLocalVolume(const sead::Vector3f& rLocalPos) const {
        f32 bottom = calcBottom();
        f32 top = calcTop();
        if (rLocalPos.y < bottom || top < rLocalPos.y) {
            return false;
        }

        if (rLocalPos.x < -500.0f || 500.0f < rLocalPos.x) {
            return false;
        }

        if (rLocalPos.z < -500.0f || 500.0f < rLocalPos.z) {
            return false;
        }

        return true;
    }

    /**
     * @brief Checks whether a line segment hits the cube.
     * @param pHitPos Receives the hit position.
     * @param pHitNormal Receives the normal at the hit position.
     * @param rStart The start of the segment.
     * @param rEnd The end of the segment.
     * @return Whether the segment hits the cube.
     */
    bool AreaShapeCube::checkArrowCollision(sead::Vector3f* pHitPos, sead::Vector3f* pHitNormal, const sead::Vector3f& rStart,
                                            const sead::Vector3f& rEnd) const {
        f32 bottom = calcBottom();
        f32 top = calcTop();

        sead::Vector3f localStart = sead::Vector3f::zero;
        calcLocalPos(&localStart, rStart);
        sead::Vector3f localEnd = sead::Vector3f::zero;
        calcLocalPos(&localEnd, rEnd);

        sead::Vector3f dir = localEnd - localStart;
        sead::Vector3f hitPos;

        if (isInLocalVolume(localStart)) {
            if (dir.y > 0.0f) {
                f32 t = (top - localStart.y) / dir.y;
                if (0.0f <= t && t <= 1.0f) {
                    hitPos = localStart + dir * t;
                    if (-500.0f <= hitPos.x && hitPos.x <= 500.0f && -500.0f <= hitPos.z && hitPos.z <= 500.0f) {
                        calcWorldPos(pHitPos, hitPos);
                        if (pHitNormal != nullptr) {
                            calcWorldDir(pHitNormal, sead::Vector3f::ey);
                        }
                        return true;
                    }
                }
            } else if (dir.y < 0.0f) {
                f32 t = (bottom - localStart.y) / dir.y;
                if (0.0f <= t && t <= 1.0f) {
                    hitPos = localStart + dir * t;
                    if (-500.0f <= hitPos.x && hitPos.x <= 500.0f && -500.0f <= hitPos.z && hitPos.z <= 500.0f) {
                        calcWorldPos(pHitPos, hitPos);
                        if (pHitNormal != nullptr) {
                            calcWorldDir(pHitNormal, -sead::Vector3f::ey);
                        }
                        return true;
                    }
                }
            }

            if (dir.z > 0.0f) {
                f32 t = (500.0f - localStart.z) / dir.z;
                if (0.0f <= t && t <= 1.0f) {
                    hitPos = localStart + dir * t;
                    if (-500.0f <= hitPos.x && hitPos.x <= 500.0f && hitPos.y >= bottom && hitPos.y <= top) {
                        calcWorldPos(pHitPos, hitPos);
                        if (pHitNormal != nullptr) {
                            calcWorldDir(pHitNormal, sead::Vector3f::ez);
                        }
                        return true;
                    }
                }
            } else if (dir.z < 0.0f) {
                f32 t = (-500.0f - localStart.z) / dir.z;
                if (0.0f <= t && t <= 1.0f) {
                    hitPos = localStart + dir * t;
                    if (-500.0f <= hitPos.x && hitPos.x <= 500.0f && hitPos.y >= bottom && hitPos.y <= top) {
                        calcWorldPos(pHitPos, hitPos);
                        if (pHitNormal != nullptr) {
                            calcWorldDir(pHitNormal, -sead::Vector3f::ez);
                        }
                        return true;
                    }
                }
            }

            if (dir.x > 0.0f) {
                f32 t = (500.0f - localStart.x) / dir.x;
                if (0.0f <= t && t <= 1.0f) {
                    hitPos = localStart + dir * t;
                    if (-500.0f <= hitPos.z && hitPos.z <= 500.0f && hitPos.y >= bottom && hitPos.y <= top) {
                        calcWorldPos(pHitPos, hitPos);
                        if (pHitNormal != nullptr) {
                            calcWorldDir(pHitNormal, sead::Vector3f::ex);
                        }
                        return true;
                    }
                }
            } else if (dir.x < 0.0f) {
                f32 t = (-500.0f - localStart.x) / dir.x;
                if (0.0f <= t && t <= 1.0f) {
                    hitPos = localStart + dir * t;
                    if (-500.0f <= hitPos.z && hitPos.z <= 500.0f && hitPos.y >= bottom && hitPos.y <= top) {
                        calcWorldPos(pHitPos, hitPos);
                        if (pHitNormal != nullptr) {
                            calcWorldDir(pHitNormal, sead::Vector3f::ey);
                        }
                        return true;
                    }
                }
            }

            return false;
        }

        if (dir.y > 0.0f) {
            f32 t = (bottom - localStart.y) / dir.y;
            if (0.0f <= t && t <= 1.0f) {
                hitPos = localStart + dir * t;
                if (-500.0f <= hitPos.x && hitPos.x <= 500.0f && -500.0f <= hitPos.z && hitPos.z <= 500.0f) {
                    calcWorldPos(pHitPos, hitPos);
                    if (pHitNormal != nullptr) {
                        calcWorldDir(pHitNormal, -sead::Vector3f::ey);
                    }
                    return true;
                }
            }
        } else if (dir.y < 0.0f) {
            f32 t = (top - localStart.y) / dir.y;
            if (0.0f <= t && t <= 1.0f) {
                hitPos = localStart + dir * t;
                if (-500.0f <= hitPos.x && hitPos.x <= 500.0f && -500.0f <= hitPos.z && hitPos.z <= 500.0f) {
                    calcWorldPos(pHitPos, hitPos);
                    if (pHitNormal != nullptr) {
                        calcWorldDir(pHitNormal, sead::Vector3f::ey);
                    }
                    return true;
                }
            }
        }

        if (dir.z > 0.0f) {
            f32 t = (-500.0f - localStart.z) / dir.z;
            if (0.0f <= t && t <= 1.0f) {
                hitPos = localStart + dir * t;
                if (-500.0f <= hitPos.x && hitPos.x <= 500.0f && hitPos.y >= bottom && hitPos.y <= top) {
                    calcWorldPos(pHitPos, hitPos);
                    if (pHitNormal != nullptr) {
                        calcWorldDir(pHitNormal, -sead::Vector3f::ez);
                    }
                    return true;
                }
            }
        } else if (dir.z < 0.0f) {
            f32 t = (500.0f - localStart.z) / dir.z;
            if (0.0f <= t && t <= 1.0f) {
                hitPos = localStart + dir * t;
                if (-500.0f <= hitPos.x && hitPos.x <= 500.0f && hitPos.y >= bottom && hitPos.y <= top) {
                    calcWorldPos(pHitPos, hitPos);
                    if (pHitNormal != nullptr) {
                        calcWorldDir(pHitNormal, sead::Vector3f::ez);
                    }
                    return true;
                }
            }
        }

        if (dir.x > 0.0f) {
            f32 t = (-500.0f - localStart.x) / dir.x;
            if (0.0f <= t && t <= 1.0f) {
                hitPos = localStart + dir * t;
                if (-500.0f <= hitPos.z && hitPos.z <= 500.0f && hitPos.y >= bottom && hitPos.y <= top) {
                    calcWorldPos(pHitPos, hitPos);
                    if (pHitNormal != nullptr) {
                        calcWorldDir(pHitNormal, -sead::Vector3f::ex);
                    }
                    return true;
                }
            }
        } else if (dir.x < 0.0f) {
            f32 t = (500.0f - localStart.x) / dir.x;
            if (0.0f <= t && t <= 1.0f) {
                hitPos = localStart + dir * t;
                if (-500.0f <= hitPos.z && hitPos.z <= 500.0f && hitPos.y >= bottom && hitPos.y <= top) {
                    calcWorldPos(pHitPos, hitPos);
                    if (pHitNormal != nullptr) {
                        calcWorldDir(pHitNormal, -sead::Vector3f::ex);
                    }
                    return true;
                }
            }
        }

        return false;
    }

    /** @brief Constructs a sphere shape. */
    AreaShapeSphere::AreaShapeSphere() {}

    /**
     * @brief Checks whether a position is inside the sphere.
     * @param rPos The position to check.
     * @return Whether the sphere contains the position.
     */
    bool AreaShapeSphere::isInVolume(const sead::Vector3f& rPos) const {
        sead::Vector3f trans;
        calcTrans(&trans);
        sead::Vector3f diff = rPos - trans;
        f32 radius = mScale.x * 500.0f;
        return diff.squaredLength() <= radius * radius;
    }

    /**
     * @brief Calculates the nearest point of the shape (unused for spheres).
     * @param pOut Receives the nearest point.
     * @param rPos The position.
     */
    void AreaShapeSphere::calcNearPoint(sead::Vector3f* pOut, const sead::Vector3f& rPos) const {}

    /**
     * @brief Calculates the point on the sphere's surface nearest to a position.
     * @param pOut Receives the nearest surface point.
     * @param rPos The position.
     * @return Whether a nearest point could be calculated.
     */
    bool AreaShapeSphere::calcNearestEdgePoint(sead::Vector3f* pOut, const sead::Vector3f& rPos) const {
        sead::Vector3f localPos = sead::Vector3f::zero;
        calcLocalPos(&localPos, rPos);

        f32 length = nerd::sqrt(localPos.squaredLength());
        if (length > 0.0f) {
            localPos *= 500.0f / length;
        }

        calcWorldPos(pOut, localPos);
        return true;
    }

    /**
     * @brief Checks whether a line segment hits the sphere (unsupported).
     * @param pHitPos Receives the hit position.
     * @param pHitNormal Receives the normal at the hit position.
     * @param rStart The start of the segment.
     * @param rEnd The end of the segment.
     * @return Always false.
     */
    bool AreaShapeSphere::checkArrowCollision(sead::Vector3f* pHitPos, sead::Vector3f* pHitNormal, const sead::Vector3f& rStart,
                                              const sead::Vector3f& rEnd) const {
        return false;
    }

    /**
     * @brief Calculates the bounding box of the sphere in world space (unsupported).
     * @param pBox Receives the bounding box.
     * @return Always false.
     */
    bool AreaShapeSphere::calcWorldBoundingBox(sead::BoundBox3f* pBox) const {
        return false;
    }

    /** @brief Constructs an oval shape. */
    AreaShapeOval::AreaShapeOval() {}

    /**
     * @brief Checks whether a position is inside the oval.
     * @param rPos The position to check.
     * @return Whether the oval contains the position.
     */
    bool AreaShapeOval::isInVolume(const sead::Vector3f& rPos) const {
        sead::Vector3f localPos = sead::Vector3f::zero;
        calcLocalPos(&localPos, rPos);
        return localPos.squaredLength() <= 500.0f * 500.0f;
    }

    /**
     * @brief Calculates the nearest point of the shape (unused for ovals).
     * @param pOut Receives the nearest point.
     * @param rPos The position.
     */
    void AreaShapeOval::calcNearPoint(sead::Vector3f* pOut, const sead::Vector3f& rPos) const {}

    /**
     * @brief Calculates the point on the oval's surface nearest to a position.
     * @param pOut Receives the nearest surface point.
     * @param rPos The position.
     * @return Whether a nearest point could be calculated.
     */
    bool AreaShapeOval::calcNearestEdgePoint(sead::Vector3f* pOut, const sead::Vector3f& rPos) const {
        sead::Vector3f localPos = sead::Vector3f::zero;
        calcLocalPos(&localPos, rPos);

        f32 length = nerd::sqrt(localPos.squaredLength());
        if (length > 0.0f) {
            localPos *= 500.0f / length;
        }

        calcWorldPos(pOut, localPos);
        return true;
    }

    /**
     * @brief Checks whether a line segment hits the oval (unsupported).
     * @param pHitPos Receives the hit position.
     * @param pHitNormal Receives the normal at the hit position.
     * @param rStart The start of the segment.
     * @param rEnd The end of the segment.
     * @return Always false.
     */
    bool AreaShapeOval::checkArrowCollision(sead::Vector3f* pHitPos, sead::Vector3f* pHitNormal, const sead::Vector3f& rStart,
                                            const sead::Vector3f& rEnd) const {
        return false;
    }

    /**
     * @brief Calculates the bounding box of the oval in world space (unsupported).
     * @param pBox Receives the bounding box.
     * @return Always false.
     */
    bool AreaShapeOval::calcWorldBoundingBox(sead::BoundBox3f* pBox) const {
        return false;
    }

    /** @brief Constructs a cylinder shape. */
    AreaShapeCylinder::AreaShapeCylinder() {}

    /**
     * @brief Checks whether a position is inside the cylinder.
     * @param rPos The position to check.
     * @return Whether the cylinder contains the position.
     */
    bool AreaShapeCylinder::isInVolume(const sead::Vector3f& rPos) const {
        sead::Vector3f localPos = sead::Vector3f::zero;
        calcLocalPos(&localPos, rPos);

        if (localPos.y < 0.0f || 500.0f < localPos.y) {
            return false;
        }

        return localPos.x * localPos.x + localPos.z * localPos.z <= 500.0f * 500.0f;
    }

    /**
     * @brief Calculates the point on or in the cylinder nearest to a position.
     * @param pOut Receives the nearest point.
     * @param rPos The position.
     */
    void AreaShapeCylinder::calcNearPoint(sead::Vector3f* pOut, const sead::Vector3f& rPos) const {
        sead::Vector3f localPos = sead::Vector3f::zero;
        calcLocalPos(&localPos, rPos);

        sead::Vector3f nearPos = sead::Vector3f::zero;
        nearPos.y = sead::Mathf::clamp(localPos.y, 0.0f, 500.0f);

        f32 lengthSq = localPos.x * localPos.x + localPos.z * localPos.z;
        if (lengthSq <= 500.0f * 500.0f) {
            nearPos.x = localPos.x;
            nearPos.z = localPos.z;
        } else {
            f32 rate = 500.0f / nerd::sqrt(lengthSq);
            nearPos.x = localPos.x * rate;
            nearPos.z = rate * localPos.z;
        }

        calcWorldPos(pOut, nearPos);
    }

    /**
     * @brief Calculates the point on the cylinder nearest to a position.
     * @param pOut Receives the nearest point.
     * @param rPos The position.
     * @return Whether a nearest point could be calculated.
     */
    bool AreaShapeCylinder::calcNearestEdgePoint(sead::Vector3f* pOut, const sead::Vector3f& rPos) const {
        sead::Vector3f localPos = sead::Vector3f::zero;
        calcLocalPos(&localPos, rPos);

        sead::Vector3f nearPos = sead::Vector3f::zero;
        nearPos.y = sead::Mathf::clamp(localPos.y, 0.0f, 500.0f);

        f32 lengthSq = localPos.x * localPos.x + localPos.z * localPos.z;
        if (lengthSq <= 500.0f * 500.0f) {
            nearPos.x = localPos.x;
            nearPos.z = localPos.z;
        } else {
            f32 rate = 500.0f / nerd::sqrt(lengthSq);
            nearPos.x = localPos.x * rate;
            nearPos.z = rate * localPos.z;
        }

        calcWorldPos(pOut, nearPos);
        return true;
    }

    /**
     * @brief Calculates the bounding box of the cylinder in world space (unsupported).
     * @param pBox Receives the bounding box.
     * @return Always false.
     */
    bool AreaShapeCylinder::calcWorldBoundingBox(sead::BoundBox3f* pBox) const {
        return false;
    }

    /**
     * @brief Checks whether a line segment hits the cylinder (unsupported).
     * @param pHitPos Receives the hit position.
     * @param pHitNormal Receives the normal at the hit position.
     * @param rStart The start of the segment.
     * @param rEnd The end of the segment.
     * @return Always false.
     */
    bool AreaShapeCylinder::checkArrowCollision(sead::Vector3f* pHitPos, sead::Vector3f* pHitNormal, const sead::Vector3f& rStart,
                                                const sead::Vector3f& rEnd) const {
        return false;
    }
};
