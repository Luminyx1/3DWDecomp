#include "Util/AreaObjUtil.hpp"
#include <cstring>
#include "AreaObj/NoRainArea.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorFlagUtil.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Player/PlayerUtil.hpp"
#include "Project/AreaObj/AreaObj.hpp"
#include "Project/AreaObj/AreaObjDirector.hpp"
#include "Project/AreaObj/AreaObjGroup.hpp"
#include "Project/AreaObj/IUseAreaObj.hpp"
#include "System/Data/SingleModeDataFunction.hpp"
#include "System/GameDataHolderAccessor.hpp"

namespace {
/// Area group index of every area object type, -1 when the scene has no such group.
s32 sAreaObjIndex[rc::AreaObjType::size()];
}  // namespace

namespace rc {
/**
 * Finds the area group of an area object type.
 * @param pAreaUser area user
 * @param type area object type
 * @return the group, or nullptr if the scene has none
 */
al::AreaObjGroup* tryFindAreaObjGroup(const al::IUseAreaObj* pAreaUser, AreaObjType type) {
    s32 index = sAreaObjIndex[type];

    if (index < 0) {
        return nullptr;
    }

    return pAreaUser->getAreaObjDirector()->mAreaGroups[index];
}

/**
 * Caches the area group index of every area object type.
 * @param pDirector area object director of the scene
 */
void initAreaObjIndex(al::AreaObjDirector* pDirector) {
    memset(sAreaObjIndex, -1, sizeof(sAreaObjIndex));

    for (s32 i = 0; i < AreaObjType::size(); i++) {
        sAreaObjIndex[i] = pDirector->getAreaObjGroupIndex(AreaObjType::text(i));
    }
}

/**
 * Gets the cached area group index of an area object type.
 * @param type area object type
 * @return the group index, or -1 if the scene has no such group
 */
s32 getAreaObjIndex(AreaObjType type) {
    return sAreaObjIndex[type];
}

/**
 * Finds the area of a type that contains a position.
 * @param pAreaUser area user
 * @param type area object type
 * @param rPos position to check
 * @param pAreaObj output area
 * @return the number of areas containing the position
 */
s32 tryFindSingleAreaObj(const al::IUseAreaObj* pAreaUser, AreaObjType type,
                         const sead::Vector3f& rPos, al::AreaObj** pAreaObj) {
    al::AreaObjGroup* group = tryFindAreaObjGroup(pAreaUser, type);

    if (group == nullptr) {
        return 0;
    }

    return group->getInVolumeAreaObj(rPos, pAreaObj);
}

/**
 * Finds an area of a type that contains a position.
 * @param pAreaUser area user
 * @param type area object type
 * @param rPos position to check
 * @return the area, or nullptr if none contains the position
 */
al::AreaObj* tryFindAreaObj(const al::IUseAreaObj* pAreaUser, AreaObjType type,
                            const sead::Vector3f& rPos) {
    al::AreaObjGroup* group = tryFindAreaObjGroup(pAreaUser, type);

    if (group == nullptr) {
        return nullptr;
    }

    return group->getInVolumeAreaObj(rPos);
}

/**
 * Finds an area of a type that is hit by a line segment.
 * @param pAreaUser area user
 * @param type area object type
 * @param rStart start of the segment
 * @param rEnd end of the segment
 * @param pHitPos output hit position
 * @param pNormal output hit normal
 * @return the area, or nullptr if none is hit
 */
al::AreaObj* tryFindAreaObj(const al::IUseAreaObj* pAreaUser, AreaObjType type,
                            const sead::Vector3f& rStart, const sead::Vector3f& rEnd,
                            sead::Vector3f* pHitPos, sead::Vector3f* pNormal) {
    al::AreaObjGroup* group = tryFindAreaObjGroup(pAreaUser, type);

    if (group == nullptr) {
        return nullptr;
    }

    return group->getInVolumeAreaObj(rStart, rEnd, pHitPos, pNormal);
}

/**
 * Checks whether an area of a type contains a position.
 * @param pAreaUser area user
 * @param type area object type
 * @param rPos position to check
 * @return true if an area contains the position
 */
bool isInAreaObj(const al::IUseAreaObj* pAreaUser, AreaObjType type, const sead::Vector3f& rPos) {
    return tryFindAreaObj(pAreaUser, type, rPos) != nullptr;
}

/**
 * Checks whether an area of a type is hit by a line segment.
 * @param pAreaUser area user
 * @param type area object type
 * @param rStart start of the segment
 * @param rEnd end of the segment
 * @param pHitPos output hit position
 * @param pNormal output hit normal
 * @return true if an area is hit
 */
bool isInAreaObj(const al::IUseAreaObj* pAreaUser, AreaObjType type,
                 const sead::Vector3f& rStart, const sead::Vector3f& rEnd,
                 sead::Vector3f* pHitPos, sead::Vector3f* pNormal) {
    return tryFindAreaObj(pAreaUser, type, rStart, rEnd, pHitPos, pNormal) != nullptr;
}

/**
 * Checks whether an area target actor is inside an area of a type.
 * @param pActor actor to check
 * @param type area object type
 * @return true if the actor is an area target inside an area
 */
bool isInAreaObj(const al::LiveActor* pActor, AreaObjType type) {
    if (!al::isAreaTarget(pActor)) {
        return false;
    }

    return isInAreaObj(pActor, type, al::getTrans(pActor));
}

/**
 * Checks whether any living player is inside an area of a type.
 * @param pAreaUser area user
 * @param type area object type
 * @param pPlayerHolder player holder
 * @return true if a player is inside an area
 */
bool isInAreaObjPlayerOne(const al::IUseAreaObj* pAreaUser, AreaObjType type,
                          const al::PlayerHolder* pPlayerHolder) {
    s32 num = al::getPlayerNumMax(pPlayerHolder);

    for (s32 i = 0; i < num; i++) {
        al::LiveActor* player = al::getPlayerActor(pPlayerHolder, i);

        if (al::isDead(player) || !al::isAreaTarget(player)) {
            continue;
        }

        if (isInAreaObj(pAreaUser, type, al::getTrans(player))) {
            return true;
        }
    }

    return false;
}

/**
 * Checks whether all living players are inside an area of a type.
 * @param pAreaUser area user
 * @param type area object type
 * @param pPlayerHolder player holder
 * @return true if no living player is outside the areas
 */
bool isInAreaObjPlayerAll(const al::IUseAreaObj* pAreaUser, AreaObjType type,
                          const al::PlayerHolder* pPlayerHolder) {
    s32 num = al::getPlayerNumMax(pPlayerHolder);

    for (s32 i = 0; i < num; i++) {
        al::LiveActor* player = al::getPlayerActor(pPlayerHolder, i);

        if (al::isDead(player) || !al::isAreaTarget(player)) {
            continue;
        }

        if (!isInAreaObj(pAreaUser, type, al::getTrans(player))) {
            return false;
        }
    }

    return true;
}

/**
 * Checks whether a position is inside a death area.
 * @param pAreaUser area user
 * @param rPos position to check
 * @return true if a death area contains the position
 */
bool isInDeathArea(const al::IUseAreaObj* pAreaUser, const sead::Vector3f& rPos) {
    return isInAreaObj(pAreaUser, AreaObjType::DeathArea, rPos);
}

/**
 * Checks whether an area target actor is inside a death area.
 * @param pActor actor to check
 * @return true if the actor is inside a death area
 */
bool isInDeathArea(const al::LiveActor* pActor) {
    if (!al::isAreaTarget(pActor)) {
        return false;
    }

    return isInDeathArea(pActor, al::getTrans(pActor));
}

/**
 * Checks whether a position is inside a death area, also counting no death areas
 * that are not the only area found.
 * @param pAreaUser area user
 * @param rPos position to check
 * @return true if the position counts as inside a death area
 */
bool isInDeathAreaWithNoDeathCheck(const al::IUseAreaObj* pAreaUser, const sead::Vector3f& rPos) {
    if (isInDeathArea(pAreaUser, rPos)) {
        return true;
    }

    al::AreaObj* noDeathArea = nullptr;
    s32 num = tryFindSingleAreaObj(pAreaUser, AreaObjType::NoDeathArea, rPos, &noDeathArea);

    return num > 0 && noDeathArea == nullptr;
}

/**
 * Checks whether an area target actor is inside a death area, with the no death area check.
 * @param pActor actor to check
 * @return true if the actor counts as inside a death area
 */
bool isInDeathAreaWithNoDeathCheck(const al::LiveActor* pActor) {
    if (!al::isAreaTarget(pActor)) {
        return false;
    }

    return isInDeathAreaWithNoDeathCheck(pActor, al::getTrans(pActor));
}

/**
 * Checks whether a position is inside a water area and not inside a no water area.
 * @param pAreaUser area user
 * @param rPos position to check
 * @return true if the position is in water
 */
bool isInWaterArea(const al::IUseAreaObj* pAreaUser, const sead::Vector3f& rPos) {
    if (isInAreaObj(pAreaUser, AreaObjType::NoWaterArea, rPos)) {
        return false;
    }

    return isInAreaObj(pAreaUser, AreaObjType::WaterArea, rPos);
}

/**
 * Checks whether a line segment hits a water area.
 * @param pAreaUser area user
 * @param rStart start of the segment
 * @param rEnd end of the segment
 * @param pHitPos output hit position
 * @param pNormal output hit normal
 * @return true if a water area is hit
 */
bool isInWaterArea(const al::IUseAreaObj* pAreaUser, const sead::Vector3f& rStart,
                   const sead::Vector3f& rEnd, sead::Vector3f* pHitPos, sead::Vector3f* pNormal) {
    return isInAreaObj(pAreaUser, AreaObjType::WaterArea, rStart, rEnd, pHitPos, pNormal);
}

/**
 * Checks whether an area target actor is in water.
 * @param pActor actor to check
 * @return true if the actor is in water
 */
bool isInWaterArea(const al::LiveActor* pActor) {
    if (!al::isAreaTarget(pActor)) {
        return false;
    }

    return isInWaterArea(pActor, al::getTrans(pActor));
}

/**
 * Checks whether a point above or below an area target actor is in water.
 * @param pActor actor to check
 * @param offsetY vertical offset from the actor's position
 * @return true if the offset position is in water
 */
bool isInWaterArea(const al::LiveActor* pActor, f32 offsetY) {
    if (!al::isAreaTarget(pActor)) {
        return false;
    }

    sead::Vector3f pos = al::getTrans(pActor) + sead::Vector3f(0.0f, offsetY, 0.0f);

    return isInWaterArea(pActor, pos);
}

/**
 * Checks whether a position is inside a water area flagged as no sink ocean.
 * @param pAreaUser area user
 * @param rPos position to check
 * @return true if the position is in no sink water
 */
bool isInWaterAreaNoSink(const al::IUseAreaObj* pAreaUser, const sead::Vector3f& rPos) {
    if (isInAreaObj(pAreaUser, AreaObjType::NoWaterArea, rPos)) {
        return false;
    }

    al::AreaObj* areaObj = tryFindAreaObj(pAreaUser, AreaObjType::WaterArea, rPos);

    return areaObj != nullptr && areaObj->mIsNoSinkOcean;
}

/**
 * Checks whether an area target actor is in no sink water.
 * @param pActor actor to check
 * @return true if the actor is in no sink water
 */
bool isInWaterAreaNoSink(const al::LiveActor* pActor) {
    if (!al::isAreaTarget(pActor)) {
        return false;
    }

    return isInWaterAreaNoSink(pActor, al::getTrans(pActor));
}

/**
 * Calculates how deep a position is below the water surface.
 * @param pAreaUser area user
 * @param rPos position to check
 * @return the depth, or -1.0f if the position is not in water
 */
f32 calcWaterSinkDepth(const al::IUseAreaObj* pAreaUser, const sead::Vector3f& rPos) {
    if (isInAreaObj(pAreaUser, AreaObjType::NoWaterArea, rPos)) {
        return -1.0f;
    }

    al::AreaObj* areaObj = tryFindAreaObj(pAreaUser, AreaObjType::WaterArea, rPos);

    if (areaObj == nullptr) {
        return -1.0f;
    }

    sead::Vector3f hitPos;
    sead::Vector3f normal;
    sead::Vector3f above = {rPos.x, rPos.y + 100000.0f, rPos.z};

    if (!areaObj->getAreaShape()->checkArrowCollision(&hitPos, &normal, rPos, above)) {
        return -1.0f;
    }

    return hitPos.y - rPos.y;
}

/**
 * Calculates how deep an actor is below the water surface.
 * @param pActor actor to check
 * @return the depth, or -1.0f if the actor is not in water
 */
f32 calcWaterSinkDepth(const al::LiveActor* pActor) {
    return calcWaterSinkDepth(pActor, al::getTrans(pActor));
}

/**
 * Checks whether a position is inside a player control off area.
 * @param pAreaUser area user
 * @param rPos position to check
 * @return true if such an area contains the position
 */
bool isInPlayerControlOffArea(const al::IUseAreaObj* pAreaUser, const sead::Vector3f& rPos) {
    return isInAreaObj(pAreaUser, AreaObjType::PlayerControlOffArea, rPos);
}

/**
 * Checks whether an area target actor is inside a player control off area.
 * @param pActor actor to check
 * @return true if the actor is inside such an area
 */
bool isInPlayerControlOffArea(const al::LiveActor* pActor) {
    if (!al::isAreaTarget(pActor)) {
        return false;
    }

    return isInPlayerControlOffArea(pActor, al::getTrans(pActor));
}

/**
 * Checks whether the first area of a type contains a position.
 * @param pAreaUser area user
 * @param type area object type
 * @param rPos position to check
 * @return true if an area contains the position
 */
bool isInAreaObjInGroup(const al::IUseAreaObj* pAreaUser, AreaObjType type,
                        const sead::Vector3f& rPos) {
    al::AreaObjGroup* group = tryFindAreaObjGroup(pAreaUser, type);

    if (group == nullptr) {
        return false;
    }

    return group->getInFirstAreaObj(rPos) != nullptr;
}

/**
 * Checks whether a position is inside a no rain area.
 * @param pAreaUser area user
 * @param rPos position to check
 * @param isCamera whether the check is for the camera (areas that ignore the camera are skipped)
 * @return true if a no rain area applies at the position
 */
bool isInNoRainArea(const al::IUseAreaObj* pAreaUser, const sead::Vector3f& rPos,
                    bool isCamera) {
    NoRainArea* area =
        static_cast<NoRainArea*>(tryFindAreaObj(pAreaUser, AreaObjType::NoRainArea, rPos));

    if (area == nullptr) {
        return false;
    }

    if (isCamera && area->mIsIgnoreCamera) {
        return false;
    }

    return true;
}

/**
 * Checks whether a position is inside a graphics area flagged as a Plessie tunnel.
 * @param pAreaUser area user
 * @param rPos position to check
 * @return true if such an area contains the position
 */
bool isInPlessieTunnel(const al::IUseAreaObj* pAreaUser, const sead::Vector3f& rPos) {
    al::AreaObjGroup* group = tryFindAreaObjGroup(pAreaUser, AreaObjType::GraphicsArea);

    if (group == nullptr) {
        return false;
    }

    s32 num = group->getSize();

    for (s32 i = 0; i < num; i++) {
        al::AreaObj* areaObj = group->getAreaObj(i);

        if (areaObj->mIsPlessieTunnel && areaObj->isInVolume(rPos)) {
            return true;
        }
    }

    return false;
}

/**
 * Checks whether a position is inside an area enabling the Plessie chase V2 special camera.
 * Only active once the Dark Bowser V2 fight is available.
 * @param pActor actor whose scene is checked
 * @param rPos position to check
 * @return true if such an area contains the position
 */
bool isInPlessieChaseV2SpecialCamera(const al::LiveActor* pActor, const sead::Vector3f& rPos) {
    if (!SingleModeDataFunction::isDarkBowserV2Available(pActor)) {
        return false;
    }

    al::AreaObjGroup* group = tryFindAreaObjGroup(pActor, AreaObjType::SwitchKeepOnArea);

    if (group == nullptr) {
        return false;
    }

    s32 num = group->getSize();

    for (s32 i = 0; i < num; i++) {
        al::AreaObj* areaObj = group->getAreaObj(i);

        if (areaObj->mIsPlessieChaseV2SpecialCamera && areaObj->isInVolume(rPos)) {
            return true;
        }
    }

    return false;
}

/**
 * Updates an actor's water material code from whether it is in water.
 * @param pActor actor to update
 */
void updateMaterialCodeWater(al::LiveActor* pActor) {
    al::updateMaterialCodeWater(pActor, isInWaterArea(pActor), false);
}
}  // namespace rc
