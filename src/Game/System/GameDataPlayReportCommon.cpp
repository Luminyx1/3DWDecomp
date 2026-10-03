#include "System/GameDataPlayReportCommon.hpp"
#include <cstring>

/**
 * @brief Clear the common play-report counters.
 * @param pHolder Game-data holder; unused by this constructor.
 */
GameDataPlayReportCommon::GameDataPlayReportCommon(GameDataHolder* pHolder) {
    std::memset(mValues, 0, sizeof(mValues));
}

/**
 * @brief Reset all common play-report counters.
 */
void GameDataPlayReportCommon::initializeData() { std::memset(mValues, 0, sizeof(mValues)); }

/**
 * @brief Accept common play-report data without additional validation.
 * @return Always true.
 */
bool GameDataPlayReportCommon::checkValid() { return true; }
