#pragma once
#include "Library/Scene/ISceneObj.hpp"
#include <basis/seadTypes.h>
#include "System/GameDataCommon.hpp"
namespace al {
class SceneObjHolder;
class NetworkSystem;
} // namespace al
namespace preport {
class PlayReportManager;
}
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
    u8 mUnknown61[0xf]; // Unreconstructed mode and report state.
    SaveDataAccessSequence* mpSaveAccess;
    al::NetworkSystem* mpNetwork;
    void* mpUnknown80;
    al::SceneObjHolder* mpSceneObjHolder;
    preport::PlayReportManager* mpPlayReportManager;
};
static_assert(sizeof(GameDataHolder) == 0x98);
