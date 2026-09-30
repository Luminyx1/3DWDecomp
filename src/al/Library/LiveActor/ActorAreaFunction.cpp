#include "Library/LiveActor/ActorAreaFunction.hpp"

#include "Library/Actor/ActorInitInfo.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorFlagUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Player/PlayerUtil.hpp"
#include "Project/AreaObj/AreaObj.hpp"
#include "Project/AreaObj/AreaObjGroup.hpp"

namespace al {
namespace {
inline bool isPlayerAreaTarget(const LiveActor* pPlayer) {
    return !isDead(pPlayer) && isAreaTarget(pPlayer);
}
}  // namespace

/**
 * Gets the highest priority area of a group that contains all area target players.
 * @param pActor The actor used to access the players.
 * @param pGroup The area group.
 * @return The area or nullptr.
 */
AreaObj* tryGetAreaObjPlayerAll(const LiveActor* pActor, const AreaObjGroup* pGroup) {
    if (!pGroup) {
        return nullptr;
    }
    AreaObj* result = nullptr;
    s32 num = pGroup->mNumAreas;
    for (s32 i = 0; i < num; i++) {
        AreaObj* area = pGroup->getAreaObj(i);
        if (!isInAreaObjPlayerAll(pActor, area)) {
            continue;
        }
        if (!result) {
            result = area;
        } else {
            result = result->mPriority < area->mPriority ? area : result;
        }
    }
    return result;
}

/**
 * Checks whether all area target players are inside an area.
 * @param pActor The actor used to access the players.
 * @param pArea The area.
 * @return Whether at least one player is inside and none is outside.
 */
bool isInAreaObjPlayerAll(const LiveActor* pActor, const AreaObj* pArea) {
    s32 playerNum = getPlayerNumMax(pActor);
    bool isIn = false;
    for (s32 i = 0; i < playerNum; i++) {
        LiveActor* player = getPlayerActor(pActor, i);
        if (!isPlayerAreaTarget(player)) {
            continue;
        }
        if (!pArea->isInVolume(getTrans(player))) {
            return false;
        }
        isIn = true;
    }
    return isIn;
}

/**
 * Checks whether all area target players are inside one area of a group.
 * @param pActor The actor used to access the players.
 * @param pGroup The area group.
 * @return Whether such an area exists.
 */
bool isInAreaObjPlayerAll(const LiveActor* pActor, const AreaObjGroup* pGroup) {
    s32 num = pGroup->mNumAreas;
    for (s32 i = 0; i < num; i++) {
        if (isInAreaObjPlayerAll(pActor, pGroup->getAreaObj(i))) {
            return true;
        }
    }
    return false;
}

/**
 * Checks whether any area target player is inside an area.
 * @param pActor The actor used to access the players.
 * @param pArea The area.
 * @return Whether a player is inside.
 */
bool isInAreaObjPlayerAnyOne(const LiveActor* pActor, const AreaObj* pArea) {
    s32 playerNum = getPlayerNumMax(pActor);
    for (s32 i = 0; i < playerNum; i++) {
        LiveActor* player = getPlayerActor(pActor, i);
        if (!isPlayerAreaTarget(player)) {
            continue;
        }
        if (pArea->isInVolume(getTrans(player))) {
            return true;
        }
    }
    return false;
}

/**
 * Checks whether any area target player is inside any area of a group.
 * @param pActor The actor used to access the players.
 * @param pGroup The area group.
 * @return Whether a player is inside.
 */
bool isInAreaObjPlayerAnyOne(const LiveActor* pActor, const AreaObjGroup* pGroup) {
    s32 num = pGroup->mNumAreas;
    for (s32 i = 0; i < num; i++) {
        if (isInAreaObjPlayerAnyOne(pActor, pGroup->getAreaObj(i))) {
            return true;
        }
    }
    return false;
}

/**
 * Checks whether a position is inside an area.
 * @param pArea The area.
 * @param rPos The position.
 * @return Whether the position is inside.
 */
bool tryIsInAreaPos(const AreaObj* pArea, const sead::Vector3f& rPos) {
    return pArea->isInVolume(rPos);
}

/**
 * Creates an area object from the actor's placement.
 * @param rInfo The actor init info.
 * @param pName The area name.
 * @return The area.
 */
AreaObj* createAreaObj(const ActorInitInfo& rInfo, const char* pName) {
    AreaInitInfo areaInitInfo(*rInfo.mPlacementInfo, rInfo.mStageSwitchDirector);
    AreaObj* area = new AreaObj(pName);
    area->init(areaInitInfo);
    return area;
}

/**
 * Creates an area group from the areas linked to an actor.
 * @param pActor The actor.
 * @param rInfo The actor init info.
 * @param pLinkName The link name.
 * @param pGroupName The group name.
 * @param pAreaName The area name.
 * @return The area group, or nullptr if nothing is linked.
 */
AreaObjGroup* createLinkAreaGroup(LiveActor* pActor, const ActorInitInfo& rInfo,
                                  const char* pLinkName, const char* pGroupName,
                                  const char* pAreaName) {
    s32 num = calcLinkChildNum(rInfo, pLinkName);
    if (num <= 0) {
        return nullptr;
    }
    AreaObjGroup* group = new AreaObjGroup(pGroupName);
    group->createBuffer(num);
    const PlacementInfo& placementInfo = *rInfo.mPlacementInfo;
    for (s32 i = 0; i < num; i++) {
        PlacementInfo linkInfo;
        getLinksInfoByIndex(&linkInfo, placementInfo, pLinkName, i);
        AreaInitInfo areaInitInfo(linkInfo, rInfo.mStageSwitchDirector);
        AreaObj* area = new AreaObj(pAreaName);
        area->init(areaInitInfo);
        group->resisterAreaObj(area);
    }
    return group;
}
}  // namespace al
