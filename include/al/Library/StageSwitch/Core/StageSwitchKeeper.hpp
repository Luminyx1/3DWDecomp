#pragma once

#include <basis/seadTypes.h>

#include "Library/StageSwitch/Core/IUseStageSwitch.hpp"

namespace al {
class StageSwitchAccesser;
class StageSwitchDirector;
struct PlacementInfo;

class StageSwitchKeeper {
public:
    StageSwitchKeeper();

    void init(StageSwitchDirector* pDirector, const PlacementInfo& rInfo);
    StageSwitchAccesser* tryGetStageSwitchAccesser(const char* pLinkName) const;
    bool isUsingSwitchNo(s32 switchNo);

    void setUseName(IUseName* pUseName) { mUseName = pUseName; }

    StageSwitchAccesser* mAccessors = nullptr;
    s32 mLinkCount = 0;
    u32 _C;
    IUseName* mUseName = nullptr;
};
}  // namespace al
