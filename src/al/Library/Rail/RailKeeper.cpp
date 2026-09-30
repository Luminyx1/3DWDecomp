#include "Library/Rail/RailKeeper.hpp"

#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementInfo.hpp"
#include "Library/Rail/Rail.hpp"
#include "Library/Rail/RailRider.hpp"

namespace al {
void getLinksInfoByIndex(PlacementInfo* pInfo, const PlacementInfo& rParentInfo,
                         const char* pLinkName, s32 index);

RailKeeper::RailKeeper(const PlacementInfo& rInfo) {
    mRail = new Rail();
    mRail->init(rInfo);
    mRailRider = new RailRider(mRail);
}

/**
 * Checks whether the keeper holds a rail.
 * @return true if a rail exists
 */
bool RailKeeper::isValid() const {
    return mRail != nullptr;
}

RailKeeper* tryCreateRailKeeper(const PlacementInfo& rInfo, const char* pLinkName) {
    PlacementInfo railInfo;
    if (!tryGetLinksInfo(&railInfo, rInfo, pLinkName)) {
        return nullptr;
    }
    return new RailKeeper(railInfo);
}

RailKeeperGroup::RailKeeperGroup() = default;

void RailKeeperGroup::init(const PlacementInfo& rInfo, const char* pLinkName) {
    mRailKeeperNum = calcLinkChildNum(rInfo, pLinkName);
    if (mRailKeeperNum <= 0) {
        return;
    }

    mRailKeepers = new RailKeeper*[mRailKeeperNum];
    for (s32 i = 0; i < mRailKeeperNum; i++) {
        PlacementInfo railInfo;
        getLinksInfoByIndex(&railInfo, rInfo, pLinkName, i);
        mRailKeepers[i] = new RailKeeper(railInfo);
    }
}

/**
 * Gets a rail keeper of the group.
 * @param index the index of the rail keeper
 * @return the rail keeper
 */
RailKeeper* RailKeeperGroup::getRailKeeper(s32 index) const {
    return mRailKeepers[index];
}

RailKeeperGroup* tryCreateRailKeeperGroup(const PlacementInfo& rInfo, const char* pLinkName) {
    if (calcLinkChildNum(rInfo, pLinkName) == 0) {
        return nullptr;
    }
    RailKeeperGroup* group = new RailKeeperGroup();
    group->init(rInfo, pLinkName);
    return group;
}
}  // namespace al
