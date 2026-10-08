#pragma once

#include <basis/seadTypes.h>

#include "Library/LiveActor/LiveActor.hpp"

/// The model actor of one of the player's figures.
class PlayerModel : public al::LiveActor {
public:
    /** @brief Gets the index used to pick this model's retargetting info. @return Index. */
    s32 getRetargettingIndex() const { return mRetargettingIndex; }

    /** @brief Gets the name of this model's animation set (e.g. "Climb"). @return Name. */
    const char* getAnimSetName() const { return mAnimSetName; }

private:
    s32 mRetargettingIndex;  // 0x144
    u8 _148[0x158 - 0x148];
    const char* mAnimSetName;  // 0x158
};
