#pragma once

#include <math/seadVector.h>

namespace al {
class BlockRailLink;
class BlockRailRider;

class BlockRailRouteSelecter {
public:
    virtual bool compareBlockRailRoute(const BlockRailRider* pRider, const BlockRailLink* pLinkA,
                                       const BlockRailLink* pLinkB) const = 0;
};

class BlockRailRider {
public:
    BlockRailRider();

    void move(f32 speed, sead::Vector3f* pPos, sead::Vector3f* pDir);
    bool calcPosAndDir(sead::Vector3f* pPos, sead::Vector3f* pDir) const;
    BlockRailLink* trySelectPrevLink() const;
    BlockRailLink* trySelectNextLink() const;
    BlockRailLink* selectRoute(BlockRailLink* pCurrentLink, BlockRailLink* pCandidateLink) const;
    bool calcDir(sead::Vector3f* pDir) const;
    void setRailPart(BlockRailLink* pLink, f32 coord);
    void setRouteSelecter(BlockRailRouteSelecter* pSelecter);
    void reverse();
    bool isRide() const;

    BlockRailLink* getRailLink() const { return mRailLink; }
    f32 getCoord() const { return mCoord; }
    bool isForward() const { return mIsForward; }
    bool isReachEnd() const { return mIsReachEnd; }

    BlockRailLink* mRailLink = nullptr;
    BlockRailRouteSelecter* mRouteSelecter;
    f32 mCoord = 0.0f;
    bool mIsForward = true;
    bool mIsReachEnd = false;
    bool mIsLeaveAtEnd = true;
};

static_assert(sizeof(BlockRailRider) == 0x18);
}  // namespace al
