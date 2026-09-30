#pragma once

#include <math/seadVector.h>

namespace al {
class ActorInitInfo;
class BlockRailParts;

class BlockRailPartsGroup {
public:
    BlockRailPartsGroup();

    void init(const ActorInitInfo& rInfo);
    void calcOffset(const sead::Vector3f& rBaseTrans);
    void updateLinkedTrans(const sead::Vector3f& rBaseTrans);
    void active();
    void specialActive(u32 index);
    void deactive();
    BlockRailParts* getParts(s32 index) const;
    s32 calcEmptyLinkCount() const;

    s32 getPartsNum() const { return mPartsNum; }

    BlockRailParts** mParts = nullptr;
    s32 mPartsNum = 0;
};

static_assert(sizeof(BlockRailPartsGroup) == 0x10);
}  // namespace al
