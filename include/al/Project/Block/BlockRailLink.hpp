#pragma once

#include <math/seadQuat.h>
#include <math/seadVector.h>

namespace al {
class ActorInitInfo;
class BlockRailShape;
class ByamlIter;

class BlockRailLink {
public:
    BlockRailLink(s32 maxLinks);

    void init(const ActorInitInfo& rInfo, const ByamlIter& rIter);
    void init(const sead::Quatf& rQuat, const sead::Vector3f& rTrans, const ByamlIter& rIter);
    void calcOffset(const sead::Vector3f& rBaseTrans);
    void updateLinkedTrans(const sead::Vector3f& rBaseTrans);
    void setShape(BlockRailShape* pShape);
    void addPrev(BlockRailLink* pLink);
    void addNext(BlockRailLink* pLink);
    bool isRide(f32* pRate, const sead::Vector3f& rPrevPos, const sead::Vector3f& rPos) const;
    f32 getTotalLength() const;
    void calcPosAndDir(sead::Vector3f* pPos, sead::Vector3f* pDir, f32 rate) const;
    void calcPos(sead::Vector3f* pPos, f32 rate) const;
    void calcDir(sead::Vector3f* pDir, f32 rate) const;
    void calcNearestParam(sead::Vector3f* pPos, f32* pRate, const sead::Vector3f& rPos) const;
    bool isTerminate() const;
    BlockRailLink* getPrevLink(s32 index) const;
    BlockRailLink* getNextLink(s32 index) const;
    bool isPrevLink(const BlockRailLink* pLink) const;
    bool isNextLink(const BlockRailLink* pLink) const;

    static void tryConnect(BlockRailLink* pLinkA, BlockRailLink* pLinkB, f32 distance);

    s32 getPrevLinkNum() const { return mPrevLinkNum; }
    s32 getNextLinkNum() const { return mNextLinkNum; }
    bool isValidRide() const { return mIsValidRide; }
    void validateRide() { mIsValidRide = true; }
    void invalidateRide() { mIsValidRide = false; }

    BlockRailShape* mShape = nullptr;
    BlockRailLink** mPrevLinks = nullptr;
    BlockRailLink** mNextLinks = nullptr;
    s32 mPrevLinkNum = 0;
    s32 mNextLinkNum = 0;
    s32 mMaxLinks;
    bool mIsValidRide = true;
};

static_assert(sizeof(BlockRailLink) == 0x28);
}  // namespace al
