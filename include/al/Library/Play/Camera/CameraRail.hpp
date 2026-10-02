#pragma once

#include <math/seadVector.h>

namespace al {
class LiveActor;
struct PlacementInfo;
class RailKeeper;

class CameraRail {
public:
    CameraRail(const PlacementInfo& rInfo);

    f32 calcTopPlayer(LiveActor* pActor, s32* pTopPlayerIndex);
    sead::Vector3f calcPlayerRailPos(LiveActor* pActor, s32 playerIndex);
    void setPlayerRailPos(LiveActor* pActor);
    void setPlayerRailDir(LiveActor* pActor);

    RailKeeper* getRailKeeper() const { return mRailKeeper; }

private:
    RailKeeper* mRailKeeper;
};

}  // namespace al
