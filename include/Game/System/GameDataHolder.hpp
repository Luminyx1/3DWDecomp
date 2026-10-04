#pragma once
#include "Library/Scene/ISceneObj.hpp"
#include <basis/seadTypes.h>
#include "System/GameDataCommon.hpp"
namespace al {
class SceneObjHolder;
class NetworkSystem;
} // namespace al
#include <preport/PlayReportManager.h>
#include <prim/seadSafeString.h>
class GameDataCommon;
class GameDataPlayReportCommon;
class GameDataFile;
class SingleModeData;
class StageListHolder;
class StageDataHolder;
class SaveDataAccessSequence;
class OceanScenarioList;
class IslandDataList;
class CourseInfo;
class ControlUserDataHolder;
enum GameMode : int;
class GameDataHolder : public al::ISceneObj {
  public:
    const char* getSceneObjName() const override;
    void setSceneObjHolder(al::SceneObjHolder* pHolder);
    GameDataFile* getGameDataFile(int fileId) const;
    SingleModeData* getSingleModeDataFile(int fileId) const;
    void setLastPlayedMode(GameMode mode);
    GameMode getLastPlayedMode() const;
    int getLastPlayingFileId() const;
    int getLastSingleModePlayingFileID() const;
    bool isUnlockLuigiBros() const;
    void unlockLuigiBros();
    void setGameFileForTitleDemo(GameDataFile* pFile);
    const StageDataHolder* getStageDataHolder() const;
    StageDataHolder* getStageDataHolderPtr();
    const CourseInfo* getCourseInfo(int courseId) const;
    CourseInfo* getCourseInfoPtr(int courseId);
    ControlUserDataHolder* getControlUserDataHolder() const;
    void addSessionId();
    bool beginPlayReport(preport::KeyEventType type, int eventId, int option);
    void endPlayReport();
    void setPlayReportData(preport::Key key, int value);
    void setPlayReportData(preport::Key key, float value);
    void setPlayReportData(preport::Key key, s64 value);
    void setPlayReportData(preport::Key key, sead::SafeString& rValue);
    void setPlayReportData(preport::Key key, int* pValues, int num);
    void setPlayReportData(preport::Key key, float* pValues, int num);
    void sendNetworkStatus();

    /**
     * @brief Access the course database.
     * @return The initialized course database.
     */
    StageListHolder* getStageList() const { return mpStageList; }

    /**
     * @brief Access the save-operation state machine.
     * @return The save sequence, or nullptr before creation.
     */
    SaveDataAccessSequence* getSaveAccess() const { return mpSaveAccess; }

    /**
     * @brief Check whether Bowser's Fury mode is active.
     * @return True for single mode.
     */
    bool isSingleMode() const { return mSingleMode; }

    /**
     * @brief Access the common play-log storage.
     * @return The allocated play-log storage.
     */
    PlayLogData* getPlayLog() const { return mpCommon->mpPlayLog; }

    /**
     * @brief Access the active 3D World save file.
     * @return The active 3D World save file.
     */
    GameDataFile* getPlayingFile() const { return mpPlayingFile; }

    /**
     * @brief Access the active Bowser's Fury save file.
     * @return The active single-mode save file.
     */
    SingleModeData* getSingleFile() const { return mpSingleFile; }

    /**
     * @brief Access the ocean-quadrant scenario lists.
     * @return The ocean scenario lists.
     */
    OceanScenarioList* getOceanScenarioList() const { return mpOceanScenarioList; }

    /**
     * @brief Access the Bowser's Fury island database.
     * @return The island list.
     */
    IslandDataList* getIslandDataList() const { return mpIslandDataList; }

    /**
     * @brief Check whether the two-player assist mode is active.
     * @return True when a second player assists.
     */
    bool is2PAssistMode() const { return mIs2PAssistMode; }

    /**
     * @brief Enable or disable the two-player assist mode.
     * @param isAssist True to enable the assist mode.
     */
    void set2PAssistMode(bool isAssist) { mIs2PAssistMode = isAssist; }

    /**
     * @brief Check whether the map is enabled.
     * @return True when the map can be used.
     */
    bool isMapEnabled() const { return mIsMapEnabled; }

    /**
     * @brief Enable or disable the map.
     * @param isEnabled True to enable the map.
     */
    void setMapEnabled(bool isEnabled) { mIsMapEnabled = isEnabled; }

    /**
     * @brief Check whether the last demo was cancelled.
     * @return True when the demo was cancelled.
     */
    bool isDemoWasCancelled() const { return mIsDemoWasCancelled; }

    /**
     * @brief Record whether the last demo was cancelled.
     * @param isCancelled True when the demo was cancelled.
     */
    void setDemoWasCancelled(bool isCancelled) { mIsDemoWasCancelled = isCancelled; }

    /**
     * @brief Check whether a save has been requested.
     * @return True when the progress should be saved.
     */
    bool isSaveRequested() const { return mIsSaveRequested; }

    /**
     * @brief Request or cancel a save.
     * @param isRequested True to request a save.
     */
    void setSaveRequested(bool isRequested) { mIsSaveRequested = isRequested; }

    /**
     * @brief Check whether the Bowser's Fury prologue phase is active.
     * @return True during phase 0.
     */
    bool isPhase0() const { return mIsPhase0; }

    /**
     * @brief Check whether the scene is being restarted.
     * @return True while the scene restarts.
     */
    bool isSceneRestart() const { return mIsSceneRestart; }

    /**
     * @brief Mark whether the scene is being restarted.
     * @param isRestart True while the scene restarts.
     */
    void setSceneRestart(bool isRestart) { mIsSceneRestart = isRestart; }

    /**
     * @brief Access the play-report manager.
     * @return The play-report manager.
     */
    preport::PlayReportManager* getPlayReportManager() const { return mpPlayReportManager; }

  private:
    GameDataCommon* mpCommon;
    GameDataPlayReportCommon* mpPlayReportCommon;
    GameDataFile** mppFiles;
    GameDataFile* mpPlayingFile;
    SingleModeData** mppSingleFiles;
    SingleModeData* mpSingleFile;
    StageListHolder* mpStageList;
    OceanScenarioList* mpOceanScenarioList;
    IslandDataList* mpIslandDataList;
    u8 mUnknown50[0x10]; // Unreconstructed stage-transition and time fields.
    bool mSingleMode;
    u8 mUnknown61[2]; // Unreconstructed mode state.
    bool mIs2PAssistMode;
    bool mIsMapEnabled;
    bool mIsDemoWasCancelled;
    u8 mUnknown66;
    bool mIsSaveRequested;
    bool mIsPhase0;
    bool mIsSceneRestart;
    u8 mUnknown6A[6]; // Unreconstructed report state.
    SaveDataAccessSequence* mpSaveAccess;
    al::NetworkSystem* mpNetwork;
    void* mpUnknown80;
    al::SceneObjHolder* mpSceneObjHolder;
    preport::PlayReportManager* mpPlayReportManager;
};
static_assert(sizeof(GameDataHolder) == 0x98);
