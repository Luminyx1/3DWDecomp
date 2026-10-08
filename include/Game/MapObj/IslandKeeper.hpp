#pragma once
#include "Library/Scene/ISceneObj.hpp"
class IslandKeeper : public al::ISceneObj {
public:
    explicit IslandKeeper(int);
    int getActiveIslandIndex() const { return mActiveIslandIndex; }
    void* findIsland(int index) const;
private:
    u8 mUnknown8[8];
    int mActiveIslandIndex;
    u8 mUnknown14[0x44];
};
static_assert(sizeof(IslandKeeper) == 0x58);
