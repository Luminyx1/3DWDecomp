#include "System/Data/SingleModeData.hpp"
#include "System/ControlUserDataHolder.hpp"

/**
 * @brief Check a single-mode progression flag.
 * @return True when the progression flag is set.
 */
bool SingleModeData::isAllShineCollected() const { return hasFlag(0x400); }

/**
 * @brief Check a single-mode progression flag.
 * @return True when the progression flag is set.
 */
bool SingleModeData::isCompleteEndingPictureSeen() const { return hasFlag(0x100); }

/**
 * @brief Check a single-mode progression flag.
 * @return True when the progression flag is set.
 */
bool SingleModeData::isDarkBowserV2Defeated() const { return hasFlag(0x200); }

/**
 * @brief Check a single-mode progression flag.
 * @return True when the progression flag is set.
 */
bool SingleModeData::isDarkBowserV2Available() const { return hasFlag(0x400); }

/**
 * @brief Check a single-mode progression flag.
 * @return True when the progression flag is set.
 */
bool SingleModeData::isPhase4BossDefeated() const { return hasFlag(0x800); }

/**
 * @brief Check a single-mode progression flag.
 * @return True when the progression flag is set.
 */
bool SingleModeData::hasSeenEnding() const { return hasFlag(0x1); }

/**
 * @brief Check a single-mode progression flag.
 * @return True when the progression flag is set.
 */
bool SingleModeData::isFirstPhase2BossDefeated() const { return hasFlag(0x40); }

/**
 * @brief Check a single-mode progression flag.
 * @return True when the progression flag is set.
 */
bool SingleModeData::isFirstPhase3BossDefeated() const { return hasFlag(0x80); }

/**
 * @brief Check a single-mode progression flag.
 * @return True when the progression flag is set.
 */
bool SingleModeData::isAlreadyPlayRidon() const { return hasFlag(0x1000); }

/**
 * @brief Record single-mode progression.
 */
void SingleModeData::setCompleteEndingPictureSeen() { mFlags |= 0x100; }

/**
 * @brief Record single-mode progression.
 */
void SingleModeData::setDarkBowserV2Defeated() { mFlags |= 0x200; }

/**
 * @brief Record single-mode progression.
 */
void SingleModeData::setHasSeenEnding() { mFlags |= 0x1; }

/**
 * @brief Record single-mode progression.
 */
void SingleModeData::setIsFirstPhase0() { mFlags |= 0x20; }

/**
 * @brief Record single-mode progression.
 */
void SingleModeData::setFirstPhase2BossDefeated() { mFlags |= 0x40; }

/**
 * @brief Record single-mode progression.
 */
void SingleModeData::setFirstPhase3BossDefeated() { mFlags |= 0x80; }

/**
 * @brief Record single-mode progression.
 */
void SingleModeData::setPhase4BossDefeated() { mFlags |= 0x800; }

/**
 * @brief Record single-mode progression.
 */
void SingleModeData::setPlayRidon() { mFlags |= 0x1000; }

/**
 * @brief Check whether all completion requirements are met.
 * @return True when the ending picture, final boss, and all-shines flags are set.
 */
bool SingleModeData::isFileComplete() const { return (mFlags & 0x700) == 0x700; }

/**
 * @brief Check whether all completion requirements are met.
 * @return True when the ending picture, final boss, and all-shines flags are set.
 */
bool SingleModeData::isMeowserJrAvailable() const { return (mFlags & 0x700) == 0x700; }

/**
 * @brief Read single-mode progression.
 * @return The stored progression identifier or count.
 */
int SingleModeData::getUnlockedPhase() const { return mUnlockedPhase; }

/**
 * @brief Read single-mode progression.
 * @return The stored progression identifier or count.
 */
int SingleModeData::getUnlockedIslandNum() const { return mUnlockedIslandNum; }

/**
 * @brief Read single-mode progression.
 * @return The stored progression identifier or count.
 */
int SingleModeData::getLastValidIslandVisited() const { return mLastIsland; }

/**
 * @brief Read single-mode progression.
 * @return The stored progression identifier or count.
 */
int SingleModeData::getCurValidIslandVisited() const { return mCurrentIsland; }

/**
 * @brief Record the previous valid island.
 * @param islandId Valid island identifier to remember.
 */
void SingleModeData::setLastValidIslandVisited(int islandId) { mLastIsland = islandId; }

/**
 * @brief Set the unlocked progression phase.
 * @param phase Progression phase identifier.
 */
void SingleModeData::setUnlockedPhase(int phase) { mUnlockedPhase = phase; }

/**
 * @brief Handle the single-mode save notification, which requires no additional work.
 */
void SingleModeData::onSave() {}

/**
 * @brief Reset player figures for a single-mode restart.
 */
void SingleModeData::restartStage() { mpUsers->resetFigureType(); }
