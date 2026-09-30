#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorFlagUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Player/PlayerUtil.hpp"
#include "Project/AreaObj/AreaObj.hpp"
#include "Project/AreaObj/AreaObjDirector.hpp"
#include "Project/AreaObj/AreaObjUtil.hpp"

namespace al {
/**
 * Checks whether a position is inside an area.
 * @param pAreaObj area to check
 * @param rPos position to check
 * @return true if the area contains the position
 */
bool isInAreaPos(const AreaObj* pAreaObj, const sead::Vector3f& rPos) {
    return pAreaObj->isInVolume(rPos);
}

/**
 * Checks whether an area of a group contains a position.
 * @param pAreaUser area user
 * @param pName name of the group
 * @param rPos position to check
 * @return true if an area contains the position
 */
bool isInAreaObj(const IUseAreaObj* pAreaUser, const char* pName, const sead::Vector3f& rPos) {
    return pAreaUser->getAreaObjDirector()->getInVolumeAreaObj(pName, rPos) != nullptr;
}

/**
 * Checks whether an area of a group is hit by a line segment.
 * @param pAreaUser area user
 * @param pName name of the group
 * @param rStart start of the segment
 * @param rEnd end of the segment
 * @param pHitPos output hit position
 * @param pNormal output hit normal
 * @return true if an area is hit
 */
bool isInAreaObj(const IUseAreaObj* pAreaUser, const char* pName, const sead::Vector3f& rStart,
                 const sead::Vector3f& rEnd, sead::Vector3f* pHitPos, sead::Vector3f* pNormal) {
    return pAreaUser->getAreaObjDirector()->getInVolumeAreaObj(pName, rStart, rEnd, pHitPos,
                                                               pNormal) != nullptr;
}

/**
 * Checks whether an area target actor is inside an area of a group.
 * @param pActor actor to check
 * @param pName name of the group
 * @return true if the actor is an area target inside an area
 */
bool isInAreaObj(const LiveActor* pActor, const char* pName) {
    if (!isAreaTarget(pActor)) {
        return false;
    }

    return isInAreaObj(pActor, pName, getTrans(pActor));
}

/**
 * Checks whether any living player is inside an area of a group.
 * @param pAreaUser area user
 * @param pName name of the group
 * @param pPlayerHolder player holder
 * @return true if a player is inside an area
 */
bool isInAreaObjPlayerOne(const IUseAreaObj* pAreaUser, const char* pName,
                          const PlayerHolder* pPlayerHolder) {
    s32 num = getPlayerNumMax(pPlayerHolder);

    for (s32 i = 0; i < num; i++) {
        LiveActor* player = getPlayerActor(pPlayerHolder, i);

        if (isDead(player) || !isAreaTarget(player)) {
            continue;
        }

        if (isInAreaObj(pAreaUser, pName, getTrans(player))) {
            return true;
        }
    }

    return false;
}

/**
 * Checks whether all living players are inside an area of a group.
 * @param pAreaUser area user
 * @param pName name of the group
 * @param pPlayerHolder player holder
 * @return true if no living player is outside the areas
 */
bool isInAreaObjPlayerAll(const IUseAreaObj* pAreaUser, const char* pName,
                          const PlayerHolder* pPlayerHolder) {
    s32 num = getPlayerNumMax(pPlayerHolder);

    for (s32 i = 0; i < num; i++) {
        LiveActor* player = getPlayerActor(pPlayerHolder, i);

        if (isDead(player) || !isAreaTarget(player)) {
            continue;
        }

        if (!isInAreaObj(pAreaUser, pName, getTrans(player))) {
            return false;
        }
    }

    return true;
}

/**
 * Checks whether an area group exists.
 * @param pAreaUser area user
 * @param pName name of the group
 * @return true if the group exists
 */
bool isExistAreaObj(const IUseAreaObj* pAreaUser, const char* pName) {
    return pAreaUser->getAreaObjDirector()->isExistAreaGroup(pName);
}

/**
 * Checks whether a position is inside a death area.
 * @param pAreaUser area user
 * @param rPos position to check
 * @return true if a death area contains the position
 */
bool isInDeathArea(const IUseAreaObj* pAreaUser, const sead::Vector3f& rPos) {
    return isInAreaObj(pAreaUser, "DeathArea", rPos);
}
}  // namespace al
