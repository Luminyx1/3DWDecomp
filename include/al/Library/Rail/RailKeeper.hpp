#pragma once

#include <basis/seadTypes.h>

namespace al {
struct PlacementInfo;
class Rail;
class RailRider;

class RailKeeper {
public:
    RailKeeper(const PlacementInfo& rInfo);

    bool isValid() const;

    Rail* getRail() const { return mRail; }
    RailRider* getRailRider() const { return mRailRider; }

private:
    Rail* mRail = nullptr;
    RailRider* mRailRider = nullptr;
};

class RailKeeperGroup {
public:
    RailKeeperGroup();

    void init(const PlacementInfo& rInfo, const char* pLinkName);
    RailKeeper* getRailKeeper(s32 index) const;

    s32 getRailKeeperNum() const { return mRailKeeperNum; }

private:
    RailKeeper** mRailKeepers = nullptr;
    s32 mRailKeeperNum = 0;
};

RailKeeper* tryCreateRailKeeper(const PlacementInfo& rInfo, const char* pLinkName);
RailKeeperGroup* tryCreateRailKeeperGroup(const PlacementInfo& rInfo, const char* pLinkName);
}  // namespace al
