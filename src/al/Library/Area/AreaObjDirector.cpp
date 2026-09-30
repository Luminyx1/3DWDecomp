#include "Project/AreaObj/AreaObjDirector.hpp"

#include <cstring>

#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Scene/Scene.hpp"
#include "Project/AreaObj/AreaObj.hpp"
#include "Project/AreaObj/AreaObjFactory.hpp"
#include "Project/AreaObj/AreaObjGroup.hpp"
#include "Project/AreaObj/AreaObjMtxConnecter.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
/**
 * Constructs the area director.
 * @param isUseGrid whether to create an area grid
 */
AreaObjDirector::AreaObjDirector(bool isUseGrid) {
    if (isUseGrid) {
        mGrid = new AreaObjDirectorGrid(100, 100);
    } else {
        mGrid = nullptr;
    }

    for (s32 i = 0; i < 100; i++) {
        mExtraAreaObjs[i] = nullptr;
    }
}

/**
 * Initializes the director with the area factory.
 * @param pFactory factory used to create areas
 */
void AreaObjDirector::init(const AreaObjFactory* pFactory) {
    mFactory = pFactory;
    mMtxConnecterHolder = new AreaObjMtxConnecterHolder(0x100);
    s32 num = mFactory->getNumFactoryEntries();
    mAreaGroups = new AreaObjGroup*[num];

    for (s32 i = 0; i < num; i++) {
        mAreaGroups[i] = nullptr;
    }
}

/**
 * Finishes initialization.
 */
void AreaObjDirector::endInit() {}

/**
 * Updates the matrix connected areas.
 */
void AreaObjDirector::update() {
    if (mMtxConnecterHolder) {
        mMtxConnecterHolder->update();
    }
}

/**
 * Places all areas of a single placement list.
 * @param rInfo area init info of the placement list
 */
void AreaObjDirector::placement(const AreaInitInfo& rInfo) {
    placement(&rInfo, 1, nullptr, nullptr);
}

/**
 * Places all areas of several placement lists.
 * @param pInfos area init infos of the placement lists
 * @param num number of placement lists
 * @param pHolder scene object holder passed to areas that need it
 * @param pScene scene that validates the placements, or nullptr
 */
void AreaObjDirector::placement(const AreaInitInfo* pInfos, s32 num, const SceneObjHolder* pHolder,
                                Scene* pScene) {
    for (s32 i = 0; i < num; i++) {
        createAreaObjGroup(pInfos[i]);
    }

    createAreaObjGroupBuffer();

    for (s32 i = 0; i < num; i++) {
        placementAreaObj(pInfos[i], pHolder, pScene);
    }
}

/**
 * Creates the area groups for the areas of a placement list and counts their areas.
 * @param rInfo area init info of the placement list
 */
void AreaObjDirector::createAreaObjGroup(const AreaInitInfo& rInfo) {
    PlacementInfo placementInfo(rInfo.mPlacementInfo);
    s32 num = getCountPlacementInfo(placementInfo);

    for (s32 i = 0; i < num; i++) {
        PlacementInfo objInfo;
        tryGetPlacementInfoByIndex(&objInfo, placementInfo, i);
        const char* objName = nullptr;
        tryGetObjectName(&objName, objInfo);

        AreaCreatorFunction creator = nullptr;
        s32 index = mFactory->getEntryIndex(&creator, objName);

        if (!creator) {
            continue;
        }

        if (!mAreaGroups[index]) {
            mAreaGroups[index] = new AreaObjGroup(objName);
        }

        mAreaGroups[index]->incrementCount();
    }
}

/**
 * Allocates the buffers of all area groups and sorts the groups by name.
 */
void AreaObjDirector::createAreaObjGroupBuffer() {
    s32 count = 0;
    s32 num = mFactory->getNumFactoryEntries();

    for (s32 i = 0; i < num; i++) {
        if (!mAreaGroups[i]) {
            continue;
        }

        mAreaGroups[i]->createBuffer();
        count++;

        for (s32 j = i; j > 0; j--) {
            AreaObjGroup* prev = mAreaGroups[j - 1];

            if (prev && strcmp(mAreaGroups[j]->mGroupName, prev->mGroupName) >= 0) {
                break;
            }

            mAreaGroups[j - 1] = mAreaGroups[j];
            mAreaGroups[j] = prev;
        }
    }

    mAreaGroupCount = count;
}

/**
 * Creates and registers the areas of a placement list.
 * @param rInfo area init info of the placement list
 * @param pHolder scene object holder passed to areas that need it
 * @param pScene scene that validates the placements, or nullptr
 */
void AreaObjDirector::placementAreaObj(const AreaInitInfo& rInfo, const SceneObjHolder* pHolder,
                                       Scene* pScene) {
    PlacementInfo placementInfo(rInfo.mPlacementInfo);
    s32 num = getCountPlacementInfo(placementInfo);

    for (s32 i = 0; i < num; i++) {
        PlacementInfo objInfo;
        tryGetPlacementInfoByIndex(&objInfo, placementInfo, i);
        const char* objName = nullptr;
        tryGetObjectName(&objName, objInfo);

        AreaCreatorFunction creator = nullptr;
        mFactory->getEntryIndex(&creator, objName);

        if (!creator) {
            continue;
        }

        if (pScene && !pScene->isValidPlacement(objInfo)) {
            continue;
        }

        const char* displayName;
        getDisplayName(&displayName, objInfo);
        AreaObj* areaObj = creator(displayName);
        AreaInitInfo initInfo(objInfo, rInfo);

        if (isEqualString(displayName, "IslandArea") ||
            isEqualString(displayName, "DisasterModeArea")) {
            areaObj->init(initInfo, pHolder);
        } else {
            areaObj->init(initInfo);
        }

        getAreaObjGroup(objName)->resisterAreaObj(areaObj);
        mMtxConnecterHolder->tryAddArea(areaObj, objInfo);
    }
}

/**
 * Counts the areas of all groups.
 * @return total number of areas
 */
s32 AreaObjDirector::getTotalAreaObjs() const {
    s32 total = 0;

    for (s32 i = 0; i < mAreaGroupCount; i++) {
        total += mAreaGroups[i]->mNumAreas;
    }

    return total;
}

/**
 * Finds an area group by name.
 * @param pName name of the group
 * @return the group, or nullptr if it doesn't exist
 */
AreaObjGroup* AreaObjDirector::getAreaObjGroup(const char* pName) const {
    s32 index = getAreaObjGroupIndex(pName);

    if (index > -1) {
        return mAreaGroups[index];
    }

    return nullptr;
}

/**
 * Checks whether an area group exists.
 * @param pName name of the group
 * @return true if the group exists
 */
bool AreaObjDirector::isExistAreaGroup(const char* pName) {
    return getAreaObjGroup(pName) != nullptr;
}

/**
 * Adds an area to the first free slot of the extra area list.
 * @param pAreaObj area to add
 */
void AreaObjDirector::addToExtraAreaGroup(AreaObj* pAreaObj) {
    for (s32 i = 0; i < 100; i++) {
        if (!mExtraAreaObjs[i]) {
            mExtraAreaObjs[i] = pAreaObj;
            mExtraAreaObjCount++;
            return;
        }
    }
}

/**
 * Finds an extra area whose shape contains a position.
 * @param rPos position to check
 * @return the area, or nullptr if none contains the position
 */
AreaObj* AreaObjDirector::tryFindInExtraAreaObjGroup(const sead::Vector3f& rPos) {
    for (s32 i = 0; i < 100; i++) {
        if (mExtraAreaObjs[i] && mExtraAreaObjs[i]->isInVolumeCheck(rPos)) {
            return mExtraAreaObjs[i];
        }
    }

    return nullptr;
}

/**
 * Sets the enable flag of all areas.
 * @param isEnable new enable flag
 */
void AreaObjDirector::setEnableAll(bool isEnable) {
    for (s32 i = 0; i < mAreaGroupCount; i++) {
        AreaObjGroup* group = mAreaGroups[i];

        if (!group) {
            continue;
        }

        for (s32 j = 0; j < group->mNumAreas; j++) {
            group->getAreaObj(j)->_66 = isEnable;
        }
    }
}

/**
 * Finds the highest priority area of a group containing a position.
 * @param pName name of the group
 * @param rPos position to check
 * @return the area, or nullptr if none contains the position
 */
AreaObj* AreaObjDirector::getInVolumeAreaObj(const char* pName, const sead::Vector3f& rPos) {
    AreaObjGroup* group = getAreaObjGroup(pName);

    if (!group) {
        return nullptr;
    }

    return group->getInVolumeAreaObj(rPos);
}

/**
 * Finds the highest priority area of a group hit by a line segment.
 * @param pName name of the group
 * @param rStart start of the segment
 * @param rEnd end of the segment
 * @param pHitPos output hit position
 * @param pNormal output hit normal
 * @return the area, or nullptr if none is hit
 */
AreaObj* AreaObjDirector::getInVolumeAreaObj(const char* pName, const sead::Vector3f& rStart,
                                             const sead::Vector3f& rEnd, sead::Vector3f* pHitPos,
                                             sead::Vector3f* pNormal) {
    AreaObjGroup* group = getAreaObjGroup(pName);

    if (!group) {
        return nullptr;
    }

    return group->getInVolumeAreaObj(rStart, rEnd, pHitPos, pNormal);
}

/**
 * Gets the holder of the matrix connected areas.
 * @return the holder
 */
AreaObjMtxConnecterHolder* AreaObjDirector::getMtxConnecterHolder() const {
    return mMtxConnecterHolder;
}

/**
 * Finds the index of an area group by binary search over the sorted group names.
 * @param pName name of the group
 * @return index of the group, or -1 if it doesn't exist
 */
s32 AreaObjDirector::getAreaObjGroupIndex(const char* pName) const {
    s32 lower = 0;
    s32 upper = mAreaGroupCount;

    while (lower < upper) {
        s32 mid = (lower + upper) / 2;
        s32 cmp = strcmp(pName, mAreaGroups[mid]->mGroupName);

        if (cmp == 0) {
            return mid;
        }

        if (cmp > 0) {
            lower = mid + 1;
        } else {
            upper = mid;
        }
    }

    return -1;
}

/**
 * Constructs an area grid.
 * @param sizeX number of cells along x
 * @param sizeZ number of cells along z
 */
AreaObjDirectorGrid::AreaObjDirectorGrid(s32 sizeX, s32 sizeZ) : _0(sizeX), _4(sizeZ) {}

/**
 * Expands the grid bounds to contain an area.
 * @param pAreaObj area to contain
 */
void AreaObjDirectorGrid::expandGrid(AreaObj* pAreaObj) {}

/**
 * Finishes initialization of the grid.
 */
void AreaObjDirectorGrid::endInit() {}
}  // namespace al
