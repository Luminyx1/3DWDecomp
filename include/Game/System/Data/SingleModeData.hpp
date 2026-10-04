#pragma once
#include "System/GameDataFileBase.hpp"
#include <container/seadBuffer.h>
#include <container/seadPtrArray.h>
#include <math/seadVector.h>
#include <nn/time.h>
class IslandSaveData;
class IslandSaveDataHolder;
class SingleModeStockItemArray;
class ScenarioInfo;
namespace al {
class IUseSceneObjHolder;
}
namespace neko {
class Target;
}

class SingleModeData : public GameDataFileBase {
  public:
    /**
     * @brief Bowser's Fury options shared by every single-mode file.
     */
    struct Options {
        bool mIsCameraReverseHorizontal;
        bool mIsCameraReverseVertical;
        s8 mCameraSensitivity;
        u8 mAssistModeType;
    };

    /**
     * @brief A checkpoint location identified by its island.
     */
    struct CheckpointInfo {
        /**
         * @brief Create an empty checkpoint location.
         */
        CheckpointInfo() = default;

        /**
         * @brief Create a checkpoint location.
         * @param islandId Island owning the checkpoint.
         * @param checkpointId Checkpoint identifier within the island.
         */
        CheckpointInfo(s32 islandId, s32 checkpointId)
            : mIslandId(islandId), mCheckpointId(checkpointId) {}

        /**
         * @brief Forget the stored checkpoint.
         */
        void reset() { *this = CheckpointInfo(); }

        s32 mIslandId = -1;
        s32 mCheckpointId = -1;
    };

    /**
     * @brief The main scenario to play next.
     */
    struct MainScenarioInfo {
        s32 mIslandId = -1;
        s32 mScenarioIndex = -1;
    };

    /**
     * @brief Saved state of one cat (neko) target.
     */
    struct NekoSaveData {
        NekoSaveData() = default;
        NekoSaveData(const NekoSaveData& rOther) = default;

        /**
         * @brief Copy the saved state of another cat target.
         * @param rOther Saved state to copy.
         * @return This saved state.
         */
        NekoSaveData& operator=(const NekoSaveData& rOther) {
            set(rOther.mId, rOther.mTrans, rOther.mParentId, rOther.mHostTrans, rOther.mIndex);
            return *this;
        }

        /**
         * @brief Store the saved state of a cat target.
         * @param id Identifier of the cat target.
         * @param rTrans Position of the target.
         * @param parentId Identifier of the target's parent.
         * @param rHostTrans Position of the target's host.
         * @param index Index of the target within its parent.
         */
        void set(s32 id, const sead::Vector3f& rTrans, s32 parentId,
                 const sead::Vector3f& rHostTrans, s32 index) {
            mId = id;
            mTrans = rTrans;
            mParentId = parentId;
            mHostTrans = rHostTrans;
            mIndex = index;
        }

        s32 mId = 0;
        sead::Vector3f mTrans = sead::Vector3f::zero;
        s32 mParentId = 0;
        sead::Vector3f mHostTrans = sead::Vector3f::zero;
        s32 mIndex = -1;
    };

    /**
     * @brief Saved demo state of one cat (neko) parent.
     */
    struct NekoParentData {
        NekoParentData() = default;
        NekoParentData(const NekoParentData& rOther) = default;

        /**
         * @brief Copy the saved demo state of another cat parent.
         * @param rOther Saved state to copy.
         * @return This saved state.
         */
        NekoParentData& operator=(const NekoParentData& rOther) {
            mParentId = rOther.mParentId;
            mIsSeenDemo = rOther.mIsSeenDemo;
            return *this;
        }

        s32 mParentId = 0;
        bool mIsSeenDemo = false;
    };

    static u16 getOptionsData();
    static void setOptionsData(u16 data);
    static void setCameraReverseHorizontal(GameDataHolder* pHolder, bool isReverse);
    static void setCameraReverseVertical(GameDataHolder* pHolder, bool isReverse);
    static void setCameraSensitivity(GameDataHolder* pHolder, s8 sensitivity);
    static void setAssistModeType(GameDataHolder* pHolder, u8 type);

    SingleModeData(GameDataHolder* pHolder, int fileId);
    void initializeData() override;
    void onSave() override;
    void startStage(int worldId, int stageId) override;
    void restartStage() override;
    int calcClearStarLevel() override;

    /**
     * @brief Identify single-mode files as twinkle data.
     * @return Always true.
     */
    bool isTwinkleData() const override { return true; }

    bool readFromStream(sead::ReadStream* pStream) override;
    void writeToStream(sead::WriteStream* pStream, bool isSkip) const override;
    void startSave() override;

    const sead::SafeString& getPhaseName(const al::IUseSceneObjHolder* pUser);
    const sead::SafeString& getPhaseNameForPhaseClearPR(int phase);
    void resetBossPlayTime();
    void setBossPlayTime();
    s64 getBossPlayTime();
    void resetIslandPlayTime();
    void setIslandPlayTime();
    s64 getIslandPlayTime();
    void resetPhaseTotalPlayTime();
    void setPhasePlayTime();
    void updatePhasePlayTime();
    s32 getPhasePlayTime();
    void calculateAllShineCollected();
    void clearIslandGraffitiVandalized();
    bool isAllShineCollected() const;
    bool isAllShineCollectedIslandData();
    bool isCompleteEndingPictureSeen() const;
    bool isFileComplete() const;
    bool isDarkBowserV2Defeated() const;
    bool isSuperHardModeOn(int hitPoint) const;
    bool isDarkBowserV2Available() const;
    int getUnlockedPhase() const;
    bool isMeowserJrAvailable() const;
    void setCompleteEndingPictureSeen();
    void setDarkBowserV2Defeated();
    void copySingleModeFile(const SingleModeData& rOther);
    void copyBrokenDisasterBlockList(const sead::Buffer<s32>& rList);
    void copyBrokenBlockHardList(const sead::Buffer<s32>& rList);
    void setIslandUnlocked(int islandId);
    bool isIslandUnlocked(int islandId) const;
    void setVandalize(int islandId, int phase);
    void clearVandalize(int islandId, int phase);
    bool isVandalized(int islandId, int phase, bool* pActive) const;
    int getUnlockedIslandNum() const;
    int getLastValidIslandVisited() const;
    void setLastValidIslandVisited(int islandId);
    int getCurValidIslandVisited() const;
    void setCurValidIslandVisited(int islandId);
    int getCurActiveScenarioIndex(int islandId) const;
    const IslandSaveData* getIslandSaveData(int islandId) const;
    void setCurActiveScenarioIndex(int islandId, int scenarioIndex);
    IslandSaveData* getIslandSaveDataPtr(int islandId);
    void setCurActiveScenarioNameSeen(int islandId);
    bool wasActiveScenarioNameSeen(int islandId);
    void setCheckpointVisited(int islandId, int checkpointId);
    void setGoalItemCheckpointVisited(int islandId, int checkpointId);
    bool setIslandCheckpointVisited(int islandId);
    void clearLastIslandCheckPointPass();
    void clearAllButGigaBellCheckpoint();
    void clearAllCheckpointPass();
    void clearGetGigaBellPlayerRespawnPoint();
    bool isAnySavedPosition() const;
    bool isPhase4BossDefeated() const;
    bool hasSeenEnding() const;
    void setHasSeenEnding();
    bool isFirstPhase0() const;
    void setIsFirstPhase0();
    bool isFirstPhase2BossDefeated() const;
    void setFirstPhase2BossDefeated();
    bool isFirstPhase3BossDefeated() const;
    void setFirstPhase3BossDefeated();
    void setPhase4BossDefeated();
    bool isAlreadyPlayRidon() const;
    void setPlayRidon();
    void resetPlayRidon();
    void setUnlockedPhase(int phase);
    void setPhaseFromPlessieChase();
    void setPhase3DarkBowserHitPoint(int hitPoint);
    int getPhase3DarkBowserHitPointPreBattle() const;
    void setPhase4DarkBowserHitPoint(int hitPoint);
    int getPhase4DarkBowserHitPointPreBattle() const;
    bool hasSeenCutscene(int cutsceneId) const;
    void setHasSeenCutscene(int cutsceneId);
    void clearHasSeenCutscene(int cutsceneId);
    bool isGenericItemSaved(int itemId) const;
    void setGenericItemSaved(int itemId);
    int getPhase1DarkBowserHitPoint() const;
    void setPhase1DarkBowserHitPoint(int hitPoint);
    int getPhase2DarkBowserHitPoint() const;
    void setPhase2DarkBowserHitPoint(int hitPoint);
    int getPhase3DarkBowserHitPoint() const;
    int getPhase4DarkBowserHitPoint() const;
    int getPhase4DarkBowserHitPointFinal() const;
    void setPhase4DarkBowserHitPointFinal(int hitPoint);
    void setPhase3DarkBowserHitPointPreBattle(int hitPoint);
    void setPhase4DarkBowserHitPointPreBattle(int hitPoint);
    bool isNewToPhase1();
    void setIsNewToPhase1(bool isNew);
    bool isNewToPhase2();
    void setIsNewToPhase2(bool isNew);
    bool isNewToPhase3();
    void setIsNewToPhase3(bool isNew);
    bool isNewToPhase4();
    void setIsNewToPhase4(bool isNew);
    bool isNewToPhase2Boss();
    void setIsNewToPhase2Boss(bool isNew);
    bool isNewToPhase3Boss();
    void setIsNewToPhase3Boss(bool isNew);
    bool isNewToPhase1BowserIntro();
    void setIsNewToPhase1BowserIntro(bool isNew);
    bool isNewToPhase2BowserIntro();
    void setIsNewToPhase2BowserIntro(bool isNew);
    bool isNewToPhase3BowserIntro();
    void setIsNewToPhase3BowserIntro(bool isNew);
    bool isNewToPhase1BowserExit();
    bool isNewToPhase2BowserExit();
    bool isNewToPhase3BowserExit();
    void setIsNewToPhase1BowserExit(bool isNew);
    void setIsNewToPhase2BowserExit(bool isNew);
    void setIsNewToPhase3BowserExit(bool isNew);
    bool shouldFadeToWhite();
    void setShouldFadeToWhite(bool isFade);
    bool tryGetGigaBellPlayerRespawnPoint(sead::Vector3f* pTrans, sead::Vector3f* pFront) const;
    void setGigaBellPlayerRespawnPoint(const sead::Vector3f& rTrans, const sead::Vector3f& rFront);
    bool tryGetGenericPlayerRespawn(sead::Vector3f* pTrans, sead::Vector3f* pFront) const;
    void clearGenericPlayerRespawn();
    void setGenericPlayerRespawn(const sead::Vector3f& rTrans, const sead::Vector3f& rFront);
    void destroyDisasterBlock(int blockId);
    bool isDisasterBlockDestroyed(int blockId);
    void resetDisasterBlocks();
    void destroyBlockHard(int blockId);
    bool isBlockHardDestroyed(int blockId);
    void resetBlockHards();
    void setGuideMessageSeen(u32 messageId, bool isFirstSet);
    void resetGuideMessageSeen(u32 messageId, bool isFirstSet);
    bool isGuideMessageAlreadySeen(u32 messageId, bool isFirstSet) const;
    bool tryGetNekoSaveData(int id, neko::Target* pTarget) const;
    int tryGetNekoSaveDataByParentID(int parentId, sead::PtrArray<neko::Target>* pTargets) const;
    void setNekoSaveData(int id, const neko::Target* pTarget);
    bool hasNekoParentSeenDemo(int parentId) const;
    void setNekoParentSeenDemo(int parentId);
    void subtractCoin(int count);
    bool hasUnlockedChar(int charaType) const;
    bool hasUnlockedAnyChar() const;
    void setUnlockedChar(int charaType);
    void updateMainScenario();
    const IslandSaveData* getOceanQuadrantSaveData(int quadrant);
    IslandSaveData* getOceanQuadrantSaveDataPtr(int quadrant);
    void completeScenario(const ScenarioInfo& rInfo);
    void resetScenario(const ScenarioInfo& rInfo);
    void setLuckyIslandPosCompleted(int posIndex);
    bool wasLuckyIslandPosCompleted(int posIndex);
    int getLuckyShineIdxByPosIdx(int posIndex);

    static constexpr s32 cNekoSaveDataNum = 16;
    static constexpr s32 cNekoParentDataNum = 4;
    static constexpr s32 cLuckyIslandPosNum = 5;

    static Options sOptions;

  private:

    /**
     * @brief Test a single-mode progression flag.
     * @param mask Progression bits to test.
     * @return True when any requested bit is set.
     */
    bool hasFlag(u32 mask) const { return (mFlags & mask) != 0; }

    u32 mFlags = 0;
    IslandSaveDataHolder* mpIslands;
    IslandSaveDataHolder* mpOceanQuadrants;
    SingleModeStockItemArray* mpStockItems;
    MainScenarioInfo mMainScenario;
    s32 mLastIsland = -1;
    s32 mCurrentIsland = -1;
    CheckpointInfo mCheckpoint;
    CheckpointInfo mGoalItemCheckpoint;
    s32 mIslandCheckpoint = -1;
    s32 mGoalItemNum;
    s32 mUnlockedPhase;
    s32 mUnlockedIslandNum;
    bool mUnknownD0;
    bool mIsNewToPhase1;
    bool mIsNewToPhase2;
    bool mIsNewToPhase3;
    bool mIsNewToPhase4;
    bool mIsNewToPhase2Boss;
    bool mIsNewToPhase3Boss;
    bool mIsNewToPhase1BowserIntro;
    bool mIsNewToPhase2BowserIntro;
    bool mIsNewToPhase3BowserIntro;
    bool mIsNewToPhase1BowserExit;
    bool mIsNewToPhase2BowserExit;
    bool mIsNewToPhase3BowserExit;
    bool mShouldFadeToWhite;
    bool mUnknownDE;
    bool mUnknownDF;
    s32 mUnknownE0;
    s32 mUnknownE4;
    s32 mUnknownE8;
    s32 mUnknownEC = -1;
    bool mUnknownF0 = false;
    s32 mPhase1DarkBowserHitPoint;
    s32 mPhase2DarkBowserHitPoint;
    s32 mPhase3DarkBowserHitPoint;
    s32 mPhase4DarkBowserHitPoint = 200;
    s32 mPhase4DarkBowserHitPointFinal;
    s32 mPhase3DarkBowserHitPointPreBattle;
    s32 mPhase4DarkBowserHitPointPreBattle;
    bool mIsGigaBellRespawnValid = false;
    sead::Vector3f mGigaBellRespawnTrans;
    sead::Vector3f mGigaBellRespawnFront;
    bool mIsGenericRespawnValid = false;
    sead::Vector3f mGenericRespawnTrans;
    sead::Vector3f mGenericRespawnFront;
    bool mUnknown148 = false;
    u32 mSeenCutsceneFlags = 0;
    u32 mSavedGenericItemFlags = 0;
    s8 mLuckyIslandPos[cLuckyIslandPosNum];
    sead::Buffer<s32> mDisasterBlocks;
    NekoSaveData* mpNekoSaveData;
    NekoParentData* mpNekoParentData;
    sead::Buffer<s32> mBlockHards;
    u32 mGuideMessageSeenFlags[2] = {};
    s32 mUnknown198;
    s32 mUnknown19C;
    nn::TimeSpan mBossPlayStartTime = {};
    s64 mBossPlayTime;
    nn::TimeSpan mIslandPlayStartTime = {};
    s64 mIslandPlayTime;
    nn::TimeSpan mPhasePlayStartTime = {};
    sead::DateTime mUnknownDateTime = sead::DateTime(0);
    s64 mPhaseTotalPlayTime;
};
static_assert(sizeof(SingleModeData) == 0x1d8);
