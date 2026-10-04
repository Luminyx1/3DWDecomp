#include "System/GameDataFlagFunction.hpp"
#include "Library/Controller/InputFunction.hpp"
#include "System/CourseInfoHolder.hpp"
#include "System/Data/WorldGameData.hpp"
#include "System/GameDataFile.hpp"
#include "System/GameDataFunction.hpp"

namespace {

/// Bit indices of the progression flags stored in a 3D World save file.
enum GameFlagBit : s32 {
    cGameFlagBit_AllClearMario = 0,
    cGameFlagBit_AllClearLuigi = 1,
    cGameFlagBit_AllClearPeach = 2,
    cGameFlagBit_AllClearKinopio = 3,
    cGameFlagBit_AllClearRosetta = 4,
    cGameFlagBit_OpenRosetta = 5,
    cGameFlagBit_OpenShowBestTime = 6,
    cGameFlagBit_OpenArrangeWorld = 7,
    cGameFlagBit_OpenChampionshipWorld = 8,
    cGameFlagBit_PlayKinopioBrigade = 9,
    cGameFlagBit_PlayCasinoRoom = 10,
    cGameFlagBit_OpenCourseSelectRocket = 11,
    cGameFlagBit_ShowWorldJumpMenuInfo = 12,
    cGameFlagBit_ShowSpecialWorldOpenDemo = 13,
    cGameFlagBit_PlayRidon = 14,
    cGameFlagBit_FirstClearKinopioBrigade = 15,
    cGameFlagBit_ShowWorldJumpMenuInfo2nd = 16,
    cGameFlagBit_ShowEventGateKeeperFirst = 17,
    cGameFlagBit_ShowEventGateKeeperSecond = 18,
    cGameFlagBit_ShowCharacterChangeExplain = 19,
    cGameFlagBit_PlayMysteryHouse = 20,
    cGameFlagBit_RetryMysteryHouse = 21,
    cGameFlagBit_PlayKinopioHouse = 22,
    cGameFlagBit_OpenNetworkSetting = 23,
    cGameFlagBit_FirstClearDrcTouchStage = 24,
    cGameFlagBit_ShowAfterGameOverInfo = 25,
    cGameFlagBit_ShowSnapshotGuide = 26,
    cGameFlagBit_FirstLaunchCourseSelectRocket = 27,
    cGameFlagBit_ShowNetworkGuide = 28,
    cGameFlagBit_ShowAmiiboGuide = 29,
    cGameFlagBit_ShowTouchGuide = 30,
    cGameFlagBit_Num = 31,
};

/**
 * @brief Build the progression flag mask of one bit.
 * @param bit Bit index of the flag.
 * @return The flag mask.
 */
constexpr u32 toGameFlag(s32 bit) {
    return 1u << bit;
}

/**
 * @brief Access the active 3D World save file.
 * @param accessor Accessor to an initialized game-data holder.
 * @return The active 3D World save file.
 */
inline GameDataFile* getPlayingFile(GameDataHolderAccessor accessor) {
    return accessor.getHolder()->getPlayingFile();
}

/**
 * @brief Check a progression flag of the active 3D World file.
 * @param accessor Accessor to an initialized game-data holder.
 * @param flag Progression flag mask to test.
 * @return True when the flag is set; always false in Bowser's Fury mode.
 */
inline bool isOnGameFlag(GameDataHolderAccessor accessor, u64 flag) {
    if (accessor.getHolder()->isSingleMode()) {
        return false;
    }

    return (getPlayingFile(accessor)->getGameFlag() & flag) != 0;
}

/**
 * @brief Set a progression flag of the active 3D World file.
 * @param writer Writer to an initialized game-data holder.
 * @param flag Progression flag mask to set.
 */
inline void onGameFlag(GameDataHolderWriter writer, u32 flag) {
    if (writer.getHolder()->isSingleMode()) {
        return;
    }

    getPlayingFile(writer)->onGameFlag(flag);
}

/**
 * @brief Check a character's all-clear flag of the active 3D World file.
 * @param accessor Accessor to an initialized game-data holder.
 * @param characterType Playable character identifier from 0 through 4.
 * @return True when the flag is set; always false in Bowser's Fury mode.
 */
inline bool isOnAllClearCharacterFlag(GameDataHolderAccessor accessor, s32 characterType) {
    if (accessor.getHolder()->isSingleMode()) {
        return false;
    }

    return (static_cast<u32>(getPlayingFile(accessor)->getGameFlag()) &
            toGameFlag(characterType)) != 0;
}

/**
 * @brief Set or clear a progression flag depending on a condition.
 * @param rGameFlag Progression flags to update.
 * @param flag Progression flag mask to update.
 * @param isOn True to set the flag, false to clear it.
 */
inline void updateGameFlag(u32& rGameFlag, u32 flag, bool isOn) {
    if (isOn) {
        rGameFlag = flag | rGameFlag;
    } else {
        rGameFlag &= ~flag;
    }
}

u64 calcNewGameFlag(GameDataHolderAccessor accessor);

/**
 * @brief Check whether a progression flag would newly become set.
 * @param accessor Accessor to an initialized game-data holder.
 * @param flag Progression flag mask to test.
 * @return True when the flag is not set yet but its condition is met.
 */
inline bool isNewGameFlag(GameDataHolderAccessor accessor, u64 flag) {
    return !isOnGameFlag(accessor, flag) && (calcNewGameFlag(accessor) & flag) != 0;
}

} // namespace

/**
 * @brief Check whether any progression flag would newly become set.
 * @param accessor Accessor to an initialized game-data holder.
 * @param isExcludeTouchGuide True to ignore the touch-guide flag.
 * @return True when at least one flag is not set yet but its condition is met.
 */
bool GameDataFlagFunction::isExistNewOpenFlag(GameDataHolderAccessor accessor,
                                              bool isExcludeTouchGuide) {
    if (accessor.getHolder()->isSingleMode()) {
        return false;
    }

    u32 mask = isExcludeTouchGuide ? ~toGameFlag(cGameFlagBit_ShowTouchGuide) : 0xFFFFFFFF;
    u32 oldFlag = static_cast<u32>(getPlayingFile(accessor)->getGameFlag()) & mask;
    u32 newFlag = static_cast<u32>(calcNewGameFlag(accessor)) & mask;
    return (newFlag & ~oldFlag) != 0;
}

namespace {

/**
 * @brief Compute the progression flags with every unset flag whose condition is met turned on.
 * @param accessor Accessor to an initialized game-data holder.
 * @return The updated flags; zero in Bowser's Fury mode.
 */
u64 calcNewGameFlag(GameDataHolderAccessor accessor) {
    u32 gameFlag = 0;
    if (!accessor.getHolder()->isSingleMode()) {
        gameFlag = getPlayingFile(accessor)->getGameFlag();
        for (s32 i = 0; i < cGameFlagBit_Num; i++) {
            u32 flag = toGameFlag(i);
            if ((flag & gameFlag) != 0) {
                continue;
            }

            switch (i) {
            case cGameFlagBit_AllClearMario:
                updateGameFlag(gameFlag, flag,
                               getPlayingFile(accessor)->isAllClearWithCharacter(0));
                break;
            case cGameFlagBit_AllClearLuigi:
                updateGameFlag(gameFlag, flag,
                               getPlayingFile(accessor)->isAllClearWithCharacter(1));
                break;
            case cGameFlagBit_AllClearPeach:
                updateGameFlag(gameFlag, flag,
                               getPlayingFile(accessor)->isAllClearWithCharacter(2));
                break;
            case cGameFlagBit_AllClearKinopio:
                updateGameFlag(gameFlag, flag,
                               getPlayingFile(accessor)->isAllClearWithCharacter(3));
                break;
            case cGameFlagBit_AllClearRosetta:
                updateGameFlag(gameFlag, flag,
                               getPlayingFile(accessor)->isAllClearWithCharacter(4));
                break;
            case cGameFlagBit_OpenRosetta:
                updateGameFlag(gameFlag, flag, getPlayingFile(accessor)->isRosettaPlayable());
                break;
            case cGameFlagBit_OpenShowBestTime:
                updateGameFlag(gameFlag, flag, getPlayingFile(accessor)->isShowBestTime());
                break;
            case cGameFlagBit_OpenArrangeWorld:
                updateGameFlag(gameFlag, flag, getPlayingFile(accessor)->isOpenWorldArrange());
                break;
            case cGameFlagBit_OpenChampionshipWorld:
                updateGameFlag(gameFlag, flag, getPlayingFile(accessor)->isOpenWorldChampionship());
                break;
            case cGameFlagBit_ShowWorldJumpMenuInfo:
                updateGameFlag(gameFlag, flag, getPlayingFile(accessor)->isShowWorldJumpMenuInfo());
                break;
            case cGameFlagBit_FirstClearKinopioBrigade:
                updateGameFlag(gameFlag, flag,
                               getPlayingFile(accessor)->isLastClearStageKinopioBrigade());
                break;
            case cGameFlagBit_ShowWorldJumpMenuInfo2nd:
                updateGameFlag(gameFlag, flag,
                               getPlayingFile(accessor)->isShowWorldJumpMenuInfo2nd());
                break;
            case cGameFlagBit_ShowCharacterChangeExplain:
                updateGameFlag(gameFlag, flag,
                               getPlayingFile(accessor)->isShowCharacterChangeExplain());
                break;
            case cGameFlagBit_OpenNetworkSetting:
                updateGameFlag(gameFlag, flag, getPlayingFile(accessor)->isOpenNetworkSetting());
                break;
            case cGameFlagBit_FirstClearDrcTouchStage:
                updateGameFlag(gameFlag, flag, getPlayingFile(accessor)->isLastClearStageUseDrc());
                break;
            case cGameFlagBit_ShowAfterGameOverInfo:
                updateGameFlag(
                    gameFlag, flag,
                    (isOnGameFlag(accessor, toGameFlag(cGameFlagBit_PlayKinopioHouse)) ||
                     isOnGameFlag(accessor, toGameFlag(cGameFlagBit_PlayCasinoRoom))) &&
                        GameDataFlagFunction::isAfterGameOver(accessor));
                break;
            case cGameFlagBit_ShowSnapshotGuide:
                updateGameFlag(gameFlag, flag, getPlayingFile(accessor)->isShowSnapshotGuide());
                break;
            case cGameFlagBit_ShowNetworkGuide:
                updateGameFlag(gameFlag, flag, getPlayingFile(accessor)->isShowNetworkGuide());
                break;
            case cGameFlagBit_ShowAmiiboGuide:
                updateGameFlag(gameFlag, flag, getPlayingFile(accessor)->isShowAmiiboGuide());
                break;
            case cGameFlagBit_ShowTouchGuide:
                updateGameFlag(gameFlag, flag, getPlayingFile(accessor)->isShowTouchGuide());
                break;
            default:
                break;
            }
        }
    }

    return gameFlag;
}

} // namespace

/**
 * @brief Check whether a world's start demo was already shown in the active 3D World file.
 * @param accessor Accessor to an initialized game-data holder.
 * @param worldId World to check.
 * @return True when the demo was shown; always false in Bowser's Fury mode.
 */
bool GameDataFlagFunction::isShowWorldStartDemo(GameDataHolderAccessor accessor, int worldId) {
    if (accessor.getHolder()->isSingleMode()) {
        return false;
    }

    return getPlayingFile(accessor)->getWorldGameData(worldId)->isShowFirstDemo();
}

/**
 * @brief Check whether a world's start demo was already shown in a 3D World file.
 * @param accessor Accessor to an initialized game-data holder.
 * @param worldId World to check.
 * @param fileId Save-file slot to check.
 * @return True when the demo was shown; always false in Bowser's Fury mode.
 */
bool GameDataFlagFunction::isShowWorldStartDemo(GameDataHolderAccessor accessor, int worldId,
                                                int fileId) {
    if (accessor.getHolder()->isSingleMode()) {
        return false;
    }

    GameDataFile* pFile = accessor.getHolder()->getGameDataFile(fileId);
    return pFile->getWorldGameData(worldId)->isShowFirstDemo();
}

/**
 * @brief Remember that a world's start demo was shown in the active 3D World file.
 * @param writer Writer to an initialized game-data holder.
 * @param worldId World whose demo was shown.
 */
void GameDataFlagFunction::setShowWorldStartDemo(GameDataHolderWriter writer, int worldId) {
    if (writer.getHolder()->isSingleMode()) {
        return;
    }

    getPlayingFile(writer)->getWorldGameData(worldId)->setShowFirstDemoFlag();
}

/**
 * @brief Check whether the active 3D World file was resumed after a game over.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True after a game over; always false in Bowser's Fury mode.
 */
bool GameDataFlagFunction::isAfterGameOver(GameDataHolderAccessor accessor) {
    if (accessor.getHolder()->isSingleMode()) {
        return false;
    }

    return getPlayingFile(accessor)->isAfterGameOver();
}

/**
 * @brief Forget that the active 3D World file was resumed after a game over.
 * @param writer Writer to an initialized game-data holder.
 */
void GameDataFlagFunction::resetAfterGameOver(GameDataHolderWriter writer) {
    if (writer.getHolder()->isSingleMode()) {
        return;
    }

    getPlayingFile(writer)->resetAfterGameOver();
}

/**
 * @brief Check whether the ending was started in the active 3D World file.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True after the ending; always false in Bowser's Fury mode.
 */
bool GameDataFlagFunction::isAfterEnding(GameDataHolderAccessor accessor) {
    if (accessor.getHolder()->isSingleMode()) {
        return false;
    }

    return getPlayingFile(accessor)->isAfterEnding();
}

/**
 * @brief Forget that the ending was started in the active 3D World file.
 * @param writer Writer to an initialized game-data holder.
 */
void GameDataFlagFunction::resetAfterEnding(GameDataHolderWriter writer) {
    if (writer.getHolder()->isSingleMode()) {
        return;
    }

    getPlayingFile(writer)->resetAfterEnding();
}

/**
 * @brief Find the message of the first progression flag a stage clear would newly set.
 * @param accessor Accessor to an initialized game-data holder.
 * @return The message label, or nullptr when no flag would newly become set.
 */
const char* GameDataFlagFunction::tryGetFirstNewOpenFlagMessageLabelByStageClear(
    GameDataHolderAccessor accessor) {
    if (accessor.getHolder()->isSingleMode()) {
        return nullptr;
    }

    if (isNewOpenAllClearCharacter(accessor, 0)) {
        return "OpenAllClearMario";
    }

    if (isNewOpenAllClearCharacter(accessor, 1)) {
        return "OpenAllClearLuigi";
    }

    if (isNewOpenAllClearCharacter(accessor, 2)) {
        return "OpenAllClearPeach";
    }

    if (isNewOpenAllClearCharacter(accessor, 3)) {
        return "OpenAllClearKinopio";
    }

    if (isNewOpenAllClearCharacter(accessor, 4)) {
        return "OpenAllClearRosetta";
    }

    if (isNewOpenShowBestTime(accessor)) {
        return "OpenShowBestTime";
    }

    if (isNewOpenRosetta(accessor)) {
        return "OpenRosettaPlayable";
    }

    if (isNewOpenArrangeWorld(accessor)) {
        return "OpenWorldArrange";
    }

    if (isNewOpenChampionshipWorld(accessor)) {
        return "OpenWorldChampionship";
    }

    if (isNewShowWorldJumpMenuInfo(accessor)) {
        return "ShowWorldJumpMenuInfo";
    }

    if (isNewShowWorldJumpMenuInfo2nd(accessor)) {
        return "ShowWorldJumpMenuInfoSecond";
    }

    if (isNewFirstClearKinopioBrigade(accessor)) {
        return "FirstClearKinopioBrigade";
    }

    if (isNewShowCharacterChangeExplain(accessor)) {
        return "ShowCharacterChangeExplain";
    }

    if (isNewFirstClearDrcTouchStage(accessor)) {
        if (al::isPadTypeHandheld(al::getMainControllerPort())) {
            return "FirstClearDrcTouchStage_Handheld";
        }

        return "FirstClearDrcTouchStage";
    }

    if (isNewShowAfterGameOverInfo(accessor)) {
        return "ShowAfterGameOverInfo";
    }

    if (isNewShowNetworkGuide(accessor)) {
        return "NetworkGuide";
    }

    if (isNewShowAmiiboGuide(accessor)) {
        return "AmiiboGuide";
    }

    if (isNewShowSnapshotGuide(accessor)) {
        return "SnapshotGuide";
    }

    return nullptr;
}

/**
 * @brief Check whether a character's all-clear flag would newly become set.
 * @param accessor Accessor to an initialized game-data holder.
 * @param characterType Playable character identifier from 0 through 4.
 * @return True when the flag is not set yet but its condition is met.
 */
bool GameDataFlagFunction::isNewOpenAllClearCharacter(GameDataHolderAccessor accessor,
                                                      int characterType) {
    return !isOnAllClearCharacterFlag(accessor, characterType) &&
           (static_cast<u32>(calcNewGameFlag(accessor)) & toGameFlag(characterType)) != 0;
}

/**
 * @brief Check whether the best-time display flag would newly become available.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is not set yet but its condition is met.
 */
bool GameDataFlagFunction::isNewOpenShowBestTime(GameDataHolderAccessor accessor) {
    return isNewGameFlag(accessor, toGameFlag(cGameFlagBit_OpenShowBestTime));
}

/**
 * @brief Check whether the Rosalina unlock flag would newly become available.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is not set yet but its condition is met.
 */
bool GameDataFlagFunction::isNewOpenRosetta(GameDataHolderAccessor accessor) {
    return isNewGameFlag(accessor, toGameFlag(cGameFlagBit_OpenRosetta));
}

/**
 * @brief Check whether the Crown world unlock flag would newly become available.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is not set yet but its condition is met.
 */
bool GameDataFlagFunction::isNewOpenArrangeWorld(GameDataHolderAccessor accessor) {
    return isNewGameFlag(accessor, toGameFlag(cGameFlagBit_OpenArrangeWorld));
}

/**
 * @brief Check whether the Champion's Road unlock flag would newly become available.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is not set yet but its condition is met.
 */
bool GameDataFlagFunction::isNewOpenChampionshipWorld(GameDataHolderAccessor accessor) {
    return isNewGameFlag(accessor, toGameFlag(cGameFlagBit_OpenChampionshipWorld));
}

/**
 * @brief Check whether the world-jump menu explanation flag would newly become available.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is not set yet but its condition is met.
 */
bool GameDataFlagFunction::isNewShowWorldJumpMenuInfo(GameDataHolderAccessor accessor) {
    return isNewGameFlag(accessor, toGameFlag(cGameFlagBit_ShowWorldJumpMenuInfo));
}

/**
 * @brief Check whether the second world-jump menu explanation flag would newly become available.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is not set yet but its condition is met.
 */
bool GameDataFlagFunction::isNewShowWorldJumpMenuInfo2nd(GameDataHolderAccessor accessor) {
    return isNewGameFlag(accessor, toGameFlag(cGameFlagBit_ShowWorldJumpMenuInfo2nd));
}

/**
 * @brief Check whether the first Captain Toad clear flag would newly become available.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is not set yet but its condition is met.
 */
bool GameDataFlagFunction::isNewFirstClearKinopioBrigade(GameDataHolderAccessor accessor) {
    return isNewGameFlag(accessor, toGameFlag(cGameFlagBit_FirstClearKinopioBrigade));
}

/**
 * @brief Check whether the character-change explanation flag would newly become available.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is not set yet but its condition is met.
 */
bool GameDataFlagFunction::isNewShowCharacterChangeExplain(GameDataHolderAccessor accessor) {
    return isNewGameFlag(accessor, toGameFlag(cGameFlagBit_ShowCharacterChangeExplain));
}

/**
 * @brief Check whether the first touch-stage clear flag would newly become available.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is not set yet but its condition is met.
 */
bool GameDataFlagFunction::isNewFirstClearDrcTouchStage(GameDataHolderAccessor accessor) {
    return isNewGameFlag(accessor, toGameFlag(cGameFlagBit_FirstClearDrcTouchStage));
}

/**
 * @brief Check whether the after-game-over explanation flag would newly become available.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is not set yet but its condition is met.
 */
bool GameDataFlagFunction::isNewShowAfterGameOverInfo(GameDataHolderAccessor accessor) {
    return isNewGameFlag(accessor, toGameFlag(cGameFlagBit_ShowAfterGameOverInfo));
}

/**
 * @brief Check whether the network guide flag would newly become available.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is not set yet but its condition is met.
 */
bool GameDataFlagFunction::isNewShowNetworkGuide(GameDataHolderAccessor accessor) {
    return isNewGameFlag(accessor, toGameFlag(cGameFlagBit_ShowNetworkGuide));
}

/**
 * @brief Check whether the amiibo guide flag would newly become available.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is not set yet but its condition is met.
 */
bool GameDataFlagFunction::isNewShowAmiiboGuide(GameDataHolderAccessor accessor) {
    return isNewGameFlag(accessor, toGameFlag(cGameFlagBit_ShowAmiiboGuide));
}

/**
 * @brief Check whether the snapshot guide flag would newly become available.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is not set yet but its condition is met.
 */
bool GameDataFlagFunction::isNewShowSnapshotGuide(GameDataHolderAccessor accessor) {
    return isNewGameFlag(accessor, toGameFlag(cGameFlagBit_ShowSnapshotGuide));
}

/**
 * @brief Set the first progression flag a stage clear newly unlocks.
 * @param writer Writer to an initialized game-data holder.
 */
void GameDataFlagFunction::setFirstAlreadyOpenFlagByStageClear(GameDataHolderWriter writer) {
    if (writer.getHolder()->isSingleMode()) {
        return;
    }

    for (s32 i = 0; i < 5; i++) {
        if (isNewOpenAllClearCharacter(writer, i)) {
            setAllClearCharacter(writer, i);
            return;
        }
    }

    if (isNewOpenShowBestTime(writer)) {
        setOpenShowBestTime(writer);
    } else if (isNewOpenRosetta(writer)) {
        setOpenRosetta(writer);
    } else if (isNewOpenArrangeWorld(writer)) {
        setOpenArrangeWorld(writer);
    } else if (isNewOpenChampionshipWorld(writer)) {
        setOpenChampionshipWorld(writer);
    } else if (isNewShowWorldJumpMenuInfo(writer)) {
        setShowWorldJumpMenuInfo(writer);
    } else if (isNewShowWorldJumpMenuInfo2nd(writer)) {
        setShowWorldJumpMenuInfo2nd(writer);
    } else if (isNewFirstClearKinopioBrigade(writer)) {
        setFirstClearKinopioBrigade(writer);
    } else if (isNewShowCharacterChangeExplain(writer)) {
        setShowCharacterChangeExplain(writer);
    } else if (isNewFirstClearDrcTouchStage(writer)) {
        setFirstClearDrcTouchStage(writer);
    } else if (isNewShowAfterGameOverInfo(writer)) {
        setShowAfterGameOverInfo(writer);
    } else if (isNewShowNetworkGuide(writer)) {
        setShowNetworkGuide(writer);
    } else if (isNewShowAmiiboGuide(writer)) {
        setShowAmiiboGuide(writer);
    } else if (isNewShowSnapshotGuide(writer)) {
        setShowSnapshotGuide(writer);
    }
}

/**
 * @brief Set a character's all-clear flag.
 * @param writer Writer to an initialized game-data holder.
 * @param characterType Playable character identifier from 0 through 4.
 */
void GameDataFlagFunction::setAllClearCharacter(GameDataHolderWriter writer, int characterType) {
    if (writer.getHolder()->isSingleMode()) {
        return;
    }

    getPlayingFile(writer)->onGameFlag(toGameFlag(characterType));
}

/**
 * @brief Set the best-time display flag.
 * @param writer Writer to an initialized game-data holder.
 */
void GameDataFlagFunction::setOpenShowBestTime(GameDataHolderWriter writer) {
    onGameFlag(writer, toGameFlag(cGameFlagBit_OpenShowBestTime));
}

/**
 * @brief Set the Rosalina unlock flag.
 * @param writer Writer to an initialized game-data holder.
 */
void GameDataFlagFunction::setOpenRosetta(GameDataHolderWriter writer) {
    onGameFlag(writer, toGameFlag(cGameFlagBit_OpenRosetta));
}

/**
 * @brief Set the Crown world unlock flag.
 * @param writer Writer to an initialized game-data holder.
 */
void GameDataFlagFunction::setOpenArrangeWorld(GameDataHolderWriter writer) {
    onGameFlag(writer, toGameFlag(cGameFlagBit_OpenArrangeWorld));
}

/**
 * @brief Set the Champion's Road unlock flag.
 * @param writer Writer to an initialized game-data holder.
 */
void GameDataFlagFunction::setOpenChampionshipWorld(GameDataHolderWriter writer) {
    onGameFlag(writer, toGameFlag(cGameFlagBit_OpenChampionshipWorld));
}

/**
 * @brief Set the world-jump menu explanation flag.
 * @param writer Writer to an initialized game-data holder.
 */
void GameDataFlagFunction::setShowWorldJumpMenuInfo(GameDataHolderWriter writer) {
    onGameFlag(writer, toGameFlag(cGameFlagBit_ShowWorldJumpMenuInfo));
}

/**
 * @brief Set the second world-jump menu explanation flag.
 * @param writer Writer to an initialized game-data holder.
 */
void GameDataFlagFunction::setShowWorldJumpMenuInfo2nd(GameDataHolderWriter writer) {
    onGameFlag(writer, toGameFlag(cGameFlagBit_ShowWorldJumpMenuInfo2nd));
}

/**
 * @brief Set the first Captain Toad clear flag.
 * @param writer Writer to an initialized game-data holder.
 */
void GameDataFlagFunction::setFirstClearKinopioBrigade(GameDataHolderWriter writer) {
    onGameFlag(writer, toGameFlag(cGameFlagBit_FirstClearKinopioBrigade));
}

/**
 * @brief Set the character-change explanation flag.
 * @param writer Writer to an initialized game-data holder.
 */
void GameDataFlagFunction::setShowCharacterChangeExplain(GameDataHolderWriter writer) {
    onGameFlag(writer, toGameFlag(cGameFlagBit_ShowCharacterChangeExplain));
}

/**
 * @brief Set the first touch-stage clear flag.
 * @param writer Writer to an initialized game-data holder.
 */
void GameDataFlagFunction::setFirstClearDrcTouchStage(GameDataHolderWriter writer) {
    onGameFlag(writer, toGameFlag(cGameFlagBit_FirstClearDrcTouchStage));
}

/**
 * @brief Set the after-game-over explanation flag.
 * @param writer Writer to an initialized game-data holder.
 */
void GameDataFlagFunction::setShowAfterGameOverInfo(GameDataHolderWriter writer) {
    onGameFlag(writer, toGameFlag(cGameFlagBit_ShowAfterGameOverInfo));
}

/**
 * @brief Set the network guide flag.
 * @param writer Writer to an initialized game-data holder.
 */
void GameDataFlagFunction::setShowNetworkGuide(GameDataHolderWriter writer) {
    onGameFlag(writer, toGameFlag(cGameFlagBit_ShowNetworkGuide));
}

/**
 * @brief Set the amiibo guide flag.
 * @param writer Writer to an initialized game-data holder.
 */
void GameDataFlagFunction::setShowAmiiboGuide(GameDataHolderWriter writer) {
    onGameFlag(writer, toGameFlag(cGameFlagBit_ShowAmiiboGuide));
}

/**
 * @brief Set the snapshot guide flag.
 * @param writer Writer to an initialized game-data holder.
 */
void GameDataFlagFunction::setShowSnapshotGuide(GameDataHolderWriter writer) {
    onGameFlag(writer, toGameFlag(cGameFlagBit_ShowSnapshotGuide));
}

/**
 * @brief Check a character's all-clear flag in the active 3D World file.
 * @param accessor Accessor to an initialized game-data holder.
 * @param characterType Playable character identifier from 0 through 4.
 * @return True when the character's all-clear flag is set.
 */
bool GameDataFlagFunction::isAlreadyOpenAllClearCharacter(GameDataHolderAccessor accessor,
                                                          int characterType) {
    return isOnAllClearCharacterFlag(accessor, characterType);
}

/**
 * @brief Check a character's all-clear flag in a 3D World file.
 * @param accessor Accessor to an initialized game-data holder.
 * @param characterType Playable character identifier from 0 through 4.
 * @param fileId Save-file slot to check.
 * @return True when the file is not new and the character's all-clear flag is set.
 */
bool GameDataFlagFunction::isAlreadyOpenAllClearCharacter(GameDataHolderAccessor accessor,
                                                          int characterType, int fileId) {
    GameDataFile* pFile = accessor.getHolder()->getGameDataFile(fileId);
    if (pFile->isNewFile()) {
        return false;
    }

    return (static_cast<u32>(pFile->getGameFlag()) & toGameFlag(characterType)) != 0;
}

/**
 * @brief Check whether the Rosalina unlock flag is already set.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is set; always false in Bowser's Fury mode.
 */
bool GameDataFlagFunction::isAlreadyOpenRosetta(GameDataHolderAccessor accessor) {
    return isOnGameFlag(accessor, toGameFlag(cGameFlagBit_OpenRosetta));
}

/**
 * @brief Check whether the best-time display flag is already set.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is set; always false in Bowser's Fury mode.
 */
bool GameDataFlagFunction::isAlreadyOpenShowBestTime(GameDataHolderAccessor accessor) {
    return isOnGameFlag(accessor, toGameFlag(cGameFlagBit_OpenShowBestTime));
}

/**
 * @brief Check whether the Crown world unlock flag is already set.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is set; always false in Bowser's Fury mode.
 */
bool GameDataFlagFunction::isAlreadyOpenArrangeWorld(GameDataHolderAccessor accessor) {
    return isOnGameFlag(accessor, toGameFlag(cGameFlagBit_OpenArrangeWorld));
}

/**
 * @brief Check whether the Champion's Road unlock flag is already set.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is set; always false in Bowser's Fury mode.
 */
bool GameDataFlagFunction::isAlreadyOpenChampionshipWorld(GameDataHolderAccessor accessor) {
    return isOnGameFlag(accessor, toGameFlag(cGameFlagBit_OpenChampionshipWorld));
}

/**
 * @brief Check whether the world-jump menu explanation flag is already set.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is set; always false in Bowser's Fury mode.
 */
bool GameDataFlagFunction::isAlreadyShowWorldJumpMenuInfo(GameDataHolderAccessor accessor) {
    return isOnGameFlag(accessor, toGameFlag(cGameFlagBit_ShowWorldJumpMenuInfo));
}

/**
 * @brief Check whether the second world-jump menu explanation flag is already set.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is set; always false in Bowser's Fury mode.
 */
bool GameDataFlagFunction::isAlreadyShowWorldJumpMenuInfo2nd(GameDataHolderAccessor accessor) {
    return isOnGameFlag(accessor, toGameFlag(cGameFlagBit_ShowWorldJumpMenuInfo2nd));
}

/**
 * @brief Check whether the first Captain Toad clear flag is already set.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is set; always false in Bowser's Fury mode.
 */
bool GameDataFlagFunction::isAlreadyFirstClearKinopioBrigade(GameDataHolderAccessor accessor) {
    return isOnGameFlag(accessor, toGameFlag(cGameFlagBit_FirstClearKinopioBrigade));
}

/**
 * @brief Check whether the after-game-over explanation flag is already set.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is set; always false in Bowser's Fury mode.
 */
bool GameDataFlagFunction::isAlreadyShowAfterGameOverInfo(GameDataHolderAccessor accessor) {
    return isOnGameFlag(accessor, toGameFlag(cGameFlagBit_ShowAfterGameOverInfo));
}

/**
 * @brief Check whether the network guide flag is already set.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is set; always false in Bowser's Fury mode.
 */
bool GameDataFlagFunction::isAlreadyShowNetworkGuide(GameDataHolderAccessor accessor) {
    return isOnGameFlag(accessor, toGameFlag(cGameFlagBit_ShowNetworkGuide));
}

/**
 * @brief Check whether the amiibo guide flag is already set.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is set; always false in Bowser's Fury mode.
 */
bool GameDataFlagFunction::isAlreadyShowAmiiboGuide(GameDataHolderAccessor accessor) {
    return isOnGameFlag(accessor, toGameFlag(cGameFlagBit_ShowAmiiboGuide));
}

/**
 * @brief Check whether the snapshot guide flag is already set.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is set; always false in Bowser's Fury mode.
 */
bool GameDataFlagFunction::isAlreadyShowSnapshotGuide(GameDataHolderAccessor accessor) {
    return isOnGameFlag(accessor, toGameFlag(cGameFlagBit_ShowSnapshotGuide));
}

/**
 * @brief Check whether the touch guide flag would newly become available.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is not set yet but its condition is met.
 */
bool GameDataFlagFunction::isNewShowTouchGuide(GameDataHolderAccessor accessor) {
    return isNewGameFlag(accessor, toGameFlag(cGameFlagBit_ShowTouchGuide));
}

/**
 * @brief Check whether the touch guide flag is already set.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is set; always false in Bowser's Fury mode.
 */
bool GameDataFlagFunction::isAlreadyShowTouchGuide(GameDataHolderAccessor accessor) {
    return isOnGameFlag(accessor, toGameFlag(cGameFlagBit_ShowTouchGuide));
}

/**
 * @brief Set the touch guide flag.
 * @param writer Writer to an initialized game-data holder.
 */
void GameDataFlagFunction::setShowTouchGuide(GameDataHolderWriter writer) {
    onGameFlag(writer, toGameFlag(cGameFlagBit_ShowTouchGuide));
}

/**
 * @brief Check whether the character-change explanation flag is already set.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is set; always false in Bowser's Fury mode.
 */
bool GameDataFlagFunction::isAlreadyShowCharacterChangeExplain(GameDataHolderAccessor accessor) {
    return isOnGameFlag(accessor, toGameFlag(cGameFlagBit_ShowCharacterChangeExplain));
}

/**
 * @brief Check whether the network-setting unlock flag would newly become available.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is not set yet but its condition is met.
 */
bool GameDataFlagFunction::isNewOpenNetworkSetting(GameDataHolderAccessor accessor) {
    return isNewGameFlag(accessor, toGameFlag(cGameFlagBit_OpenNetworkSetting));
}

/**
 * @brief Check whether the network-setting unlock flag is already set.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is set; always false in Bowser's Fury mode.
 */
bool GameDataFlagFunction::isAlreadyOpenNetworkSetting(GameDataHolderAccessor accessor) {
    return isOnGameFlag(accessor, toGameFlag(cGameFlagBit_OpenNetworkSetting));
}

/**
 * @brief Set the network-setting unlock flag.
 * @param writer Writer to an initialized game-data holder.
 */
void GameDataFlagFunction::setOpenNetworkSetting(GameDataHolderWriter writer) {
    onGameFlag(writer, toGameFlag(cGameFlagBit_OpenNetworkSetting));
}

/**
 * @brief Check whether the first touch-stage clear flag is already set.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is set; always false in Bowser's Fury mode.
 */
bool GameDataFlagFunction::isAlreadyFirstClearDrcTouchStage(GameDataHolderAccessor accessor) {
    return isOnGameFlag(accessor, toGameFlag(cGameFlagBit_FirstClearDrcTouchStage));
}

/**
 * @brief Check whether the special-world opening demo flag is already set.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is set; always false in Bowser's Fury mode.
 */
bool GameDataFlagFunction::isAlreadyShowSpecialWorldOpenDemo(GameDataHolderAccessor accessor) {
    return isOnGameFlag(accessor, toGameFlag(cGameFlagBit_ShowSpecialWorldOpenDemo));
}

/**
 * @brief Set the special-world opening demo flag.
 * @param writer Writer to an initialized game-data holder.
 */
void GameDataFlagFunction::setShowSpecialWorldOpenDemo(GameDataHolderWriter writer) {
    onGameFlag(writer, toGameFlag(cGameFlagBit_ShowSpecialWorldOpenDemo));
}

/**
 * @brief Check whether the Captain Toad played flag is already set.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is set; always false in Bowser's Fury mode.
 */
bool GameDataFlagFunction::isAlreadyPlayKinopioBrigade(GameDataHolderAccessor accessor) {
    return isOnGameFlag(accessor, toGameFlag(cGameFlagBit_PlayKinopioBrigade));
}

/**
 * @brief Set the Captain Toad played flag.
 * @param writer Writer to an initialized game-data holder.
 */
void GameDataFlagFunction::setPlayKinopioBrigade(GameDataHolderWriter writer) {
    onGameFlag(writer, toGameFlag(cGameFlagBit_PlayKinopioBrigade));
}

/**
 * @brief Check whether the casino-room played flag is already set.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is set; always false in Bowser's Fury mode.
 */
bool GameDataFlagFunction::isAlreadyPlayCasinoRoom(GameDataHolderAccessor accessor) {
    return isOnGameFlag(accessor, toGameFlag(cGameFlagBit_PlayCasinoRoom));
}

/**
 * @brief Set the casino-room played flag.
 * @param writer Writer to an initialized game-data holder.
 */
void GameDataFlagFunction::setPlayCasinoRoom(GameDataHolderWriter writer) {
    onGameFlag(writer, toGameFlag(cGameFlagBit_PlayCasinoRoom));
}

/**
 * @brief Check whether the Plessie played flag is already set.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is set; always false in Bowser's Fury mode.
 */
bool GameDataFlagFunction::isAlreadyPlayRidon(GameDataHolderAccessor accessor) {
    return isOnGameFlag(accessor, toGameFlag(cGameFlagBit_PlayRidon));
}

/**
 * @brief Set the Plessie played flag.
 * @param writer Writer to an initialized game-data holder.
 */
void GameDataFlagFunction::setPlayRidon(GameDataHolderWriter writer) {
    onGameFlag(writer, toGameFlag(cGameFlagBit_PlayRidon));
}

/**
 * @brief Clear the Plessie played flag.
 * @param writer Writer to an initialized game-data holder.
 */
void GameDataFlagFunction::resetPlayRidon(GameDataHolderWriter writer) {
    if (writer.getHolder()->isSingleMode()) {
        return;
    }

    getPlayingFile(writer)->offGameFlag(toGameFlag(cGameFlagBit_PlayRidon));
}

/**
 * @brief Check whether the course-select rocket unlock flag is already set.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is set; always false in Bowser's Fury mode.
 */
bool GameDataFlagFunction::isAlreadyOpenCourseSelectRocket(GameDataHolderAccessor accessor) {
    return isOnGameFlag(accessor, toGameFlag(cGameFlagBit_OpenCourseSelectRocket));
}

/**
 * @brief Set the course-select rocket unlock flag.
 * @param writer Writer to an initialized game-data holder.
 */
void GameDataFlagFunction::setOpenCourseSelectRocket(GameDataHolderWriter writer) {
    onGameFlag(writer, toGameFlag(cGameFlagBit_OpenCourseSelectRocket));
}

/**
 * @brief Check whether the first course-select rocket launch flag is already set.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is set; always false in Bowser's Fury mode.
 */
bool GameDataFlagFunction::isAlreadyFirstLaunchCourseSelectRocket(GameDataHolderAccessor accessor) {
    return isOnGameFlag(accessor, toGameFlag(cGameFlagBit_FirstLaunchCourseSelectRocket));
}

/**
 * @brief Set the first course-select rocket launch flag.
 * @param writer Writer to an initialized game-data holder.
 */
void GameDataFlagFunction::setFirstLaunchCourseSelectRocket(GameDataHolderWriter writer) {
    onGameFlag(writer, toGameFlag(cGameFlagBit_FirstLaunchCourseSelectRocket));
}

/**
 * @brief Check whether the first gatekeeper event flag is already set.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is set; always false in Bowser's Fury mode.
 */
bool GameDataFlagFunction::isAlreadyShowEventGateKeeperFirst(GameDataHolderAccessor accessor) {
    return isOnGameFlag(accessor, toGameFlag(cGameFlagBit_ShowEventGateKeeperFirst));
}

/**
 * @brief Set the first gatekeeper event flag.
 * @param writer Writer to an initialized game-data holder.
 */
void GameDataFlagFunction::setShowEventGateKeeperFirst(GameDataHolderWriter writer) {
    onGameFlag(writer, toGameFlag(cGameFlagBit_ShowEventGateKeeperFirst));
}

/**
 * @brief Check whether the second gatekeeper event flag is already set.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is set; always false in Bowser's Fury mode.
 */
bool GameDataFlagFunction::isAlreadyShowEventGateKeeperSecond(GameDataHolderAccessor accessor) {
    return isOnGameFlag(accessor, toGameFlag(cGameFlagBit_ShowEventGateKeeperSecond));
}

/**
 * @brief Set the second gatekeeper event flag.
 * @param writer Writer to an initialized game-data holder.
 */
void GameDataFlagFunction::setShowEventGateKeeperSecond(GameDataHolderWriter writer) {
    onGameFlag(writer, toGameFlag(cGameFlagBit_ShowEventGateKeeperSecond));
}

/**
 * @brief Check whether the Mystery House played flag is already set.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is set; always false in Bowser's Fury mode.
 */
bool GameDataFlagFunction::isAlreadyPlayMysteryHouse(GameDataHolderAccessor accessor) {
    return isOnGameFlag(accessor, toGameFlag(cGameFlagBit_PlayMysteryHouse));
}

/**
 * @brief Set the Mystery House played flag.
 * @param writer Writer to an initialized game-data holder.
 */
void GameDataFlagFunction::setPlayMysteryHouse(GameDataHolderWriter writer) {
    onGameFlag(writer, toGameFlag(cGameFlagBit_PlayMysteryHouse));
}

/**
 * @brief Check whether the Mystery House retry flag is already set.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is set; always false in Bowser's Fury mode.
 */
bool GameDataFlagFunction::isAlreadyRetryMysteryHouse(GameDataHolderAccessor accessor) {
    return isOnGameFlag(accessor, toGameFlag(cGameFlagBit_RetryMysteryHouse));
}

/**
 * @brief Set the Mystery House retry flag.
 * @param writer Writer to an initialized game-data holder.
 */
void GameDataFlagFunction::setRetryMysteryHouse(GameDataHolderWriter writer) {
    onGameFlag(writer, toGameFlag(cGameFlagBit_RetryMysteryHouse));
}

/**
 * @brief Check whether the Toad House played flag is already set.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the flag is set; always false in Bowser's Fury mode.
 */
bool GameDataFlagFunction::isAlreadyPlayKinopioHouse(GameDataHolderAccessor accessor) {
    return isOnGameFlag(accessor, toGameFlag(cGameFlagBit_PlayKinopioHouse));
}

/**
 * @brief Set the Toad House played flag.
 * @param writer Writer to an initialized game-data holder.
 */
void GameDataFlagFunction::setPlayKinopioHouse(GameDataHolderWriter writer) {
    onGameFlag(writer, toGameFlag(cGameFlagBit_PlayKinopioHouse));
}

/**
 * @brief Check whether the Sprixie Princess leaves the castle.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when her leaving course is cleared but the course-select rocket is not open yet.
 */
bool GameDataFlagFunction::isLeaveCastleFairyPrincess(GameDataHolderAccessor accessor) {
    if (accessor.getHolder()->isSingleMode()) {
        return false;
    }

    s32 courseId = GameDataFunction::calcStartFairyPrincessLeaveCastleCourseId(accessor);
    if (!CourseInfoFunction::isClear(accessor, courseId)) {
        return false;
    }

    return !isAlreadyOpenCourseSelectRocket(accessor);
}

/**
 * @brief Check whether the demo of the Sprixie Princess leaving the castle may play.
 * @param accessor Accessor to an initialized game-data holder.
 * @param worldId World whose Bowser castle is checked.
 * @return True when the castle is cleared and was not first cleared just now.
 */
bool GameDataFlagFunction::isEnableDemoLeaveCastleFairyPrincess(GameDataHolderAccessor accessor,
                                                                int worldId) {
    if (accessor.getHolder()->isSingleMode()) {
        return false;
    }

    s32 courseId = GameDataFunction::findKoopaCastleCourseId(accessor, worldId);
    if (GameDataFunction::isStageLastPlayAndFirstClear(accessor, courseId)) {
        return false;
    }

    return CourseInfoFunction::isClear(accessor, courseId);
}

/**
 * @brief Check whether the rock in front of the course-select rocket is shown.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True once the Crown world is open or about to open.
 */
bool GameDataFlagFunction::isEnableShowCourseSelectRocketRock(GameDataHolderAccessor accessor) {
    if (accessor.getHolder()->isSingleMode()) {
        return false;
    }

    if (isAlreadyOpenArrangeWorld(accessor)) {
        return true;
    }

    return isNewOpenArrangeWorld(accessor);
}
