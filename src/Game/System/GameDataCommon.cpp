#include "System/GameDataCommon.hpp"
#include "System/PlayLogData.hpp"

/**
 * @brief Create common save data and its play-log storage.
 * @param pHolder Non-null game-data holder providing the course database.
 */
GameDataCommon::GameDataCommon(GameDataHolder* pHolder)
    : mValues{}, mpPlayLog(nullptr), mState(0), mPlayReportVersion(1) {
    mpPlayLog = new PlayLogData(pHolder);
}

/**
 * @brief Reset common save values and play-report version.
 */
void GameDataCommon::initializeData() {
    mValues[0] = 0;
    mValues[1] = 0;
    mValues[2] = 0;
    mpPlayLog->initializeData();
    mState = 0;
    mPlayReportVersion = 1;
}

/**
 * @brief Mark the common play-report data as version one.
 */
void GameDataCommon::updatePlayReportCommonVersion() { mPlayReportVersion = 1; }

/**
 * @brief Accept the common save-data block without additional validation.
 * @return Always true.
 */
bool GameDataCommon::checkValid() { return true; }
