#include "Library/Rail/RailKeeper.hpp"

#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementInfo.hpp"
#include "Library/Rail/Rail.hpp"
#include "Library/Rail/RailRider.hpp"

namespace al {

/**
 * @brief Creates the rail described by a placement and a rider on it.
 * @param rInfo The rail placement info.
 */
RailKeeper::RailKeeper(const PlacementInfo& rInfo) {
    mRail = new Rail();
    mRail->init(rInfo);
    mRailRider = new RailRider(mRail);
}

/**
 * @brief Checks whether a rail was created.
 * @return True if the keeper has a rail.
 */
bool RailKeeper::isValid() const {
    return mRail != nullptr;
}

/**
 * @brief Creates a rail keeper from a linked rail, if there is one.
 * @param rInfo The placement info of the owner.
 * @param pLinkName The link name of the rail.
 * @return The new keeper, or null if no rail is linked.
 */
RailKeeper* tryCreateRailKeeper(const PlacementInfo& rInfo, const char* pLinkName) {
    PlacementInfo linkInfo;
    if (!tryGetLinksInfo(&linkInfo, rInfo, pLinkName)) {
        return nullptr;
    }

    return new RailKeeper(linkInfo);
}

/**
 * @brief Constructs an empty group.
 */
RailKeeperGroup::RailKeeperGroup() = default;

/**
 * @brief Creates a rail keeper for every linked rail.
 * @param rInfo The placement info of the owner.
 * @param pLinkName The link name of the rails.
 */
void RailKeeperGroup::init(const PlacementInfo& rInfo, const char* pLinkName) {
    mRailKeeperNum = calcLinkChildNum(rInfo, pLinkName);
    if (mRailKeeperNum <= 0) {
        return;
    }

    mRailKeepers = new RailKeeper*[mRailKeeperNum];
    for (s32 i = 0; i < mRailKeeperNum; i++) {
        PlacementInfo linkInfo;
        getLinksInfoByIndex(&linkInfo, rInfo, pLinkName, i);
        mRailKeepers[i] = new RailKeeper(linkInfo);
    }
}

/**
 * @brief Gets a rail keeper by index.
 * @param index The index.
 * @return The rail keeper.
 */
RailKeeper* RailKeeperGroup::getRailKeeper(s32 index) const {
    return mRailKeepers[index];
}

/**
 * @brief Creates a group of rail keepers from linked rails, if there are any.
 * @param rInfo The placement info of the owner.
 * @param pLinkName The link name of the rails.
 * @return The new group, or null if no rail is linked.
 */
RailKeeperGroup* tryCreateRailKeeperGroup(const PlacementInfo& rInfo, const char* pLinkName) {
    if (calcLinkChildNum(rInfo, pLinkName) == 0) {
        return nullptr;
    }

    RailKeeperGroup* group = new RailKeeperGroup();
    group->init(rInfo, pLinkName);
    return group;
}

}  // namespace al
