#include "Project/AreaObj/AreaObjGroup.hpp"

#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
/**
 * Constructs an empty area group.
 * @param pGroupName name of the group
 */
AreaObjGroup::AreaObjGroup(const char* pGroupName)
    : mGroupName(pGroupName) {}

/**
 * Constructs an area group from the areas linked to an actor.
 * @param pGroupName name of the group
 * @param pLinkName link name of the areas
 * @param rInfo actor init info
 */
AreaObjGroup::AreaObjGroup(const char* pGroupName, const char* pLinkName,
                           const ActorInitInfo& rInfo)
    : AreaObjGroup(pGroupName) {
    AreaInitInfo areaInitInfo;
    s32 num = calcLinkChildNum(rInfo, pLinkName);
    mMaxAreas = num;
    if (num < 1) {
        return;
    }

    createBuffer();
    for (s32 i = 0; i < num; i++) {
        PlacementInfo placementInfo;
        getLinksInfoByIndex(&placementInfo, rInfo, pLinkName, i);
        areaInitInfo.set(placementInfo, rInfo.mStageSwitchDirector);
        AreaObj* areaObj = new AreaObj("name");
        areaObj->init(areaInitInfo);
        resisterAreaObj(areaObj);
    }
}

/**
 * Sets the capacity and allocates the area buffer.
 * @param maxAreas capacity of the group
 */
void AreaObjGroup::createBuffer(s32 maxAreas) {
    mMaxAreas = maxAreas;
    createBuffer();
}

/**
 * Adds an area to the group if there is space left.
 * @param pAreaObj area to add
 */
void AreaObjGroup::resisterAreaObj(AreaObj* pAreaObj) {
    if (mNumAreas < mMaxAreas) {
        mAreaObjs[mNumAreas] = pAreaObj;
        mNumAreas++;
    }
}

/**
 * Deletes all areas of the group.
 */
AreaObjGroup::~AreaObjGroup() {
    for (s32 i = 0; i < mNumAreas; i++) {
        delete mAreaObjs[i];
    }

    delete[] mAreaObjs;
}

/**
 * Increases the capacity of the group by one.
 */
void AreaObjGroup::incrementCount() {
    mMaxAreas++;
}

/**
 * Allocates the area buffer for the current capacity.
 */
void AreaObjGroup::createBuffer() {
    if (mMaxAreas < 1) {
        return;
    }

    if (isEqualString(mGroupName, "CameraArea")) {
        mMaxAreas += 130;
    }

    mAreaObjs = new AreaObj*[mMaxAreas];
}

/**
 * Gets an area by index.
 * @param index index of the area
 * @return the area
 */
AreaObj* AreaObjGroup::getAreaObj(s32 index) const {
    return mAreaObjs[index];
}

/**
 * Finds the highest priority area containing a position.
 * @param rPos position to check
 * @return the area, or nullptr if none contains the position
 */
AreaObj* AreaObjGroup::getInVolumeAreaObj(const sead::Vector3f& rPos) {
    AreaObj* result = nullptr;
    for (s32 i = 0; i < mNumAreas; i++) {
        AreaObj* areaObj = mAreaObjs[i];
        if (!result || result->mPriority <= areaObj->mPriority) {
            if (areaObj->isInVolume(rPos)) {
                result = areaObj;
            }
        }
    }

    return result;
}

/**
 * Finds the highest priority area containing a position and counts the active candidates.
 * @param rPos position to check
 * @param pAreaObj output area, or nullptr if none contains the position
 * @return number of active areas that were checked
 */
s32 AreaObjGroup::getInVolumeAreaObj(const sead::Vector3f& rPos, AreaObj** pAreaObj) {
    s32 count = 0;
    AreaObj* result = nullptr;
    for (s32 i = 0; i < mNumAreas; i++) {
        AreaObj* areaObj = mAreaObjs[i];
        if (!result || result->mPriority <= areaObj->mPriority) {
            if (areaObj->mIsValid && !areaObj->mIsDisabled && areaObj->_66) {
                count++;
                if (areaObj->isInVolume(rPos)) {
                    result = areaObj;
                }
            }
        }
    }

    *pAreaObj = result;
    return count;
}

/**
 * Finds the highest priority area hit by a line segment.
 * @param rStart start of the segment
 * @param rEnd end of the segment
 * @param pHitPos output hit position
 * @param pNormal output hit normal
 * @return the area, or nullptr if none is hit
 */
AreaObj* AreaObjGroup::getInVolumeAreaObj(const sead::Vector3f& rStart, const sead::Vector3f& rEnd,
                                          sead::Vector3f* pHitPos, sead::Vector3f* pNormal) {
    AreaObj* result = nullptr;
    for (s32 i = 0; i < mNumAreas; i++) {
        AreaObj* areaObj = mAreaObjs[i];
        if (!result || result->mPriority <= areaObj->mPriority) {
            if (areaObj->isInVolume(rStart, rEnd, pHitPos, pNormal)) {
                result = areaObj;
            }
        }
    }

    return result;
}

/**
 * Finds the first area containing a position.
 * @param rPos position to check
 * @return the area, or nullptr if none contains the position
 */
AreaObj* AreaObjGroup::getInFirstAreaObj(const sead::Vector3f& rPos) {
    s32 num = mNumAreas;
    for (s32 i = 0; i < num; i++) {
        AreaObj* areaObj = mAreaObjs[i];
        if (areaObj && areaObj->isInVolume(rPos)) {
            return areaObj;
        }
    }

    return nullptr;
}
}  // namespace al
