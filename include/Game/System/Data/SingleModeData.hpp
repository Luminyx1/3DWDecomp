#pragma once
#include "System/GameDataFileBase.hpp"
class IslandSaveDataHolder;
class SingleModeData : public GameDataFileBase {
  public:
    void initializeData() override;
    void onSave() override;
    void startStage(int worldId, int stageId) override;
    void restartStage() override;
    int calcClearStarLevel() override;
    bool isTwinkleData() const override;
    bool readFromStream(sead::ReadStream* pStream) override;
    void writeToStream(sead::WriteStream* pStream, bool option) const override;
    int getUnlockedPhase() const;
    int getUnlockedIslandNum() const;
    int getLastValidIslandVisited() const;
    int getCurValidIslandVisited() const;
    void setLastValidIslandVisited(int islandId);
    void setUnlockedPhase(int phase);
    bool isAllShineCollected() const;
    bool isCompleteEndingPictureSeen() const;
    bool isDarkBowserV2Defeated() const;
    bool isDarkBowserV2Available() const;
    bool isPhase4BossDefeated() const;
    bool hasSeenEnding() const;
    bool isFirstPhase2BossDefeated() const;
    bool isFirstPhase3BossDefeated() const;
    bool isAlreadyPlayRidon() const;
    void setCompleteEndingPictureSeen();
    void setDarkBowserV2Defeated();
    void setHasSeenEnding();
    void setIsFirstPhase0();
    void setFirstPhase2BossDefeated();
    void setFirstPhase3BossDefeated();
    void setPhase4BossDefeated();
    void setPlayRidon();
    bool isFileComplete() const;
    bool isMeowserJrAvailable() const;

  private:
    /**
     * @brief Test a single-mode progression flag.
     * @param mask Progression bits to test.
     * @return True when any requested bit is set.
     */
    bool hasFlag(u32 mask) const { return (mFlags & mask) != 0; }

    u32 mFlags;
    u32 mUnknown84;
    IslandSaveDataHolder* mpIslands;
    u8 mUnknown90[0x18];
    int mLastIsland;
    int mCurrentIsland;
    u8 mUnknownB0[0x18];
    int mUnlockedPhase;
    int mUnlockedIslandNum;
    u8 mUnreconstructedD0[0x108];
};
static_assert(sizeof(SingleModeData) == 0x1d8);
