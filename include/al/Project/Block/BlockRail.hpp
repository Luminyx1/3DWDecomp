#pragma once

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class BlockRailLink;
class BlockRailRider;

class BlockRail : public LiveActor {
public:
    BlockRail(const char* pName);

    void init(const ActorInitInfo& rInfo) override;
    void makeActorAppeared() override;
    void makeActorDead() override;

    void tryCreateRailEnd(const ActorInitInfo& rInfo);
    void createRailEnd(const ActorInitInfo& rInfo, const sead::Vector3f& rPos,
                       const sead::Vector3f& rDir, LiveActor** pRailEnd);

    static void tryConnect(BlockRail* pRailA, BlockRail* pRailB);

    BlockRailLink* getRailLink() const { return mRailLink; }

    BlockRailLink* mRailLink = nullptr;
    LiveActor* mStartRailEnd = nullptr;
    LiveActor* mEndRailEnd = nullptr;
    const char* mEndModelName = nullptr;
    s32 mRailColor = 0;
};

static_assert(sizeof(BlockRail) == 0x170);

void registerBlockRail(BlockRail* pRail);
bool tryRideBlockRail(const LiveActor* pActor, BlockRailRider* pRider,
                      const sead::Vector3f& rPrevPos, const sead::Vector3f& rPos);
}  // namespace al
