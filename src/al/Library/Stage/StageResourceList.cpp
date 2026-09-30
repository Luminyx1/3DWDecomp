#include "Library/Stage/StageResourceList.hpp"

#include "Library/File/FileUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementInfo.hpp"
#include "Library/Resource/Resource.hpp"
#include "Library/Resource/ResourceFunction.hpp"
#include "Library/Scene/SceneUtil.hpp"
#include "Library/Stage/StageInfo.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
/**
 * Gets the placement data of the stage.
 * @return the placement data
 */
const ByamlIter& StageInfo::getPlacementIter() const {
    return mPlacementInfo->getPlacementIter();
}

/**
 * Gets the placement data of the zone in its parent.
 * @return the zone placement data
 */
const ByamlIter& StageInfo::getZoneIter() const {
    return mPlacementInfo->getZoneIter();
}

/**
 * Gets the placement info of the parent zone.
 * @return the parent placement info, or nullptr
 */
PlacementInfo* StageInfo::getParentInfo() const {
    return mPlacementInfo->_20;
}

/**
 * Gets the zone id.
 * @return the zone id
 */
s32 StageInfo::getID() const {
    return mPlacementInfo->_28;
}

static bool tryGetZoneListPlacementInfo(PlacementInfo* pOut, const char* pStageName,
                                        const char* pListName, s32 scenarioNo,
                                        bool isOneResource) {
    StringTmp<128> archivePath;
    makeStageDataArchivePath(&archivePath, pStageName, scenarioNo, "Map", isOneResource);
    Resource* resource = findOrCreateResource(archivePath, nullptr);
    if (!resource) {
        return false;
    }
    StringTmp<256> fileName("%s%s", pStageName, "Map");
    return tryGetPlacementInfo(pOut, resource, fileName.cstr(), pListName);
}

static s32 calcZoneNumRecursive(const char* pStageName, const char* pListName, s32 scenarioNo,
                                bool isOneResource) {
    PlacementInfo listInfo;
    if (!tryGetZoneListPlacementInfo(&listInfo, pStageName, pListName, scenarioNo,
                                     isOneResource)) {
        return 0;
    }
    s32 num = getCountPlacementInfo(listInfo);
    s32 zoneNum = 0;
    for (s32 i = 0; i < num; i++) {
        PlacementInfo zoneInfo;
        if (!tryGetPlacementInfoByIndex(&zoneInfo, listInfo, i)) {
            continue;
        }
        const char* zoneName = nullptr;
        if (!tryGetObjectName(&zoneName, zoneInfo)) {
            continue;
        }
        zoneNum += calcZoneNumRecursive(zoneName, "ZoneList", scenarioNo, isOneResource) + 1;
    }
    return zoneNum;
}

/**
 * Loads the stage data of a stage and all of its zones.
 * @param pStageName name of the stage
 * @param scenarioNo scenario number
 * @param pResourceType type of stage data to load
 * @param isOneResource whether the stage data is a single archive for all scenarios
 */
StageResourceList::StageResourceList(const char* pStageName, s32 scenarioNo,
                                     const char* pResourceType, bool isOneResource)
    : mIsOneResource(isOneResource) {
    StringTmp<128> archivePath;
    makeStageDataArchivePath(&archivePath, pStageName, scenarioNo, pResourceType,
                             mIsOneResource);
    bool isExist = isExistArchive(archivePath);
    if (!isExist && isEqualString(pResourceType, "Map")) {
        return;
    }

    StringTmp<128> mapArchivePath;
    makeStageDataArchivePath(&mapArchivePath, pStageName, scenarioNo, "Map", mIsOneResource);
    Resource* mapResource = findOrCreateResource(mapArchivePath, nullptr);
    PlacementInfo zoneListInfo;
    StringTmp<256> mapFileName("%s%s", pStageName, "Map");
    s32 zoneNum = 0;
    if (tryGetPlacementInfo(&zoneListInfo, mapResource, mapFileName.cstr(), "ZoneList")) {
        zoneNum = getCountPlacementInfo(zoneListInfo);
    }
    s32 islandNum = calcZoneNumRecursive(pStageName, "IslandList", scenarioNo, mIsOneResource);
    mStageInfos.allocBuffer(zoneNum + islandNum + 1, nullptr);

    if (isExist) {
        Resource* resource = findOrCreateResource(archivePath, nullptr);
        const u8* byml =
            resource->tryGetByml(StringTmp<256>("%s%s", pStageName, pResourceType));
        if (byml) {
            ByamlIter placementIter(byml);
            ByamlIter zoneIter;
            s32 id = mLastZoneId++;
            mStageInfos.pushBack(
                new StageInfo(resource, placementIter, zoneIter, pStageName, nullptr, id));
        }
    }

    for (s32 i = 0; i < zoneNum; i++) {
        PlacementInfo zoneInfo;
        tryGetPlacementInfoByIndex(&zoneInfo, zoneListInfo, i);
        StageInfo* stageInfo = initZoneInfo(zoneInfo, scenarioNo, pResourceType, nullptr);
        if (stageInfo) {
            mStageInfos.pushBack(stageInfo);
        }
    }

    if (islandNum != 0) {
        initZoneInfoRecursive(pStageName, "IslandList", scenarioNo, pResourceType, "ZoneList",
                              nullptr);
    }
}

/**
 * Loads the stage data of a zone.
 * @param rZoneInfo placement info of the zone
 * @param scenarioNo scenario number
 * @param pResourceType type of stage data to load
 * @param pParentInfo placement info of the parent zone, or nullptr
 * @return the stage info, or nullptr if the zone has no stage data
 */
StageInfo* StageResourceList::initZoneInfo(PlacementInfo& rZoneInfo, s32 scenarioNo,
                                           const char* pResourceType,
                                           PlacementInfo* pParentInfo) {
    StringTmp<128> archivePath;
    const char* zoneName = nullptr;
    getObjectName(&zoneName, rZoneInfo);
    makeStageDataArchivePath(&archivePath, zoneName, scenarioNo, pResourceType, mIsOneResource);
    StageInfo* stageInfo = nullptr;
    if (!isExistArchive(archivePath)) {
        return stageInfo;
    }
    Resource* resource = findOrCreateResource(archivePath, nullptr);
    const u8* byml = resource->tryGetByml(StringTmp<256>("%s%s", zoneName, pResourceType));
    if (!byml) {
        return nullptr;
    }
    s32 id = mLastZoneId++;
    stageInfo = new StageInfo(resource, ByamlIter(byml), rZoneInfo.getPlacementIter(), zoneName,
                              pParentInfo, id);
    sead::Vector3f rotate = sead::Vector3f::ones;
    sead::Vector3f scale = sead::Vector3f::zero;
    tryGetRotate(&rotate, rZoneInfo);
    tryGetScale(&scale, rZoneInfo);
    return stageInfo;
}

/**
 * Loads the stage data of the zones of a zone list and their child zones.
 * @param pStageName name of the stage owning the list
 * @param pListName name of the zone list
 * @param scenarioNo scenario number
 * @param pResourceType type of stage data to load
 * @param pChildListName name of the zone lists of the child zones
 * @param pParentInfo placement info of the parent zone, or nullptr
 */
void StageResourceList::initZoneInfoRecursive(const char* pStageName, const char* pListName,
                                              s32 scenarioNo, const char* pResourceType,
                                              const char* pChildListName,
                                              PlacementInfo* pParentInfo) {
    PlacementInfo listInfo;
    if (!tryGetZoneListPlacementInfo(&listInfo, pStageName, pListName, scenarioNo,
                                     mIsOneResource)) {
        return;
    }
    s32 num = getCountPlacementInfo(listInfo);
    for (s32 i = 0; i < num; i++) {
        PlacementInfo zoneInfo;
        if (!tryGetPlacementInfoByIndex(&zoneInfo, listInfo, i)) {
            continue;
        }
        StageInfo* stageInfo = initZoneInfo(zoneInfo, scenarioNo, pResourceType, pParentInfo);
        if (!stageInfo) {
            continue;
        }
        mStageInfos.pushBack(stageInfo);
        initZoneInfoRecursive(stageInfo->mName.cstr(), pChildListName, scenarioNo, pResourceType,
                              pChildListName, stageInfo->mPlacementInfo);
    }
}

/**
 * Gets the number of loaded stage infos.
 * @return the number of stage infos
 */
s32 StageResourceList::getStageResourceNum() const {
    return mStageInfos.size();
}

/**
 * Gets a stage info by index.
 * @param index index of the stage info
 * @return the stage info, or nullptr if the index is out of range
 */
StageInfo* StageResourceList::getStageInfo(s32 index) const {
    return mStageInfos.at(index);
}

/**
 * Finds a stage info by name.
 * @param pName name of the stage
 * @return the stage info, or nullptr if it doesn't exist
 */
StageInfo* StageResourceList::findStageInfo(const char* pName) const {
    s32 num = mStageInfos.size();
    for (s32 i = 0; i < num; i++) {
        StageInfo* stageInfo = mStageInfos.at(i);
        if (isEqualString(stageInfo->mName, sead::SafeString(pName))) {
            return stageInfo;
        }
    }
    return nullptr;
}
}  // namespace al
