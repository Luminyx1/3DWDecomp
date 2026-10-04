#include "System/GameDataCommon.hpp"
#include "System/Data/SingleModeData.hpp"
#include "System/GameDataFile.hpp"
#include "System/PlayLogData.hpp"
#include <stream/seadStream.h>

namespace {

/**
 * @brief Serialized layout of the common save-data block.
 */
struct GameDataCommonSaveBlock {
    u32 values[3];
    bool state;
    u16 options;
    u16 optionsData;
    u8 playReportVersion;
    u8 reserved[0x15];
};

static_assert(sizeof(GameDataCommonSaveBlock) == 0x28);

} // namespace

/**
 * @brief Create common save data and its play-log storage.
 * @param pHolder Non-null game-data holder providing the course database.
 */
GameDataCommon::GameDataCommon(GameDataHolder* pHolder)
    : mValues{0, 0, 0}, mpPlayLog(nullptr), mState(0), mPlayReportVersion(1) {
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

/**
 * @brief Read the common save-data block, global options and play-log data.
 * @param pStream Non-null input stream positioned at the common block.
 * @return Always true; stream errors are handled by the stream.
 */
bool GameDataCommon::readFromStream(sead::ReadStream* pStream) {
    GameDataCommonSaveBlock block = {};
    pStream->readMemBlock(&block, sizeof(block));
    mValues[0] = block.values[0];
    mValues[1] = block.values[1];
    mValues[2] = block.values[2];
    mState = block.state;
    GameDataFile::setOptions(block.options);
    SingleModeData::setOptionsData(block.optionsData);
    mPlayReportVersion = block.playReportVersion;
    mpPlayLog->readFromStream(pStream);
    return true;
}

/**
 * @brief Write the common save-data block, global options and play-log data.
 * @param pStream Non-null output stream positioned at the common block.
 */
void GameDataCommon::writeToStream(sead::WriteStream* pStream) const {
    GameDataCommonSaveBlock block = {};
    block.values[0] = mValues[0];
    block.values[1] = mValues[1];
    block.values[2] = mValues[2];
    block.state = mState;
    block.options = GameDataFile::getOptions();
    block.optionsData = SingleModeData::getOptionsData();
    block.playReportVersion = mPlayReportVersion;
    pStream->writeMemBlock(&block, sizeof(block));
    mpPlayLog->writeToStream(pStream);
}
