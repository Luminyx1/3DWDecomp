#include "Library/StageSwitch/Core/StageSwitchKeeper.hpp"

#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementId.hpp"
#include "Library/Play/Placement/PlacementInfo.hpp"
#include "Library/StageSwitch/Core/StageSwitchDirector.hpp"
#include "Library/StageSwitch/StageSwitchAccesser.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
/**
 * Constructs an empty switch info.
 */
StageSwitchInfo::StageSwitchInfo() : mPlacementId(nullptr), mIsOn(false) {}

/**
 * Constructs an empty accesser list node.
 */
StageSwitchAccesserList::StageSwitchAccesserList() = default;

/**
 * Constructs an accesser list node.
 * @param pAccesser accesser of the node
 */
StageSwitchAccesserList::StageSwitchAccesserList(const StageSwitchAccesser* pAccesser)
    : mAccesser(pAccesser) {}

/**
 * Constructs an empty stage switch keeper.
 */
StageSwitchKeeper::StageSwitchKeeper() = default;

/**
 * Creates an accesser for every StageSwitch link of a placement.
 * @param pDirector stage switch director
 * @param rInfo placement info
 */
void StageSwitchKeeper::init(StageSwitchDirector* pDirector, const PlacementInfo& rInfo) {
    mLinkCount = calcLinkCountClassName(rInfo, "StageSwitch");
    mAccessors = new StageSwitchAccesser[mLinkCount];

    PlacementInfo links;
    tryGetPlacementInfoByKey(&links, rInfo, "Links");

    s32 linkNum = getCountPlacementInfo(links);
    for (s32 index = 0, i = 0; i < linkNum; i++) {
        PlacementInfo link;
        const char* linkName = nullptr;
        tryGetPlacementInfoAndKeyNameByIndex(&link, &linkName, links, i);
        PlacementInfo linkData;
        tryGetPlacementInfoByIndex(&linkData, link, 0);
        if (isClassName(linkData, "StageSwitch")) {
            PlacementId placementId;
            tryGetPlacementID(&placementId, linkData);
            bool isDisasterMode = false;
            tryGetArg(&isDisasterMode, linkData, "IsDisasterCameraOn");
            mAccessors[index].setUseName(mUseName);
            mAccessors[index].init(pDirector, linkName, placementId, isDisasterMode);
            index++;
        }
    }
}

/**
 * Finds the accesser of a link.
 * @param pLinkName link name
 * @return accesser, or null if not found
 */
StageSwitchAccesser* StageSwitchKeeper::tryGetStageSwitchAccesser(const char* pLinkName) const {
    for (s32 i = 0; i < mLinkCount; i++) {
        if (isEqualString(pLinkName, mAccessors[i].getLinkName())) {
            return &mAccessors[i];
        }
    }

    return nullptr;
}

/**
 * Checks whether any accesser uses a switch.
 * @param switchNo switch number
 * @return true if the switch is used
 */
bool StageSwitchKeeper::isUsingSwitchNo(s32 switchNo) {
    for (s32 i = 0; i < mLinkCount; i++) {
        if (mAccessors[i].getSwitchNo() == switchNo) {
            return true;
        }
    }

    return false;
}
}  // namespace al
