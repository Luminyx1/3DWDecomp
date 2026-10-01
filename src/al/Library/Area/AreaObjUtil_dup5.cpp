#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/System/SystemKit.hpp"
#include "Project/AreaObj/AreaObj.hpp"
#include "Project/AreaObj/AreaObjDirector.hpp"
#include "Project/AreaObj/AreaObjGroup.hpp"
#include "Project/AreaObj/AreaObjUtil.hpp"

namespace al {
static AreaObj* findAreaObj(const IUseAreaObj* pAreaUser, const char* pName,
                            const sead::Vector3f& rPos) {
    return pAreaUser->getAreaObjDirector()->getInVolumeAreaObj(pName, rPos);
}

static bool checkArrow(sead::Vector3f* pHitPos, sead::Vector3f* pNormal, const AreaObj* pAreaObj,
                       const sead::Vector3f& rStart, const sead::Vector3f& rEnd) {
    return pAreaObj->mShape->checkArrowCollision(pHitPos, pNormal, rStart, rEnd);
}

/**
 * Calculates how deep a position is below the surface of a water area.
 * @param pAreaUser area user
 * @param rPos position to check
 * @return depth below the surface, or -1 if the position isn't in water
 */
f32 calcWaterSinkDepth(const IUseAreaObj* pAreaUser, const sead::Vector3f& rPos) {
    if (findAreaObj(pAreaUser, "NoWaterArea", rPos) != nullptr) {
        return -1.0f;
    }

    AreaObj* areaObj = findAreaObj(pAreaUser, "WaterArea", rPos);

    if (areaObj == nullptr) {
        return -1.0f;
    }

    sead::Vector3f hitPos;
    sead::Vector3f normal;
    sead::Vector3f above = {rPos.x, rPos.y + 100000.0f, rPos.z};

    if (!checkArrow(&hitPos, &normal, areaObj, rPos, above)) {
        return -1.0f;
    }

    return hitPos.y - rPos.y;
}

/**
 * Finds the height of a water surface within a distance above or below a position.
 * @param pAreaUser area user
 * @param rPos position to check
 * @param distance distance to search
 * @param pHeight output surface height
 * @return true if a surface was found
 */
bool calcWaterDistanceCheck(const IUseAreaObj* pAreaUser, const sead::Vector3f& rPos,
                            f32 distance, f32* pHeight) {
    AreaObj* areaObj = findAreaObj(pAreaUser, "WaterArea", rPos);

    if (areaObj != nullptr) {
        sead::Vector3f hitPos;
        sead::Vector3f normal;
        sead::Vector3f end = {rPos.x, rPos.y + distance, rPos.z};

        if (!checkArrow(&hitPos, &normal, areaObj, rPos, end)) {
            end = rPos + sead::Vector3f(0.0f, -distance, 0.0f);

            if (!checkArrow(&hitPos, &normal, areaObj, rPos, end)) {
                return false;
            }
        }

        *pHeight = hitPos.y;
        return true;
    }

    sead::Vector3f start = {rPos.x, rPos.y + distance, rPos.z};
    sead::Vector3f end = start + sead::Vector3f(0.0f, distance, 0.0f);
    AreaObj* startAreaObj = findAreaObj(pAreaUser, "WaterArea", start);

    if (startAreaObj != nullptr) {
        sead::Vector3f hitPos;
        sead::Vector3f normal;

        if (!checkArrow(&hitPos, &normal, startAreaObj, rPos, end)) {
            end = rPos + sead::Vector3f(0.0f, -distance, 0.0f);

            if (!checkArrow(&hitPos, &normal, startAreaObj, start, end)) {
                return false;
            }
        }

        *pHeight = hitPos.y;
        return true;
    }

    return false;
}

/**
 * Calculates how deep an actor is below the surface of a water area.
 * @param pActor actor to check
 * @return depth below the surface, or -1 if the actor isn't in water
 */
f32 calcWaterSinkDepth(const LiveActor* pActor) {
    return calcWaterSinkDepth(pActor, getTrans(pActor));
}

/**
 * Checks whether a position is inside a Plessie tunnel graphics area.
 * @param pAreaUser area user
 * @param rPos position to check
 * @return true if such an area contains the position
 */
bool isInPlessieTunnel(const IUseAreaObj* pAreaUser, const sead::Vector3f& rPos) {
    AreaObjGroup* group = pAreaUser->getAreaObjDirector()->getAreaObjGroup("GraphicsArea");

    if (group == nullptr) {
        return false;
    }

    s32 num = group->mNumAreas;

    for (s32 i = 0; i < num; i++) {
        AreaObj* areaObj = group->getAreaObj(i);

        if (areaObj->mIsPlessieTunnel && areaObj->isInVolume(rPos)) {
            return true;
        }
    }

    return false;
}

/**
 * Gets an integer argument of an area.
 * @param pArg output argument
 * @param pAreaObj area to read from
 * @param pKey argument key
 * @return true if the argument exists
 */
bool tryGetAreaObjArg(s32* pArg, const AreaObj* pAreaObj, const char* pKey) {
    if (pAreaObj->mPlacementInfo == nullptr) {
        return false;
    }

    return tryGetArg(pArg, *pAreaObj->mPlacementInfo, pKey);
}

/**
 * Gets a float argument of an area.
 * @param pArg output argument
 * @param pAreaObj area to read from
 * @param pKey argument key
 * @return true if the argument exists
 */
bool tryGetAreaObjArg(f32* pArg, const AreaObj* pAreaObj, const char* pKey) {
    if (pAreaObj->mPlacementInfo == nullptr) {
        return false;
    }

    return tryGetArg(pArg, *pAreaObj->mPlacementInfo, pKey);
}

/**
 * Gets a bool argument of an area.
 * @param pArg output argument
 * @param pAreaObj area to read from
 * @param pKey argument key
 * @return true if the argument exists
 */
bool tryGetAreaObjArg(bool* pArg, const AreaObj* pAreaObj, const char* pKey) {
    if (pAreaObj->mPlacementInfo == nullptr) {
        return false;
    }

    return tryGetArg(pArg, *pAreaObj->mPlacementInfo, pKey);
}

/**
 * Gets a string argument of an area.
 * @param pArg output argument
 * @param pAreaObj area to read from
 * @param pKey argument key
 * @return true if the argument exists
 */
bool tryGetAreaObjStringArg(const char** pArg, const AreaObj* pAreaObj, const char* pKey) {
    if (pAreaObj->mPlacementInfo == nullptr) {
        return false;
    }

    return tryGetStringArg(pArg, *pAreaObj->mPlacementInfo, pKey);
}

/**
 * Checks whether the player is inside an area of a possibly missing group.
 * @param pGroup area group, or nullptr
 * @return true if an area contains the player
 */
bool tryIsInAreaObjPlayer(AreaObjGroup* pGroup) {
    const sead::Vector3f& playerPos = alProjectInterface::getPlayerPos();

    if (pGroup == nullptr) {
        return false;
    }

    return pGroup->getInVolumeAreaObj(playerPos) != nullptr;
}

/**
 * Finds the area of a possibly missing group containing the player.
 * @param pGroup area group, or nullptr
 * @return the area, or nullptr if none contains the player
 */
AreaObj* tryGetAreaObjPlayer(AreaObjGroup* pGroup) {
    const sead::Vector3f& playerPos = alProjectInterface::getPlayerPos();

    if (pGroup == nullptr) {
        return nullptr;
    }

    return pGroup->getInVolumeAreaObj(playerPos);
}

/**
 * Checks whether the player is inside an area.
 * @param pAreaObj area to check
 * @return true if the area contains the player
 */
bool tryIsInAreaPlayer(const AreaObj* pAreaObj) {
    return pAreaObj->isInVolume(alProjectInterface::getPlayerPos());
}

/**
 * Gets the base matrix of an area.
 * @param pAreaObj area to read from
 * @return the base matrix
 */
const sead::Matrix34f& getAreaObjBaseMtx(const AreaObj* pAreaObj) {
    return pAreaObj->_28;
}
}  // namespace al
