#pragma once

#include <basis/seadTypes.h>

namespace al {
struct PlacementInfo;
class Rail;
class RailRider;

class RailKeeper {
public:
    RailKeeper(const PlacementInfo&);

    bool isValid() const;

    Rail* getRail() const { return mRail; }

    RailRider* getRailRider() const { return mRailRider; }

private:
    Rail* mRail = nullptr;            // _0
    RailRider* mRailRider = nullptr;  // _8
};

class RailKeeperGroup {
public:
    RailKeeperGroup();

    void init(const PlacementInfo&, const char*);
    RailKeeper* getRailKeeper(s32) const;

    s32 getRailKeeperNum() const { return mRailKeeperNum; }

private:
    RailKeeper** mRailKeepers = nullptr;  // _0
    s32 mRailKeeperNum = 0;               // _8
};

RailKeeper* tryCreateRailKeeper(const PlacementInfo&, const char*);
RailKeeperGroup* tryCreateRailKeeperGroup(const PlacementInfo&, const char*);
}  // namespace al
