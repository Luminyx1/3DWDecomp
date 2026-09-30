#pragma once

#include <math/seadVector.h>

#include "Library/Scene/ISceneObj.hpp"

namespace al {
class ActorInitInfo;
class BlockRail;
class BlockRailRider;

class BlockRailDirector : public ISceneObj {
public:
    BlockRailDirector();

    void initAfterPlacementSceneObj(const ActorInitInfo& rInfo) override;

    void registerBlockRail(BlockRail* pRail);
    bool tryRideBlockRail(BlockRailRider* pRider, const sead::Vector3f& rPrevPos,
                          const sead::Vector3f& rPos);

    BlockRail** mRails = nullptr;
    s32 mRailNum = 0;
    s32 mMaxRails = 0x100;
};

static_assert(sizeof(BlockRailDirector) == 0x18);
}  // namespace al
