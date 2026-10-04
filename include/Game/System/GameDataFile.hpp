#pragma once
#include "System/GameDataFileBase.hpp"
class StageDataHolder;
class CourseInfo;
class CourseInfoHolder;
class WorldGameData;
class StockItemList;
class GameDataFile : public GameDataFileBase {
  public:
    void initializeData() override;
    void onSave() override;
    void startStage(int, int) override;
    void onStageStart() override;
    void onStageEnd() override;
    void restartStage() override;
    int calcClearStarLevel() override;
    bool isTwinkleData() const override;
    bool readFromStream(sead::ReadStream*) override;
    void writeToStream(sead::WriteStream*, bool) const override;
    bool entryPlayer(int, int) override;
    void startOpening() override;
    void initPlayerLife(int) override;
    int getPlayerLife() const override;
    bool addPlayerLife(int) override;
    virtual bool checkValid();
    virtual const StageDataHolder* getStageDataHolder() const;
    virtual StageDataHolder* getStageDataHolderPtr();
    virtual void copySaveFile(GameDataFile const&);
    virtual void setPlayingFile();
    virtual void recoverGameOver();
    virtual void recoverGameOverFromGoldenExpress();
    virtual void startSingleModeStage(int, int);
    virtual void onSaveStartInCourseSelect();
    virtual void onWorldWarp(int);
    virtual void onWorldWarpDokanDemo();
    virtual void onGotoTitle(int);
    virtual void updateBestScoreUser();
    virtual void setBestScoreUserId(int);
    virtual void clearStage();
    virtual void clearStageWorldWarp();
    virtual void retireStage();
    virtual void retireStageExitDoor();
    virtual void restartKinopioBrigade();
    virtual void restartMysteryBox();
    virtual void restartTimeupMysteryBox();
    virtual void playWorldStartDemo(int);
    virtual void startEnding();
    virtual void resetCasinoRoomCounter();
    virtual int getCasinoRoomCounter() const;
    virtual int getGoldenExpressCounter() const;
    virtual int getRetryCount() const;
    virtual int getRetryCountSaved() const;
    virtual int getGhostPresentCounter() const;
    virtual const CourseInfo* getCourseInfo(int) const;
    virtual CourseInfo* getCourseInfoPtr(int);
    virtual const CourseInfoHolder* getCourseInfoHolder() const;
    virtual WorldGameData* getWorldGameData(int) const;
    virtual bool isGameOver() const;
    virtual int tryGetLastStageBestScoreUserID() const;
    virtual int getPlayingCourseId() const;
    virtual int getLastPlayCourseId() const;
    virtual int getLastPlayCourseIdSaved() const;
    virtual bool isLastPlayCourseClear() const;
    virtual bool isLastPlayCourseClearWorldWarp() const;
    virtual bool isLastPlayCourseFirstClear() const;
    virtual bool isLastPlayCourseFirstAcquireIllustItem() const;
    virtual bool isLastPlayCourseUpdateBestScore() const;
    virtual bool isLastPlayCourseUpdateBestTime() const;
    virtual bool isNeedOpenCasinoRoom() const;
    virtual bool isNeedOpenGoldenExpress() const;
    virtual bool isNeedSave() const;
    virtual bool isClearNormalEnding() const;
    virtual void setMiiverseSetting(int);
    virtual int getMiiverseSetting() const;
    virtual void setGhostSetting(int);
    virtual int getGhostSetting() const;
    virtual u64 getGameFlag() const;
    virtual int getNetworkSetting() const;
    virtual int getLastTotalAcquireGreenStarNum() const;
    virtual int calcTotalAcquireGreenStarNum() const;
    virtual int calcTotalIllustItemNum() const;
    virtual int getClearStarLevel() const;
    virtual bool isAllClearWithCharacter(int) const;
    virtual bool isRosettaPlayable() const;
    virtual bool isShowBestTime() const;
    virtual bool isOpenWorldStar() const;
    virtual bool isOpenWorldArrange() const;
    virtual bool isOpenWorldChampionship() const;
    virtual bool isShowWorldJumpMenuInfo() const;
    virtual bool isShowWorldJumpMenuInfo2nd() const;
    virtual bool isLastClearStageKinopioBrigade() const;
    virtual bool isLastClearStageUseDrc() const;
    virtual bool isShowCharacterChangeExplain() const;
    virtual bool isOpenNetworkSetting() const;
    virtual bool isShowNetworkGuide() const;
    virtual bool isShowAmiiboGuide() const;
    virtual bool isShowSnapshotGuide() const;
    virtual bool isEnableCancelBossDemo() const;
    virtual void setAlreadyShowBossDemo();
    virtual bool isIsInsideSuperbView() const;
    virtual void setIsInsideSuperbView();
    virtual void resetIsInsideSuperbView();
    virtual u32* getPlayLogStageFlag();
    virtual bool read3DWorldFromStream(sead::ReadStream*);
    static u16 getOptions();
    static void setOptions(u16 options);
    static bool getCameraReverseHorizontal();
    static bool getCameraReverseVertical();
    static void initCameraSettings();
    static void setCameraReverseVertical(GameDataHolder* pHolder, bool reverse);
    static void setCameraReverseHorizontal(GameDataHolder* pHolder, bool reverse);

    /**
     * @brief Access the saved stock items of the file.
     * @return The file's stock-item list.
     */
    StockItemList* getStockItemList() const { return mpStockItems; }

  private:
    static u8 sCameraSettings;
    u8 mUnreconstructed80[0x50]; // Per-course progress, counters, flags, and stage holders.
    StockItemList* mpStockItems;
    u8 mUnreconstructedD8[0x60]; // Remaining counters, flags, and stage holders.
};
static_assert(sizeof(GameDataFile) == 0x138);
