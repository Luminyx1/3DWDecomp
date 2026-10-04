#include "System/PlayLogFunction.hpp"
#include "System/PlayLogData.hpp"
#include "System/Application.hpp"
#include "System/GameDataFile.hpp"
#include "Util/ControlUserUtil.hpp"
#include "Library/System/SystemKit.hpp"
#include <math/seadMathCalcCommon.h>

namespace {

/**
 * @brief Access the common play-log counters.
 * @param writer Accessor to an initialized game-data holder.
 * @return The common counters of the play log.
 */
inline PlayLogCommonData* getCommonData(GameDataHolderWriter writer) {
    return writer.getHolder()->getPlayLog()->getCommonData();
}

} // namespace

/**
 * @brief Access the common play-log storage.
 * @param accessor Accessor to an initialized game-data holder.
 * @return The requested counter count or buffer.
 */
int PlayLogFunction::getPlayLogDataNum(GameDataHolderAccessor accessor) {
    return accessor.getHolder()->getPlayLog()->getValueCount();
}

/**
 * @brief Access the common play-log storage.
 * @param accessor Accessor to an initialized game-data holder.
 * @return The requested counter count or buffer.
 */
u32* PlayLogFunction::getPlayLogData(GameDataHolderAccessor accessor) {
    return accessor.getHolder()->getPlayLog()->getValues();
}

/**
 * @brief Count another play session.
 * @param writer Accessor to an initialized game-data holder.
 */
void PlayLogFunction::setPlayStart(GameDataHolderWriter writer) {
    ++getCommonData(writer)->mPlayCount;
}

/**
 * @brief Accumulate time in the common play log.
 * @param writer Accessor to an initialized game-data holder.
 * @param time Elapsed play time to add in the caller's time units.
 */
void PlayLogFunction::setPlayTime(GameDataHolderWriter writer, int time) {
    getCommonData(writer)->mPlayTime += time;
}

/**
 * @brief Count a stage played with the given character.
 * @param writer Accessor to an initialized game-data holder.
 * @param characterType Character type of the playing user.
 */
void PlayLogFunction::setPlayChara(GameDataHolderWriter writer, int characterType) {
    switch (characterType) {
    case 0:
        ++getCommonData(writer)->mMarioCount;
        break;
    case 1:
        ++getCommonData(writer)->mLuigiCount;
        break;
    case 2:
        ++getCommonData(writer)->mPeachCount;
        break;
    case 3:
        ++getCommonData(writer)->mKinopioCount;
        break;
    case 4:
        ++getCommonData(writer)->mRosettaCount;
        break;
    default:
        break;
    }
}

/**
 * @brief Count a stage played with the given number of users.
 * @param writer Accessor to an initialized game-data holder.
 * @param num Number of active users.
 */
void PlayLogFunction::setUserNum(GameDataHolderWriter writer, int num) {
    switch (num) {
    case 1:
        ++getCommonData(writer)->mUserNum1Count;
        rc::getControlUserPortNumber(writer, rc::getActiveControlUserFirst(writer));
        break;
    case 2:
        ++getCommonData(writer)->mUserNum2Count;
        break;
    case 3:
        ++getCommonData(writer)->mUserNum3Count;
        break;
    case 4:
        ++getCommonData(writer)->mUserNum4Count;
        break;
    default:
        break;
    }
}

/**
 * @brief Check whether the current stage was played by a single user on the gamepad.
 * @param accessor Accessor to an initialized game-data holder.
 * @return True when the stage flag records single-user gamepad play.
 */
bool PlayLogFunction::isUseDrcOneUser(GameDataHolderAccessor accessor) {
    if (accessor.getHolder()->isSingleMode()) {
        return false;
    }

    return *accessor.getHolder()->getPlayingFile()->getPlayLogStageFlag() & 1;
}

/**
 * @brief Record that the assist player was used in the current stage.
 * @param writer Accessor to an initialized game-data holder.
 */
void PlayLogFunction::useAssistPlayer(GameDataHolderWriter writer) {
    if (writer.getHolder()->isSingleMode()) {
        return;
    }

    *writer.getHolder()->getPlayingFile()->getPlayLogStageFlag() |= 2;
}

/**
 * @brief Record that the cross key was used in the current stage.
 * @param writer Accessor to an initialized game-data holder.
 */
void PlayLogFunction::useCrossKey(GameDataHolderWriter writer) {
    if (writer.getHolder()->isSingleMode()) {
        return;
    }

    *writer.getHolder()->getPlayingFile()->getPlayLogStageFlag() |= 4;
}

/**
 * @brief Record that the touch panel was used in the current stage.
 * @param writer Accessor to an initialized game-data holder.
 */
void PlayLogFunction::useTouchPanel(GameDataHolderWriter writer) {
    if (writer.getHolder()->isSingleMode()) {
        return;
    }

    *writer.getHolder()->getPlayingFile()->getPlayLogStageFlag() |= 8;
}

/**
 * @brief Record that the camera was rotated in the current stage.
 * @param writer Accessor to an initialized game-data holder.
 */
void PlayLogFunction::useCameraRotate(GameDataHolderWriter writer) {
    if (writer.getHolder()->isSingleMode()) {
        return;
    }

    *writer.getHolder()->getPlayingFile()->getPlayLogStageFlag() |= 0x10;
}

/**
 * @brief Store the Miiverse setting in the common play log.
 * @param writer Accessor to an initialized game-data holder.
 * @param isEnable True when Miiverse posting is enabled.
 */
void PlayLogFunction::setMiiverseFlag(GameDataHolderWriter writer, bool isEnable) {
    getCommonData(writer)->mMiiverseFlag = isEnable;
}

/**
 * @brief Count a Miiverse post.
 * @param writer Accessor to an initialized game-data holder.
 */
void PlayLogFunction::setPostMiiverse(GameDataHolderWriter writer) {
    ++getCommonData(writer)->mPostMiiverseCount;
}

/**
 * @brief Record that a player entered during the current stage.
 * @param writer Accessor to an initialized game-data holder.
 */
void PlayLogFunction::setPlayerEntry(GameDataHolderWriter writer) {
    if (writer.getHolder()->isSingleMode()) {
        return;
    }

    *writer.getHolder()->getPlayingFile()->getPlayLogStageFlag() |= 0x20;
}

/**
 * @brief Accumulate the current stage's usage flags into the common play log.
 * @param writer Accessor to an initialized game-data holder.
 */
void PlayLogFunction::onStageEnd(GameDataHolderWriter writer) {
    if (writer.getHolder()->isSingleMode()) {
        return;
    }

    u32* pFlag = writer.getHolder()->getPlayingFile()->getPlayLogStageFlag();

    if (*pFlag & 2) {
        ++getCommonData(writer)->mUseAssistPlayerCount;
    }

    if (*pFlag & 4) {
        ++getCommonData(writer)->mUseCrossKeyCount;
    }

    if (*pFlag & 8) {
        ++getCommonData(writer)->mUseTouchPanelCount;
    }

    if (*pFlag & 0x10) {
        ++getCommonData(writer)->mUseCameraRotateCount;
    }

    if (*pFlag & 0x20) {
        ++getCommonData(writer)->mPlayerEntryCount;
    }
}

/**
 * @brief Count a start of the given course.
 * @param writer Accessor to an initialized game-data holder.
 * @param courseId Course being started.
 */
void PlayLogFunction::startStage(GameDataHolderWriter writer, int courseId) {
    u32* pData = writer.getHolder()->getPlayLog()->tryGetCourseData(courseId);

    if (pData != nullptr) {
        ++pData[0];
    }
}

/**
 * @brief Record a clear of the given course.
 * @param writer Accessor to an initialized game-data holder.
 * @param courseId Cleared course.
 * @param time Time-attack count to accumulate.
 * @param greenStarNum Number of green stars acquired.
 * @param isAcquireIllustItem True when the stamp was acquired.
 * @param isUseAssistBlock True when the assist block was used.
 */
void PlayLogFunction::clearStage(GameDataHolderWriter writer, int courseId, int time,
                                 int greenStarNum, bool isAcquireIllustItem,
                                 bool isUseAssistBlock) {
    s32* pData = reinterpret_cast<s32*>(
        writer.getHolder()->getPlayLog()->tryGetCourseData(courseId));

    if (pData != nullptr) {
        pData[4] += time;
        ++pData[1];
        pData[2] = sead::Mathi::max(pData[2], greenStarNum);
        pData[3] = sead::Mathi::max(pData[3], isAcquireIllustItem);
        pData[5] = sead::Mathi::max(pData[5], isUseAssistBlock);
    }
}

/**
 * @brief Record the green-star count of the given course.
 * @param writer Accessor to an initialized game-data holder.
 * @param courseId Course to update.
 * @param num Number of green stars acquired.
 */
void PlayLogFunction::setGreenStarNum(GameDataHolderWriter writer, int courseId, int num) {
    s32* pData = reinterpret_cast<s32*>(
        writer.getHolder()->getPlayLog()->tryGetCourseData(courseId));

    if (pData != nullptr) {
        pData[2] = sead::Mathi::max(pData[2], num);
    }
}

/**
 * @brief Access the application's system kit for the action library.
 * @return The system kit created by the application.
 */
al::SystemKit* alProjectInterface::getSystemKit() {
    return Application::instance()->getSystemKit();
}
