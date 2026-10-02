#pragma once

#include <basis/seadTypes.h>

#include "Library/HostIO/IUseHioNode.hpp"

namespace al {
class AnimInfoTable;

class AnimPlayerBase : public HioNode {
public:
    AnimPlayerBase();

    virtual void updateLast() { _10 = false; }

    virtual bool calcNeedUpdateAnimNext() = 0;

    AnimInfoTable* getAnimInfoTable() const { return mInfoTable; }

    AnimInfoTable* mInfoTable = nullptr;
    bool _10 = false;  // set when the animation was changed this frame
    bool _11 = false;  // set while the animation needs to be applied to the model
};

static_assert(sizeof(AnimPlayerBase) == 0x18);

}  // namespace al
