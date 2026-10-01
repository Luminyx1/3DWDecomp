#include "Project/Collision/CollisionUtil.hpp"

#include <algorithm>
#include <container/seadRingBuffer.h>

#include "Library/Collision/CollisionCheckInfo.hpp"
#include "Library/Collision/CollisionDirector.hpp"
#include "Library/Collision/SphereInterpolator.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Project/Collision/CollisionParts.hpp"
#include "Project/Collision/CollisionPartsFilterBase.hpp"
#include "Project/Collision/HitDb.hpp"
#include "Project/Collision/IUseCollision.hpp"
#include "Project/Collision/TriangleFilterBase.hpp"

namespace {
class TriangleFilterIgnoreHitPrism : public al::TriangleFilterBase {
public:
    bool isInvalidTriangle(const al::Triangle& rTriangle) const override {
        if (mTriFilter != nullptr && mTriFilter->isInvalidTriangle(rTriangle)) {
            return true;
        }

        for (const al::KCPrismData* prismData : mHitPrisms) {
            if (prismData == rTriangle.mPrismData) {
                return true;
            }
        }

        return false;
    }

    void reset(const al::TriangleFilterBase* pTriFilter) {
        mHitPrisms.clear();
        mTriFilter = pTriFilter;
    }

    void addHitPrism(const al::KCPrismData* pPrismData) { mHitPrisms.pushBack(pPrismData); }

private:
    sead::FixedRingBuffer<const al::KCPrismData*, 256> mHitPrisms;
    const al::TriangleFilterBase* mTriFilter = nullptr;
};

TriangleFilterIgnoreHitPrism sHitPrismFilter;

al::CollisionDirector* getDirector(const al::IUseCollision* pCollision) {
    return pCollision->getCollisionDirector();
}

inline f32 calcSphereHitTime(const sead::Vector3f& rHitPos, const sead::Vector3f& rCenter,
                             f32 radius, const sead::Vector3f& rMoveVec) {
    sead::Vector3f diff = rHitPos - rCenter;
    f32 a = rMoveVec.dot(rMoveVec);
    f32 b = rMoveVec.dot(diff) * 2.0f;
    f32 c = diff.dot(diff) - radius * radius;
    f32 discriminant = b * b + a * -4.0f * c;
    f32 time = 0.0f;

    if (discriminant >= 0.0f) {
        f32 sqrtDiscriminant = sead::Mathf::sqrt(discriminant);
        f32 q = (b > 0.0f ? -b - sqrtDiscriminant : sqrtDiscriminant - b) * 0.5f;
        time = 1.0f / a * q;

        if (time < 0.0f) {
            time = c / (a * time);
        }
    }

    return time;
}
}  // namespace

namespace alCollisionUtil {
/**
 * Gets the active collision parts keeper.
 * @param pCollision collision user
 * @return the active collision parts keeper
 */
al::ICollisionPartsKeeper* getCollisionPartsKeeper(const al::IUseCollision* pCollision) {
    return getDirector(pCollision)->getActivePartsKeeper();
}

/**
 * Gets the hit position of a hit info.
 * @param pHitInfo hit info
 * @return the hit position
 */
const sead::Vector3f& getCollisionHitPos(const al::HitInfo* pHitInfo) {
    return pHitInfo->mPos;
}

/**
 * Gets the normal of the hit triangle.
 * @param pHitInfo hit info
 * @return the normal
 */
const sead::Vector3f* getCollisionHitNormal(const al::HitInfo* pHitInfo) {
    return pHitInfo->mTriangle.getNormal(0);
}

/**
 * Gets the sensor of the hit triangle.
 * @param pHitInfo hit info
 * @return the sensor
 */
al::HitSensor* getCollisionHitSensor(const al::HitInfo* pHitInfo) {
    return pHitInfo->mTriangle.getSensor();
}

/**
 * Gets the collision parts of the hit triangle.
 * @param pHitInfo hit info
 * @return the collision parts
 */
const al::CollisionParts* getCollisionHitParts(const al::HitInfo* pHitInfo) {
    return pHitInfo->mTriangle.mCollisionParts;
}

/**
 * Gets the actor owning the hit collision parts.
 * @param pHitInfo hit info
 * @return the actor
 */
al::LiveActor* getCollisionHitActor(const al::HitInfo* pHitInfo) {
    return pHitInfo->mTriangle.mCollisionParts->getConnectedHost();
}

/**
 * Collects the collisions of a sphere.
 * @param pCollision collision user
 * @param rPos center of the sphere
 * @param radius radius of the sphere
 * @param pPartsFilter collision parts filter
 * @param pTriFilter triangle filter
 * @return the number of hits
 */
s32 checkStrikeSphere(const al::IUseCollision* pCollision, const sead::Vector3f& rPos, f32 radius,
                      const al::CollisionPartsFilterBase* pPartsFilter,
                      const al::TriangleFilterBase* pTriFilter) {
    al::CollisionDirector* director = getDirector(pCollision);
    director->setPartsFilter(pPartsFilter);
    director->setTriFilter(pTriFilter);
    return director->checkStrikeSphere(rPos, radius, false, sead::Vector3f::zero);
}

/**
 * Collects the collisions of a moving sphere.
 * @param pCollision collision user
 * @param rPos center of the sphere
 * @param radius radius of the sphere
 * @param rMoveVec movement of the sphere
 * @param pPartsFilter collision parts filter
 * @param pTriFilter triangle filter
 * @return the number of hits
 */
s32 checkStrikeSphereMovingReaction(const al::IUseCollision* pCollision, const sead::Vector3f& rPos,
                                    f32 radius, const sead::Vector3f& rMoveVec,
                                    const al::CollisionPartsFilterBase* pPartsFilter,
                                    const al::TriangleFilterBase* pTriFilter) {
    al::CollisionDirector* director = getDirector(pCollision);
    director->setPartsFilter(pPartsFilter);
    director->setTriFilter(pTriFilter);
    return director->checkStrikeSphere(rPos, radius, true, rMoveVec);
}

/**
 * Collects the collisions of a disk.
 * @param pCollision collision user
 * @param rPos center of the disk
 * @param radius radius of the disk
 * @param height height of the disk
 * @param rDir direction of the disk
 * @param pPartsFilter collision parts filter
 * @param pTriFilter triangle filter
 * @return the number of hits
 */
s32 checkStrikeDisk(const al::IUseCollision* pCollision, const sead::Vector3f& rPos, f32 radius,
                    f32 height, const sead::Vector3f& rDir,
                    const al::CollisionPartsFilterBase* pPartsFilter,
                    const al::TriangleFilterBase* pTriFilter) {
    al::CollisionDirector* director = getDirector(pCollision);
    director->setPartsFilter(pPartsFilter);
    director->setTriFilter(pTriFilter);
    return director->checkStrikeDisk(rPos, radius, height, rDir);
}

/**
 * Collects the collisions of an arrow.
 * @param pCollision collision user
 * @param rPos start of the arrow
 * @param rDir direction and length of the arrow
 * @param pPartsFilter collision parts filter
 * @param pTriFilter triangle filter
 * @return the number of hits
 */
s32 checkStrikeArrow(const al::IUseCollision* pCollision, const sead::Vector3f& rPos,
                     const sead::Vector3f& rDir, const al::CollisionPartsFilterBase* pPartsFilter,
                     const al::TriangleFilterBase* pTriFilter) {
    al::CollisionDirector* director = getDirector(pCollision);
    director->setPartsFilter(pPartsFilter);
    director->setTriFilter(pTriFilter);
    return director->checkStrikeArrow(rPos, rDir);
}

/**
 * Collects the collisions of a player sphere.
 * @param pCollision collision user
 * @param rPos center of the sphere
 * @param radius radius of the sphere
 * @param pPartsFilter collision parts filter
 * @param pTriFilter triangle filter
 * @return the number of hits
 */
s32 checkStrikeSphereForPlayer(const al::IUseCollision* pCollision, const sead::Vector3f& rPos,
                               f32 radius, const al::CollisionPartsFilterBase* pPartsFilter,
                               const al::TriangleFilterBase* pTriFilter) {
    al::CollisionDirector* director = getDirector(pCollision);
    director->setPartsFilter(pPartsFilter);
    director->setTriFilter(pTriFilter);
    return director->checkStrikeSphereForPlayer(rPos, radius);
}

/**
 * Finds the nearest hit of an arrow.
 * @param pCollision collision user
 * @param ppHitInfo output nearest hit info, or nullptr
 * @param rPos start of the arrow
 * @param rDir direction and length of the arrow
 * @param pPartsFilter collision parts filter
 * @param pTriFilter triangle filter
 * @return true if the arrow hit something
 */
bool getFirstPolyOnArrow(const al::IUseCollision* pCollision, const al::ArrowHitInfo** ppHitInfo,
                         const sead::Vector3f& rPos, const sead::Vector3f& rDir,
                         const al::CollisionPartsFilterBase* pPartsFilter,
                         const al::TriangleFilterBase* pTriFilter) {
    s32 hitNum = checkStrikeArrow(pCollision, rPos, rDir, pPartsFilter, pTriFilter);

    if (hitNum == 0) {
        return false;
    }

    f32 minDist = 1000000.0f;
    s32 minIndex = -1;

    for (s32 i = 0; i != hitNum; i++) {
        const al::ArrowHitInfo* hitInfo = getStrikeArrowInfo(pCollision, i);

        if ((pTriFilter == nullptr || !pTriFilter->isInvalidTriangle(hitInfo->mTriangle)) &&
            hitInfo->_70 < minDist) {
            minDist = hitInfo->_70;
            minIndex = i;
        }
    }

    if (minIndex == -1) {
        return false;
    }

    if (ppHitInfo != nullptr) {
        *ppHitInfo = getStrikeArrowInfo(pCollision, minIndex);
    }

    return true;
}

/**
 * Gets an arrow hit info.
 * @param pCollision collision user
 * @param index hit index
 * @return the hit info
 */
const al::ArrowHitInfo* getStrikeArrowInfo(const al::IUseCollision* pCollision, u32 index) {
    return getDirector(pCollision)->getStrikeArrowInfo(index);
}

/**
 * Finds the nearest hit of an arrow on collision parts with a special purpose.
 * @param pCollision collision user
 * @param pHitPos output hit position, or nullptr
 * @param pTriangle output hit triangle, or nullptr
 * @param rPos start of the arrow
 * @param rDir direction and length of the arrow
 * @param pSpecialPurpose special purpose of the collision parts
 * @return true if the arrow hit something
 */
bool getFirstPolyOnArrow(const al::IUseCollision* pCollision, sead::Vector3f* pHitPos,
                         al::Triangle* pTriangle, const sead::Vector3f& rPos,
                         const sead::Vector3f& rDir, const char* pSpecialPurpose) {
    al::CollisionPartsFilterSpecialPurpose filter(pSpecialPurpose);
    return getFirstPolyOnArrow(pCollision, pHitPos, pTriangle, rPos, rDir, &filter, nullptr);
}

/**
 * Finds the nearest hit of an arrow.
 * @param pCollision collision user
 * @param pHitPos output hit position, or nullptr
 * @param pTriangle output hit triangle, or nullptr
 * @param rPos start of the arrow
 * @param rDir direction and length of the arrow
 * @param pPartsFilter collision parts filter
 * @param pTriFilter triangle filter
 * @return true if the arrow hit something
 */
bool getFirstPolyOnArrow(const al::IUseCollision* pCollision, sead::Vector3f* pHitPos,
                         al::Triangle* pTriangle, const sead::Vector3f& rPos,
                         const sead::Vector3f& rDir,
                         const al::CollisionPartsFilterBase* pPartsFilter,
                         const al::TriangleFilterBase* pTriFilter) {
    const al::ArrowHitInfo* hitInfo = nullptr;

    if (!getFirstPolyOnArrow(pCollision, &hitInfo, rPos, rDir, pPartsFilter, pTriFilter)) {
        return false;
    }

    if (pHitPos != nullptr) {
        *pHitPos = hitInfo->mPos;
    }

    if (pTriangle != nullptr) {
        *pTriangle = hitInfo->mTriangle;
    }

    return true;
}

/**
 * Finds the nearest hit position of an arrow.
 * @param pCollision collision user
 * @param pHitPos output hit position, or nullptr
 * @param rPos start of the arrow
 * @param rDir direction and length of the arrow
 * @param pPartsFilter collision parts filter
 * @param pTriFilter triangle filter
 * @return true if the arrow hit something
 */
bool getHitPosOnArrow(const al::IUseCollision* pCollision, sead::Vector3f* pHitPos,
                      const sead::Vector3f& rPos, const sead::Vector3f& rDir,
                      const al::CollisionPartsFilterBase* pPartsFilter,
                      const al::TriangleFilterBase* pTriFilter) {
    al::Triangle triangle;
    return getFirstPolyOnArrow(pCollision, pHitPos, &triangle, rPos, rDir, pPartsFilter,
                               pTriFilter);
}

/**
 * Finds the nearest hit position and normal of an arrow.
 * @param pCollision collision user
 * @param pHitPos output hit position, or nullptr
 * @param pNormal output normal
 * @param rPos start of the arrow
 * @param rDir direction and length of the arrow
 * @param pPartsFilter collision parts filter
 * @param pTriFilter triangle filter
 * @return true if the arrow hit something
 */
bool getHitPosAndNormalOnArrow(const al::IUseCollision* pCollision, sead::Vector3f* pHitPos,
                               sead::Vector3f* pNormal, const sead::Vector3f& rPos,
                               const sead::Vector3f& rDir,
                               const al::CollisionPartsFilterBase* pPartsFilter,
                               const al::TriangleFilterBase* pTriFilter) {
    al::Triangle triangle;

    if (!getFirstPolyOnArrow(pCollision, pHitPos, &triangle, rPos, rDir, pPartsFilter,
                             pTriFilter)) {
        return false;
    }

    pNormal->set(*triangle.getNormal(0));
    return true;
}

/**
 * Finds the nearest hit position, normal and sensor of an arrow.
 * @param pCollision collision user
 * @param pHitPos output hit position, or nullptr
 * @param pNormal output normal
 * @param ppSensor output sensor
 * @param rPos start of the arrow
 * @param rDir direction and length of the arrow
 * @param pPartsFilter collision parts filter
 * @param pTriFilter triangle filter
 * @return true if the arrow hit something
 */
bool getHitPosAndNormalAndSensorOnArrow(const al::IUseCollision* pCollision,
                                        sead::Vector3f* pHitPos, sead::Vector3f* pNormal,
                                        al::HitSensor** ppSensor, const sead::Vector3f& rPos,
                                        const sead::Vector3f& rDir,
                                        const al::CollisionPartsFilterBase* pPartsFilter,
                                        const al::TriangleFilterBase* pTriFilter) {
    al::Triangle triangle;

    if (!getFirstPolyOnArrow(pCollision, pHitPos, &triangle, rPos, rDir, pPartsFilter,
                             pTriFilter)) {
        return false;
    }

    pNormal->set(*triangle.getNormal(0));
    *ppSensor = triangle.mCollisionParts->getSensor();
    return true;
}

/**
 * Finds the nearest hit position and sensor of an arrow.
 * @param pCollision collision user
 * @param pHitPos output hit position, or nullptr
 * @param ppSensor output sensor
 * @param rPos start of the arrow
 * @param rDir direction and length of the arrow
 * @param pPartsFilter collision parts filter
 * @param pTriFilter triangle filter
 * @return true if the arrow hit something
 */
bool getFirstCollisionSensorOnArrow(const al::IUseCollision* pCollision, sead::Vector3f* pHitPos,
                                    al::HitSensor** ppSensor, const sead::Vector3f& rPos,
                                    const sead::Vector3f& rDir,
                                    const al::CollisionPartsFilterBase* pPartsFilter,
                                    const al::TriangleFilterBase* pTriFilter) {
    al::Triangle triangle;

    if (!getFirstPolyOnArrow(pCollision, pHitPos, &triangle, rPos, rDir, pPartsFilter,
                             pTriFilter)) {
        return false;
    }

    *ppSensor = triangle.mCollisionParts->getSensor();
    return true;
}

/**
 * Finds the collision parts nearest hit by an arrow.
 * @param pCollision collision user
 * @param pHitPos output hit position, or nullptr
 * @param rPos start of the arrow
 * @param rDir direction and length of the arrow
 * @param pPartsFilter collision parts filter
 * @param pTriFilter triangle filter
 * @return the collision parts, or nullptr
 */
al::CollisionParts* getStrikeArrowCollisionParts(const al::IUseCollision* pCollision,
                                                 sead::Vector3f* pHitPos, const sead::Vector3f& rPos,
                                                 const sead::Vector3f& rDir,
                                                 const al::CollisionPartsFilterBase* pPartsFilter,
                                                 const al::TriangleFilterBase* pTriFilter) {
    al::Triangle triangle;

    if (!getFirstPolyOnArrow(pCollision, pHitPos, &triangle, rPos, rDir, pPartsFilter,
                             pTriFilter)) {
        return nullptr;
    }

    return const_cast<al::CollisionParts*>(triangle.mCollisionParts);
}

/**
 * Finds the sensor of the collision parts nearest hit by an arrow.
 * @param pCollision collision user
 * @param rPos start of the arrow
 * @param rDir direction and length of the arrow
 * @param pPartsFilter collision parts filter
 * @param pTriFilter triangle filter
 * @return the sensor, or nullptr
 */
al::HitSensor* tryGetStrikeArrowCollisionSensor(const al::IUseCollision* pCollision,
                                                const sead::Vector3f& rPos,
                                                const sead::Vector3f& rDir,
                                                const al::CollisionPartsFilterBase* pPartsFilter,
                                                const al::TriangleFilterBase* pTriFilter) {
    al::CollisionParts* parts =
        getStrikeArrowCollisionParts(pCollision, nullptr, rPos, rDir, pPartsFilter, pTriFilter);

    if (parts == nullptr) {
        return nullptr;
    }

    return parts->getSensor();
}

/**
 * Gets the number of arrow hits.
 * @param pCollision collision user
 * @return the number of hits
 */
u32 getStrikeArrowInfoNum(const al::IUseCollision* pCollision) {
    return getDirector(pCollision)->getStrikeArrowInfoNum();
}

/**
 * Gets a sphere hit info.
 * @param pCollision collision user
 * @param index hit index
 * @return the hit info
 */
const al::SphereHitInfo* getStrikeSphereInfo(const al::IUseCollision* pCollision, u32 index) {
    return getDirector(pCollision)->getStrikeSphereInfo(index);
}

/**
 * Gets the number of sphere hits.
 * @param pCollision collision user
 * @return the number of hits
 */
u32 getStrikeSphereInfoNum(const al::IUseCollision* pCollision) {
    return getDirector(pCollision)->getStrikeSphereInfoNum();
}

/**
 * Gets the position of a sphere hit.
 * @param pCollision collision user
 * @param index hit index
 * @return the hit position
 */
const sead::Vector3f& getStrikeSphereHitPos(const al::IUseCollision* pCollision, u32 index) {
    return getDirector(pCollision)->getStrikeSphereInfo(index)->mPos;
}

/**
 * Gets a disk hit info.
 * @param pCollision collision user
 * @param index hit index
 * @return the hit info
 */
const al::DiskHitInfo* getStrikeDiskInfo(const al::IUseCollision* pCollision, u32 index) {
    return getDirector(pCollision)->getStrikeDiskInfo(index);
}

/**
 * Gets the number of disk hits.
 * @param pCollision collision user
 * @return the number of hits
 */
u32 getStrikeDiskInfoNum(const al::IUseCollision* pCollision) {
    return getDirector(pCollision)->getStrikeDiskInfoNum();
}

/**
 * Gets the position of a disk hit.
 * @param pCollision collision user
 * @param index hit index
 * @return the hit position
 */
const sead::Vector3f& getStrikeDiskHitPos(const al::IUseCollision* pCollision, u32 index) {
    return getDirector(pCollision)->getStrikeDiskInfo(index)->mPos;
}

/**
 * Checks whether collision parts must be skipped by a check.
 * @param rParts collision parts
 * @param rCheckInfo check info
 * @return true if the collision parts must be skipped
 */
bool isInvalidParts(const al::CollisionParts& rParts, const al::CollisionCheckInfoBase& rCheckInfo) {
    if (rParts.getSpecialPurpose() != nullptr && rCheckInfo.getPartsFilter() == nullptr &&
        rCheckInfo.getTriangleFilter() == nullptr) {
        return true;
    }

    if (!rParts.isValidCollision()) {
        return true;
    }

    if (rCheckInfo.getPartsFilter() != nullptr &&
        rCheckInfo.getPartsFilter()->isInvalidParts(rParts)) {
        return true;
    }

    return false;
}

/**
 * Checks whether the bounding sphere of collision parts is out of range of a sphere.
 * @param rParts collision parts
 * @param rPos center of the sphere
 * @param distance radius of the sphere
 * @return true if the collision parts are out of range
 */
bool isFarAway(const al::CollisionParts& rParts, const sead::Vector3f& rPos, f32 distance) {
    f32 range = rParts.getBoundingSphereRange() + distance;
    const sead::Matrix34f& rMtx = rParts.getBaseMtx();
    f32 dx = sead::Mathf::abs(rMtx(0, 3) - rPos.x);

    if (range < dx) {
        return true;
    }

    f32 dy = sead::Mathf::abs(rMtx(1, 3) - rPos.y);

    if (range < dy) {
        return true;
    }

    f32 dz = sead::Mathf::abs(rMtx(2, 3) - rPos.z);

    if (range < dz) {
        return true;
    }

    return dx * dx + dy * dy + dz * dz > range * range;
}

/**
 * Collects the collisions of a sphere moving along a vector, sorted by time.
 * @param pCollision collision user
 * @param pHitInfos output hit infos
 * @param hitInfoNum maximum number of hit infos
 * @param rPos start position of the sphere
 * @param radius radius of the sphere
 * @param rMoveVec movement of the sphere
 * @param pPartsFilter collision parts filter
 * @param pTriFilter triangle filter
 * @return the number of hits
 */
s32 checkStrikeSphereMove(const al::IUseCollision* pCollision, SphereMoveHitInfo* pHitInfos,
                          s32 hitInfoNum, const sead::Vector3f& rPos, f32 radius,
                          const sead::Vector3f& rMoveVec,
                          const al::CollisionPartsFilterBase* pPartsFilter,
                          const al::TriangleFilterBase* pTriFilter) {
    sHitPrismFilter.reset(pTriFilter);
    al::SphereInterpolator interpolator;
    interpolator.startInterp(rPos, rPos + rMoveVec, radius, radius,
                             std::min(radius, 35.0f));
    interpolator.nextStep();
    s32 hitNum = 0;

    while (!interpolator.isEnd()) {
        sead::Vector3f pos = {0.0f, 0.0f, 0.0f};
        f32 size = 1.0f;
        interpolator.calcInterp(&pos, &size, nullptr);

        u32 strikeNum = checkStrikeSphereMovingReaction(pCollision, pos, size, rMoveVec,
                                                        pPartsFilter, &sHitPrismFilter);

        for (u32 i = 0; i < strikeNum; i++) {
            if (hitNum >= hitInfoNum) {
                break;
            }

            const al::SphereHitInfo* hitInfo = getStrikeSphereInfo(pCollision, i);
            f32 dist = hitInfo->_70;
            const al::Triangle& rTriangle = hitInfo->mTriangle;
            const sead::Vector3f& rFaceNormal = *rTriangle.getFaceNormal();

            if (rFaceNormal.dot(rMoveVec) < 0.0f) {
                SphereMoveHitInfo& rMoveHitInfo = pHitInfos[hitNum];

                if (hitInfo->isCollisionAtFace()) {
                    f32 time = dist / sead::Mathf::abs(rFaceNormal.dot(rMoveVec));
                    sead::Vector3f moveBack = rMoveVec * -time;
                    sead::Vector3f slide;
                    al::verticalizeVec(&slide, rFaceNormal, moveBack);
                    sead::Vector3f hitPos = hitInfo->mPos + slide;

                    sead::Vector3f pos0 = *rTriangle.getPos(0);
                    sead::Vector3f pos1 = *rTriangle.getPos(1);
                    sead::Vector3f pos2 = *rTriangle.getPos(2);
                    f32 edgeDot0 = rTriangle.getEdgeNormal(0)->dot(hitPos - pos0);
                    sead::Vector3f edgeNormal1 = *rTriangle.getEdgeNormal(1);
                    const sead::Vector3f& rEdgeNormal2 = *rTriangle.getEdgeNormal(2);

                    if (edgeDot0 > 0.0f || (hitPos - pos1).dot(edgeNormal1) > 0.0f ||
                        (hitPos - pos2).dot(rEdgeNormal2) > 0.0f) {
                        if (al::isNearZero(slide)) {
                            continue;
                        }

                        sead::Vector3f diff = hitPos - hitInfo->mPos;
                        s32 edgeIndex = 4;

                        if (rTriangle.getEdgeNormal(0)->dot(diff) > 0.0f) {
                            edgeIndex = 5;

                            if (rTriangle.getEdgeNormal(1)->dot(diff) > 0.0f) {
                                edgeIndex = 6;
                            }
                        }

                        s32 vertexIndex = al::modi(edgeIndex, 3);
                        sead::Vector3f toVertex = *rTriangle.getPos(vertexIndex) - hitInfo->mPos;
                        sead::Vector3f cross;
                        cross.setCross(toVertex, diff);
                        s32 crossEdgeIndex =
                            al::modi(vertexIndex + (rFaceNormal.dot(cross) > 0.0f ? 4 : 3), 3);
                        const sead::Vector3f& rCrossEdgeNormal =
                            *rTriangle.getEdgeNormal(crossEdgeIndex);
                        f32 denom = rCrossEdgeNormal.dot(diff);

                        if (al::isNearZero(denom)) {
                            continue;
                        }

                        f32 rate = rCrossEdgeNormal.dot(toVertex) / denom;
                        sead::Vector3f edgePos = hitInfo->mPos + diff * rate;
                        f32 hitTime = calcSphereHitTime(edgePos, pos, size, rMoveVec);
                        rMoveHitInfo.time = interpolator.getCurrentStep() - hitTime;
                        rMoveHitInfo.pos = edgePos;
                    } else {
                        rMoveHitInfo.time = interpolator.getCurrentStep() - time;
                        rMoveHitInfo.pos = hitPos;
                    }
                } else {
                    f32 hitTime = calcSphereHitTime(hitInfo->mPos, pos, size, rMoveVec);
                    rMoveHitInfo.time = interpolator.getCurrentStep() - hitTime;
                    rMoveHitInfo.pos = hitInfo->mPos;
                }

                rMoveHitInfo.triangle = rTriangle;
                hitNum++;
            }

            sHitPrismFilter.addHitPrism(rTriangle.mPrismData);
        }

        if (hitNum >= hitInfoNum) {
            break;
        }

        interpolator.nextStep();
    }

    std::sort(pHitInfos, pHitInfos + hitNum, SphereMoveHitInfo::compare);
    return hitNum;
}

/**
 * Compares the times of two sphere move hits.
 * @param rLhs first hit
 * @param rRhs second hit
 * @return true if the first hit happens earlier
 */
bool SphereMoveHitInfo::compare(const SphereMoveHitInfo& rLhs, const SphereMoveHitInfo& rRhs) {
    return rLhs.time < rRhs.time;
}

/**
 * Calls a delegate for every filtered collision parts near a sphere.
 * @param pCollision collision user
 * @param rPos center of the sphere
 * @param radius radius of the sphere
 * @param rDelegate delegate to call
 * @param pPartsFilter collision parts filter
 */
void searchCollisionParts(const al::IUseCollision* pCollision, const sead::Vector3f& rPos,
                          f32 radius, sead::IDelegate1<al::CollisionParts*>& rDelegate,
                          const al::CollisionPartsFilterBase* pPartsFilter) {
    getDirector(pCollision)->searchCollisionPartsWithSphere(rPos, radius, rDelegate, pPartsFilter);
}

/**
 * Calls a delegate for every collision parts near a sphere.
 * @param pCollision collision user
 * @param rPos center of the sphere
 * @param radius radius of the sphere
 * @param rDelegate delegate to call
 */
void searchCollisionParts(const al::IUseCollision* pCollision, const sead::Vector3f& rPos,
                          f32 radius, sead::IDelegate1<al::CollisionParts*>& rDelegate) {
    getDirector(pCollision)->searchCollisionPartsWithSphere(rPos, radius, rDelegate);
}

/**
 * Restricts collision checks to a collision parts array.
 * @param pCollision collision user
 * @param pPartsArray collision parts array
 */
void validateCollisionPartsPtrArray(const al::IUseCollision* pCollision,
                                    sead::PtrArray<al::CollisionParts>* pPartsArray) {
    getDirector(pCollision)->validateCollisionPartsPtrArray(pPartsArray);
}

/**
 * Stops restricting collision checks to a collision parts array.
 * @param pCollision collision user
 */
void invalidateCollisionPartsPtrArray(const al::IUseCollision* pCollision) {
    getDirector(pCollision)->invalidateCollisionPartsPtrArray();
}

/**
 * Gets the collision parts array collision checks are restricted to.
 * @param pCollision collision user
 * @return the collision parts array
 */
sead::PtrArray<al::CollisionParts>* getCollisionPartsPtrArray(const al::IUseCollision* pCollision) {
    return getDirector(pCollision)->getCollisionPartsPtrArray();
}
}  // namespace alCollisionUtil
