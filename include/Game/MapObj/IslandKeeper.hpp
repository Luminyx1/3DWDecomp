#pragma once
#include <math/seadVector.h>
#include "Library/Scene/ISceneObj.hpp"
class IslandHolder;
namespace al {
class IUseSceneObjHolder;
class LiveActor;
}  // namespace al
class IslandKeeper : public al::ISceneObj {
public:
    explicit IslandKeeper(int);
    int getActiveIslandIndex() const { return mActiveIslandIndex; }
    void* findIsland(int index) const;
    static IslandKeeper* tryGetIslandKeeper(const al::IUseSceneObjHolder* pHolder);
    void addActorLinkToIsland(al::LiveActor* pActor, int islandId);
    int getIndexHolderNum() const;
    IslandHolder* getIslandHolderIndex(int index) const;
    bool getIslandStartPos(int islandId, sead::Vector3f& rPos, sead::Vector3f& rFront);
    /** @brief Id of the island the player is currently on. @return The island id. */
    int getCurrentIslandId() const { return mCurrentIslandId; }
    void setIslandLODDisable(int islandId, bool isDisable);
    void tryUpdateLastIslandScenario();
private:
    int mCurrentIslandId;
    u8 mUnknownC[4];
    int mActiveIslandIndex;
    u8 mUnknown14[0x44];
};
static_assert(sizeof(IslandKeeper) == 0x58);
