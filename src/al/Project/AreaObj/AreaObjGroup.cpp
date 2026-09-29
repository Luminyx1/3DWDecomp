#include "Project/AreaObj/AreaObjGroup.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
    /**
     * @brief Constructs an empty group.
     * @param pGroupName The name of the group.
     */
    AreaObjGroup::AreaObjGroup(const char* pGroupName)
        : mGroupName(pGroupName), mAreaObjs(nullptr), mNumAreas(0), mMaxAreas(0) {}

    /**
     * @brief Constructs a group from the areas linked to an actor.
     * @param pGroupName The name of the group.
     * @param pLinkName The name of the link to the areas.
     * @param rInfo The actor's init info.
     */
    AreaObjGroup::AreaObjGroup(const char* pGroupName, const char* pLinkName, const ActorInitInfo& rInfo)
        : mGroupName(pGroupName), mAreaObjs(nullptr), mNumAreas(0), mMaxAreas(0) {
        AreaInitInfo areaInitInfo;
        s32 linkNum = calcLinkChildNum(rInfo, pLinkName);
        createBuffer(linkNum);

        for (s32 i = 0; i < linkNum; i++) {
            PlacementInfo placementInfo;
            getLinksInfoByIndex(&placementInfo, rInfo, pLinkName, i);
            areaInitInfo.set(placementInfo, rInfo.mStageSwitchDirector);

            AreaObj* areaObj = new AreaObj("name");
            areaObj->init(areaInitInfo);
            resisterAreaObj(areaObj);
        }
    }

    /**
     * @brief Allocates the area buffer.
     * @param size The number of areas the group can hold.
     */
    void AreaObjGroup::createBuffer(s32 size) {
        mMaxAreas = size;
        if (size < 1) {
            return;
        }

        if (isEqualString(mGroupName, "CameraArea")) {
            mMaxAreas += 130;
        }

        mAreaObjs = new AreaObj*[mMaxAreas];
    }

    /**
     * @brief Adds an area to the group if there is room left.
     * @param pAreaObj The area to add.
     */
    void AreaObjGroup::resisterAreaObj(AreaObj* pAreaObj) {
        if (mNumAreas < mMaxAreas) {
            mAreaObjs[mNumAreas] = pAreaObj;
            mNumAreas++;
        }
    }

    /** @brief Destroys the group and all of its areas. */
    AreaObjGroup::~AreaObjGroup() {
        for (s32 i = 0; i < mNumAreas; i++) {
            if (mAreaObjs[i] != nullptr) {
                delete mAreaObjs[i];
            }
        }

        delete[] mAreaObjs;
    }

    /** @brief Increases the number of areas the buffer will be created for. */
    void AreaObjGroup::incrementCount() {
        mMaxAreas++;
    }

    /** @brief Allocates the area buffer for the counted number of areas. */
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
     * @brief Gets an area of the group.
     * @param index The index of the area.
     * @return The area.
     */
    AreaObj* AreaObjGroup::getAreaObj(s32 index) const {
        return mAreaObjs[index];
    }

    /**
     * @brief Finds the highest priority area that contains a position.
     * @param rPos The position to check.
     * @return The area, or nullptr if no area contains the position.
     */
    AreaObj* AreaObjGroup::getInVolumeAreaObj(const sead::Vector3f& rPos) {
        AreaObj* result = nullptr;
        for (s32 i = 0; i < mNumAreas; i++) {
            AreaObj* areaObj = mAreaObjs[i];
            if (result != nullptr && result->mPriority > areaObj->mPriority) {
                continue;
            }

            if (areaObj->isInVolume(rPos)) {
                result = areaObj;
            }
        }

        return result;
    }

    /**
     * @brief Finds the highest priority valid area that contains a position.
     * @param rPos The position to check.
     * @param pAreaObj Receives the area, or nullptr if no area contains the position.
     * @return The number of valid areas that were checked.
     */
    s32 AreaObjGroup::getInVolumeAreaObj(const sead::Vector3f& rPos, AreaObj** pAreaObj) {
        s32 checkedNum = 0;
        AreaObj* result = nullptr;
        for (s32 i = 0; i < mNumAreas; i++) {
            AreaObj* areaObj = mAreaObjs[i];
            if (result != nullptr && result->mPriority > areaObj->mPriority) {
                continue;
            }

            if (!areaObj->mIsValid || areaObj->mIsDisabled || !areaObj->_66) {
                continue;
            }

            checkedNum++;
            if (areaObj->isInVolume(rPos)) {
                result = areaObj;
            }
        }

        *pAreaObj = result;
        return checkedNum;
    }

    /**
     * @brief Finds the highest priority area that a line segment enters.
     * @param rStart The start of the segment.
     * @param rEnd The end of the segment.
     * @param pHitPos Receives the entry position.
     * @param pHitNormal Receives the normal at the entry position.
     * @return The area, or nullptr if the segment enters no area.
     */
    AreaObj* AreaObjGroup::getInVolumeAreaObj(const sead::Vector3f& rStart, const sead::Vector3f& rEnd,
                                              sead::Vector3f* pHitPos, sead::Vector3f* pHitNormal) {
        AreaObj* result = nullptr;
        for (s32 i = 0; i < mNumAreas; i++) {
            AreaObj* areaObj = mAreaObjs[i];
            if (result != nullptr && result->mPriority > areaObj->mPriority) {
                continue;
            }

            if (areaObj->isInVolume(rStart, rEnd, pHitPos, pHitNormal)) {
                result = areaObj;
            }
        }

        return result;
    }

    /**
     * @brief Finds the first area that contains a position.
     * @param rPos The position to check.
     * @return The area, or nullptr if no area contains the position.
     */
    AreaObj* AreaObjGroup::getInFirstAreaObj(const sead::Vector3f& rPos) {
        s32 areaNum = mNumAreas;
        for (s32 i = 0; i < areaNum; i++) {
            AreaObj* areaObj = mAreaObjs[i];
            if (areaObj != nullptr && areaObj->isInVolume(rPos)) {
                return areaObj;
            }
        }

        return nullptr;
    }
};
