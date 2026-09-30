#pragma once

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class BlockRailLink;
class ByamlIter;
struct PlacementInfo;

class BlockRailParts : public LiveActor {
public:
    BlockRailParts(const char* pName);

    void init(const ActorInitInfo& rInfo) override;
    void appear() override;
    void updateLinkedTrans(const sead::Vector3f& rBaseTrans) override;
    void startFarLod() override;
    void endFarLod() override;

    void initRailLink(const ActorInitInfo& rInfo);
    void calcOffset(const sead::Vector3f& rBaseTrans);
    ActorInitInfo* getInitInfo() const;
    void initRailLink();
    void initRailLink(const ByamlIter& rIter);
    BlockRailLink* getLink(s32 index) const;
    void setIsHideModel(bool isHide);

    static void tryConnect(BlockRailParts* pPartsA, BlockRailParts* pPartsB);

    s32 getLinkNum() const { return mLinkNum; }

    BlockRailLink** mLinks = nullptr;
    s32 mLinkNum = 0;
    const char* mModelSuffix = nullptr;
    ActorInitInfo* mInitInfo = nullptr;
    PlacementInfo* mPlacementInfo = nullptr;
    bool mIsHideModel = false;
    sead::Vector3f mOffset;
};

static_assert(sizeof(BlockRailParts) == 0x180);
}  // namespace al
