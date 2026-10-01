#include "Project/AreaObj/AreaObjUtil.hpp"

#include "Library/LiveActor/Util/ActorFlagUtil.hpp"
#include "Library/Player/PlayerUtil.hpp"
#include "Project/AreaObj/AreaObj.hpp"
#include "Project/AreaObj/AreaObjDirector.hpp"
#include "Project/AreaObj/AreaObjGroup.hpp"
#include "Project/AreaObj/IUseAreaObj.hpp"

namespace al {
/**
 * Finds the highest priority area of a group containing a position.
 * @param pAreaUser area user
 * @param pName name of the group
 * @param rPos position to check
 * @return the area, or nullptr if none contains the position
 */
AreaObj* tryFindAreaObj(const IUseAreaObj* pAreaUser, const char* pName,
                        const sead::Vector3f& rPos) {
    return pAreaUser->getAreaObjDirector()->getInVolumeAreaObj(pName, rPos);
}

/**
 * Finds the highest priority area of a group hit by a line segment.
 * @param pAreaUser area user
 * @param pName name of the group
 * @param rStart start of the segment
 * @param rEnd end of the segment
 * @param pHitPos output hit position
 * @param pNormal output hit normal
 * @return the area, or nullptr if none is hit
 */
AreaObj* tryFindAreaObj(const IUseAreaObj* pAreaUser, const char* pName,
                        const sead::Vector3f& rStart, const sead::Vector3f& rEnd,
                        sead::Vector3f* pHitPos, sead::Vector3f* pNormal) {
    return pAreaUser->getAreaObjDirector()->getInVolumeAreaObj(pName, rStart, rEnd, pHitPos,
                                                               pNormal);
}

/**
 * Finds the highest priority area of a group containing a position that passes a filter.
 * @param pAreaUser area user
 * @param pName name of the group
 * @param rPos position to check
 * @param pFilter filter the area must pass
 * @return the area, or nullptr if none was found
 */
AreaObj* tryFindAreaObjWithFilter(const IUseAreaObj* pAreaUser, const char* pName,
                                  const sead::Vector3f& rPos, AreaObjFilterBase* pFilter) {
    AreaObjGroup* group = tryFindAreaObjGroup(pAreaUser, pName);

    if (group == nullptr) {
        return nullptr;
    }

    AreaObj* result = nullptr;
    s32 num = group->getSize();

    for (s32 i = 0; i < num; i++) {
        AreaObj* areaObj = group->getAreaObj(i);

        if ((result == nullptr || result->getPriority() <= areaObj->getPriority()) && areaObj->isInVolume(rPos) &&
            pFilter->isValidArea(areaObj)) {
            result = areaObj;
        }
    }

    return result;
}

/**
 * Finds an area group by name.
 * @param pAreaUser area user
 * @param pName name of the group
 * @return the group, or nullptr if it doesn't exist
 */
AreaObjGroup* tryFindAreaObjGroup(const IUseAreaObj* pAreaUser, const char* pName) {
    return pAreaUser->getAreaObjDirector()->getAreaObjGroup(pName);
}

/**
 * Finds the first code linked area of a group that isn't a Plessie camera area.
 * @param pAreaUser area user
 * @param pName name of the group
 * @return the area, or nullptr if none was found
 */
AreaObj* tryFindAreaObj(const IUseAreaObj* pAreaUser, const char* pName) {
    AreaObjGroup* group = tryFindAreaObjGroup(pAreaUser, pName);

    if (group == nullptr) {
        return nullptr;
    }

    s32 num = group->getSize();

    for (s32 i = 0; i < num; i++) {
        AreaObj* areaObj = group->getAreaObj(i);

        if (areaObj != nullptr && areaObj->mIsCodeLink && !areaObj->mIsPlessieCameraOn) {
            return areaObj;
        }
    }

    return nullptr;
}

/**
 * Finds the first code linked Plessie camera area of a group.
 * @param pAreaUser area user
 * @param pName name of the group
 * @return the area, or nullptr if none was found
 */
AreaObj* tryFindPlessieAreaObj(const IUseAreaObj* pAreaUser, const char* pName) {
    AreaObjGroup* group = tryFindAreaObjGroup(pAreaUser, pName);

    if (group == nullptr) {
        return nullptr;
    }

    s32 num = group->getSize();

    for (s32 i = 0; i < num; i++) {
        AreaObj* areaObj = group->getAreaObj(i);

        if (areaObj != nullptr && areaObj->mIsCodeLink && areaObj->mIsPlessieCameraOn) {
            return areaObj;
        }
    }

    return nullptr;
}

/**
 * Finds the first Plessie tunnel area of a group.
 * @param pAreaUser area user
 * @param pName name of the group
 * @return the area, or nullptr if none was found
 */
AreaObj* tryFindPlessieTunnelAreaObj(const IUseAreaObj* pAreaUser, const char* pName) {
    AreaObjGroup* group = tryFindAreaObjGroup(pAreaUser, pName);

    if (group == nullptr) {
        return nullptr;
    }

    s32 num = group->getSize();

    for (s32 i = 0; i < num; i++) {
        AreaObj* areaObj = group->getAreaObj(i);

        if (areaObj->mIsPlessieTunnel) {
            return areaObj;
        }
    }

    return nullptr;
}

/**
 * Checks whether any area of a group contains a position.
 * @param pAreaUser area user
 * @param pName name of the group
 * @param rPos position to check
 * @return true if an area contains the position
 */
bool isInAreaObjInGroup(const IUseAreaObj* pAreaUser, const char* pName,
                        const sead::Vector3f& rPos) {
    AreaObjGroup* group = tryFindAreaObjGroup(pAreaUser, pName);

    if (group == nullptr) {
        return false;
    }

    return group->getInFirstAreaObj(rPos) != nullptr;
}

/**
 * Checks whether a position is inside a code linked disaster camera area.
 * @param pAreaUser area user
 * @param rPos position to check
 * @return true if such an area contains the position
 */
bool isInDisasterCameraArea(const IUseAreaObj* pAreaUser, const sead::Vector3f& rPos) {
    AreaObjGroup* group = tryFindAreaObjGroup(pAreaUser, "CameraArea");

    if (group == nullptr) {
        return false;
    }

    s32 num = group->getSize();

    for (s32 i = 0; i < num; i++) {
        AreaObj* areaObj = group->getAreaObj(i);

        if (areaObj != nullptr && areaObj->mIsCodeLink && areaObj->mIsDisasterCameraOn &&
            areaObj->isInVolumeCheck(rPos)) {
            return true;
        }
    }

    return false;
}

/**
 * Checks whether a position is inside a code linked Plessie camera area.
 * @param pAreaUser area user
 * @param rPos position to check
 * @return true if such an area contains the position
 */
bool isInPlessieCameraArea(const IUseAreaObj* pAreaUser, const sead::Vector3f& rPos) {
    AreaObjGroup* group = tryFindAreaObjGroup(pAreaUser, "CameraArea");

    if (group == nullptr) {
        return false;
    }

    s32 num = group->getSize();

    for (s32 i = 0; i < num; i++) {
        AreaObj* areaObj = group->getAreaObj(i);

        if (areaObj != nullptr && areaObj->mIsCodeLink && areaObj->mIsPlessieCameraOn &&
            areaObj->isInVolumeCheck(rPos)) {
            return true;
        }
    }

    return false;
}

/**
 * Finds the entrance camera area containing a position.
 * @param pAreaUser area user
 * @param rPos position to check
 * @param pCameraDirector camera director
 * @return the area, or nullptr if none contains the position
 */
AreaObj* getStartCameraArea(const IUseAreaObj* pAreaUser, const sead::Vector3f& rPos,
                            CameraDirector_RS* pCameraDirector) {
    AreaObjGroup* group = tryFindAreaObjGroup(pAreaUser, "CameraArea");

    if (group == nullptr) {
        return nullptr;
    }

    s32 num = group->getSize();

    for (s32 i = 0; i < num; i++) {
        AreaObj* areaObj = group->getAreaObj(i);

        if (areaObj != nullptr && areaObj->mIsSpawnEntranceCamera && areaObj->isInVolumeCheck(rPos)) {
            return areaObj;
        }
    }

    return nullptr;
}

/**
 * Adds an area to the extra area list.
 * @param pAreaUser area user
 * @param pAreaObj area to add
 */
void addToExtraAreaObjectGroup(const IUseAreaObj* pAreaUser, AreaObj* pAreaObj) {
    pAreaUser->getAreaObjDirector()->addToExtraAreaGroup(pAreaObj);
}

/**
 * Finds an extra area containing a position.
 * @param pAreaUser area user
 * @param rPos position to check
 * @return the area, or nullptr if none contains the position
 */
AreaObj* tryGetAreaInExtraAreaGroupAtPos(const IUseAreaObj* pAreaUser,
                                         const sead::Vector3f& rPos) {
    return pAreaUser->getAreaObjDirector()->tryFindInExtraAreaObjGroup(rPos);
}

/**
 * Finds the highest priority area of a group containing any living player.
 * @param pAreaUser area user
 * @param pName name of the group
 * @param pPlayerHolder player holder
 * @return the area, or nullptr if no player is inside an area
 */
AreaObj* tryFindAreaObjPlayerOne(const IUseAreaObj* pAreaUser, const char* pName,
                                 const PlayerHolder* pPlayerHolder) {
    AreaObj* result = nullptr;
    s32 num = getPlayerNumMax(pPlayerHolder);

    for (s32 i = 0; i < num; i++) {
        if (isPlayerDead(pPlayerHolder, i)) {
            continue;
        }

        if (!isAreaTarget(getPlayerActor(pPlayerHolder, i))) {
            continue;
        }

        AreaObj* areaObj = tryFindAreaObj(pAreaUser, pName, getPlayerPos(pPlayerHolder, i));

        if (areaObj == nullptr) {
            continue;
        }

        if (result == nullptr || result->getPriority() < areaObj->getPriority()) {
            result = areaObj;
        }
    }

    return result;
}

/**
 * Finds the highest priority area of a group containing all living players.
 * @param pAreaUser area user
 * @param pName name of the group
 * @param pPlayerHolder player holder
 * @return the area, or nullptr if no area contains all players
 */
AreaObj* tryFindAreaObjPlayerAll(const IUseAreaObj* pAreaUser, const char* pName,
                                 const PlayerHolder* pPlayerHolder) {
    AreaObjGroup* group = tryFindAreaObjGroup(pAreaUser, pName);

    if (group == nullptr) {
        return nullptr;
    }

    s32 areaNum = group->getSize();
    s32 playerNum = getPlayerNumMax(pPlayerHolder);
    AreaObj* result = nullptr;

    for (s32 i = 0; i < areaNum; i++) {
        AreaObj* areaObj = group->getAreaObj(i);
        AreaObj* found = nullptr;

        for (s32 j = 0; j < playerNum; j++) {
            if (isPlayerDead(pPlayerHolder, j)) {
                continue;
            }

            bool isIn = areaObj->isInVolume(getPlayerPos(pPlayerHolder, j));
            found = areaObj;

            if (!isIn) {
                found = nullptr;
                break;
            }
        }

        if (found == nullptr) {
            continue;
        }

        if (result == nullptr || result->getPriority() < found->mPriority) {
            result = found;
        }
    }

    return result;
}

/**
 * Checks whether an area of a group contains a position.
 * @param pGroup area group
 * @param rPos position to check
 * @return true if an area contains the position
 */
bool isInAreaObj(AreaObjGroup* pGroup, const sead::Vector3f& rPos) {
    return pGroup->getInVolumeAreaObj(rPos) != nullptr;
}

/**
 * Checks whether an area of a possibly missing group contains a position.
 * @param pGroup area group, or nullptr
 * @param rPos position to check
 * @return true if an area contains the position
 */
bool tryIsInAreaObj(AreaObjGroup* pGroup, const sead::Vector3f& rPos) {
    if (pGroup == nullptr) {
        return false;
    }

    return pGroup->getInVolumeAreaObj(rPos) != nullptr;
}

/**
 * Finds the area of a possibly missing group containing a position.
 * @param pGroup area group, or nullptr
 * @param rPos position to check
 * @return the area, or nullptr if none contains the position
 */
AreaObj* tryGetAreaObj(AreaObjGroup* pGroup, const sead::Vector3f& rPos) {
    if (pGroup == nullptr) {
        return nullptr;
    }

    return pGroup->getInVolumeAreaObj(rPos);
}
}  // namespace al
