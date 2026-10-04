#include "Util/ControlUserUtil.hpp"
#include "Library/Controller/InputFunction.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorSensorUtil.hpp"
#include "Library/Player/PlayerUtil.hpp"
#include "System/ControlUserData.hpp"
#include "System/ControlUserDataHolder.hpp"
#include "System/Data/SingleModeData.hpp"
#include "System/GameDataConst.hpp"
#include "System/GameDataFile.hpp"
#include "System/GameDataHolderAccessor.hpp"
#include "System/GameDataHolderWriter.hpp"
#include "Util/DrcUtil.hpp"
#include "Util/ProjectMsgUtil.hpp"

/**
 * @brief Player actor; only the members used by this unit are declared.
 */
class PlayerActor : public al::LiveActor {
  public:
    s32 getInputPort() const;
};

namespace LayoutFontUtil {
const char16_t* getPictureFontPlayer(s32 characterType);
} // namespace LayoutFontUtil

namespace {

/// Number of local control users.
constexpr s32 cControlUserNum = 4;

/// Character types at or above this value address a control user slot directly.
constexpr s32 cCharacterTypeUserSlotBase = 5;

/**
 * @brief Fetch the save file currently in use for the active game mode.
 * @param accessor Accessor to a valid game-data holder.
 * @return The Bowser's Fury file in single mode, else the 3D World file.
 */
GameDataFileBase* getPlayingFileBase(GameDataHolderAccessor accessor) {
    GameDataHolder* pHolder = accessor.getHolder();
    if (pHolder->isSingleMode()) {
        return pHolder->getSingleFile();
    }

    return pHolder->getPlayingFile();
}

/**
 * @brief Read one control user's record from the playing file.
 * @param accessor Accessor to a valid game-data holder.
 * @param userId Control user index.
 * @return The user's record.
 */
const ControlUserData* getUserData(GameDataHolderAccessor accessor, s32 userId) {
    return getPlayingFileBase(accessor)->getControlUserData(userId);
}

/**
 * @brief Access one control user's record of the playing file for writing.
 * @param writer Writer to a valid game-data holder.
 * @param userId Control user index.
 * @return The user's record.
 */
ControlUserData* getUserDataPtr(GameDataHolderWriter writer, s32 userId) {
    return getPlayingFileBase(writer)->getControlUserDataHolder()->getControlUserDataPtr(userId);
}

} // namespace

/**
 * @brief Number of local control users.
 * @return Always 4.
 */
s32 rc::getControlUserNumMax() {
    return cControlUserNum;
}

/**
 * @brief Pad port assigned to a user index by default.
 * @param userId Control user index.
 * @return The pad port of that index.
 */
s32 rc::getPadPortByUserId(s32 userId) {
    return GameDataConst::getPadPortList()[userId];
}

/**
 * @brief Find which default user index uses a pad port.
 * @param port Pad port to look up.
 * @return The user index, or 0 when the port is not listed.
 */
s32 rc::calcControlUserIdByPortNum(s32 port) {
    const s32* pPorts = GameDataConst::getPadPortList();
    for (s32 i = 0; i < cControlUserNum; i++) {
        if (pPorts[i] == port) {
            return i;
        }
    }

    return 0;
}

/**
 * @brief Find the touch-panel port matching a pad port.
 * @param port Pad port to look up.
 * @return The touch-panel port, falling back to that of the first slot; -1 for a negative port.
 */
s32 rc::calcTouchPanelPortByPortNum(s32 port) {
    if (port < 0) {
        return -1;
    }

    s32 portNum = GameDataConst::getPadPortListNum();
    const s32* pPorts = GameDataConst::getPadPortList();
    s32 index = 0;
    for (s32 i = 0; i < portNum; i++) {
        if (pPorts[i] == port) {
            index = i;
            break;
        }
    }

    s32 touchPanelPort = al::getTouchPanelPort(index);
    if (touchPanelPort == -1) {
        return al::getTouchPanelPort(0);
    }

    return touchPanelPort;
}

/**
 * @brief Exchange the players of two users while they keep their pad ports.
 * @param writer Writer to a valid game-data holder.
 * @param userIdA User whose controller gets disconnected.
 * @param userIdB User that takes over the player of userIdA.
 */
void rc::changeUserPort(GameDataHolderWriter writer, s32 userIdA, s32 userIdB) {
    ControlUserData* pUserA = getUserDataPtr(writer, userIdA);
    ControlUserData* pUserB = getUserDataPtr(writer, userIdB);

    ControlUserData::State state = pUserA->mState;
    pUserA->mState = pUserB->mState;
    pUserB->mState = state;

    s32 characterType = pUserA->mCharacterType;
    pUserA->mCharacterType = pUserB->mCharacterType;
    pUserB->mCharacterType = characterType;

    s32 figureType = pUserA->mFigureType;
    pUserA->mFigureType = pUserB->mFigureType;
    pUserB->mFigureType = figureType;

    GameDataFile* pFile = writer.getHolder()->getPlayingFile();
    s32 bestScoreUserId = pFile->tryGetLastStageBestScoreUserID();
    if (bestScoreUserId == userIdA) {
        pFile->setBestScoreUserId(userIdB);
    } else if (bestScoreUserId == userIdB) {
        pFile->setBestScoreUserId(-1);
    }

    al::setPadDisconnect(pUserA->mPadPort);
}

/**
 * @brief Pad port of the active user that comes first in pad-port order.
 * @param accessor Accessor to a valid game-data holder.
 * @return The pad port; undefined when no user is active.
 */
s32 rc::calcPadPortByFirstActiveUser(GameDataHolderAccessor accessor) {
    s32 userIds[cControlUserNum];
    findActiveUserIdList(userIds, accessor);
    s32 userId = userIds[0];
    return GameDataConst::getPadPortList()[userId];
}

/**
 * @brief List the active users sorted by the order of their pad ports.
 * @param pUserIds Output for up to four user indices, or nullptr to only count.
 * @param accessor Accessor to a valid game-data holder.
 * @return The number of active users.
 */
s32 rc::findActiveUserIdList(s32* pUserIds, GameDataHolderAccessor accessor) {
    s32 activeUserIds[cControlUserNum];
    s32 activeUserNum = 0;
    for (s32 i = 0; i < cControlUserNum; i++) {
        if (getUserData(accessor, i)->isActive()) {
            activeUserIds[activeUserNum++] = i;
        }
    }

    if (pUserIds != nullptr) {
        s32 portNum = GameDataConst::getPadPortListNum();
        const s32* pPorts = GameDataConst::getPadPortList();
        s32 listNum = 0;
        for (s32 i = 0; i < portNum; i++) {
            for (s32 j = 0; j < activeUserNum; j++) {
                s32 userId = activeUserIds[j];
                if (getUserData(accessor, userId)->mPadPort == pPorts[i]) {
                    pUserIds[listNum] = userId;
                    listNum++;
                    break;
                }
            }
        }
    }

    return activeUserNum;
}

/**
 * @brief Pad port of the active user that comes first in pad-port order.
 * @param accessor Accessor to a valid game-data holder.
 * @return The pad port, or -1 when no user is active.
 */
s32 rc::tryCalcPadPortByFirstActiveUser(GameDataHolderAccessor accessor) {
    s32 userIds[cControlUserNum];
    if (findActiveUserIdList(userIds, accessor) < 1) {
        return -1;
    }

    s32 userId = userIds[0];
    return GameDataConst::getPadPortList()[userId];
}

/**
 * @brief Number of selectable character types.
 * @param accessor Accessor to a valid game-data holder.
 * @return The character type count.
 */
s32 rc::calcCharacterTypeNumMax(GameDataHolderAccessor accessor) {
    return accessor.getHolder()->calcCharacterTypeNumMax();
}

/**
 * @brief Count the participating users.
 * @param accessor Accessor to a valid game-data holder.
 * @return The number of active users.
 */
s32 rc::getActiveControlUserNum(GameDataHolderAccessor accessor) {
    s32 num = 0;
    for (s32 i = 0; i < cControlUserNum; i++) {
        num += getUserData(accessor, i)->isActive();
    }

    return num;
}

/**
 * @brief Collect the pad ports of the participating users.
 * @param accessor Accessor to a valid game-data holder.
 * @return One bit per pad port of an active user.
 */
u64 rc::getActiveInputPortList(GameDataHolderAccessor accessor) {
    u16 portList = 0;
    for (s32 i = 0; i < cControlUserNum; i++) {
        const ControlUserData* pUser = getUserData(accessor, i);
        if (pUser->isActive()) {
            portList |= 1 << pUser->mPadPort;
        }
    }

    return portList;
}

/**
 * @brief Pad port of a user.
 * @param accessor Accessor to a valid game-data holder.
 * @param userId Control user index.
 * @return The user's pad port.
 */
s32 rc::getControlUserPortNumber(GameDataHolderAccessor accessor, s32 userId) {
    return getUserData(accessor, userId)->mPadPort;
}

/**
 * @brief Position of a user among the active users in pad-port order.
 * @param accessor Accessor to a valid game-data holder.
 * @param userId Control user index.
 * @return The position, or 0 when the user is not active.
 */
s32 rc::calcActiveUserNumInOrder(GameDataHolderAccessor accessor, s32 userId) {
    s32 userIds[cControlUserNum];
    s32 num = findActiveUserIdList(userIds, accessor);
    for (s32 i = 0; i < num; i++) {
        if (userIds[i] == userId) {
            return i;
        }
    }

    return 0;
}

/**
 * @brief Check whether a user participates.
 * @param accessor Accessor to a valid game-data holder.
 * @param userId Control user index.
 * @return True when the user is active.
 */
bool rc::isActiveControlUser(GameDataHolderAccessor accessor, s32 userId) {
    return getUserData(accessor, userId)->isActive();
}

/**
 * @brief Check whether a user's player died in the current stage.
 * @param accessor Accessor to a valid game-data holder.
 * @param userId Control user index.
 * @return True when the user is dead in the stage.
 */
bool rc::isDeadControlUserInStage(GameDataHolderAccessor accessor, s32 userId) {
    return getUserData(accessor, userId)->isDeadInStage();
}

/**
 * @brief Revive a user that died in the current stage.
 * @param writer Writer to a valid game-data holder.
 * @param userId Control user index.
 */
void rc::resetDeadFlagControlUserInStage(GameDataHolderWriter writer, s32 userId) {
    getUserDataPtr(writer, userId)->resetDeadInStage();
}

/**
 * @brief Check whether any user died in the current stage.
 * @param accessor Accessor to a valid game-data holder.
 * @return True when at least one user is dead in the stage.
 */
bool rc::isExistDeadlUserInStage(GameDataHolderAccessor accessor) {
    for (s32 i = 0; i < cControlUserNum; i++) {
        if (getUserData(accessor, i)->isDeadInStage()) {
            return true;
        }
    }

    return false;
}

/**
 * @brief Lowest index of a participating user.
 * @param accessor Accessor to a valid game-data holder.
 * @return The user index, or -1 when no user is active.
 */
s32 rc::getActiveControlUserFirst(GameDataHolderAccessor accessor) {
    for (s32 i = 0; i < cControlUserNum; i++) {
        if (getUserData(accessor, i)->isActive()) {
            return i;
        }
    }

    return -1;
}

/**
 * @brief Character played by a user.
 * @param accessor Accessor to a valid game-data holder.
 * @param userId Control user index.
 * @return The character type.
 */
s32 rc::getControlUserCharacterType(GameDataHolderAccessor accessor, s32 userId) {
    return getUserData(accessor, userId)->mCharacterType;
}

/**
 * @brief Name of the character played by a user.
 * @param accessor Accessor to a valid game-data holder.
 * @param userId Control user index.
 * @return The character name.
 */
const char* rc::getControlUserCharacterName(GameDataHolderAccessor accessor, s32 userId) {
    return GameDataConst::getPlayerCharacterName(getUserData(accessor, userId)->mCharacterType);
}

/**
 * @brief Picture-font glyph of the character played by a user.
 * @param accessor Accessor to a valid game-data holder.
 * @param userId Control user index.
 * @return The picture-font text.
 */
const char16_t* rc::getControlUserCharacterPictureFont(GameDataHolderAccessor accessor,
                                                        s32 userId) {
    return LayoutFontUtil::getPictureFontPlayer(getUserData(accessor, userId)->mCharacterType);
}

/**
 * @brief Figure (amiibo) type of a user.
 * @param accessor Accessor to a valid game-data holder.
 * @param userId Control user index.
 * @return The figure type.
 */
s32 rc::getControlUserFigureType(GameDataHolderAccessor accessor, s32 userId) {
    return getUserData(accessor, userId)->mFigureType;
}

/**
 * @brief Set the figure (amiibo) type of a user.
 * @param writer Writer to a valid game-data holder.
 * @param userId Control user index.
 * @param figureType Figure type to store.
 */
void rc::setControlUserFigureType(GameDataHolderWriter writer, s32 userId, s32 figureType) {
    getUserDataPtr(writer, userId)->mFigureType = figureType;
}

/**
 * @brief Count the occupied pad ports listed before a user's port.
 * @param accessor Accessor to a valid game-data holder.
 * @param userId Control user index.
 * @return The display position, or 0 when the user's port is not listed.
 */
s32 rc::calcControlUserDisplayOrder(GameDataHolderAccessor accessor, s32 userId) {
    s32 port = getUserData(accessor, userId)->mPadPort;
    const s32* pPorts = GameDataConst::getPadPortList();
    s32 order = 0;
    for (s32 i = 0; i < GameDataConst::getPadPortListNum(); i++) {
        if (pPorts[i] == port) {
            return order;
        }

        if (tryCalcControlUserIdFromPortNum(accessor, pPorts[i]) >= 0) {
            order++;
        }
    }

    return 0;
}

/**
 * @brief Find the active user holding a pad port.
 * @param accessor Accessor to a valid game-data holder.
 * @param port Pad port to look up.
 * @return The user index, or -1 when no active user holds the port.
 */
s32 rc::tryCalcControlUserIdFromPortNum(GameDataHolderAccessor accessor, s32 port) {
    for (s32 i = 0; i < cControlUserNum; i++) {
        const ControlUserData* pUser = getUserData(accessor, i);
        if (pUser->mPadPort == port && pUser->isActive()) {
            return i;
        }
    }

    return -1;
}

/**
 * @brief Find the active user holding a pad port.
 * @param accessor Accessor to a valid game-data holder.
 * @param port Pad port to look up.
 * @return The user index, or -1 when no active user holds the port.
 */
s32 rc::calcControlUserIdFromPortNum(GameDataHolderAccessor accessor, s32 port) {
    return tryCalcControlUserIdFromPortNum(accessor, port);
}

/**
 * @brief Find the active user playing a character.
 * @param accessor Accessor to a valid game-data holder.
 * @param characterType Character type, or 5 + user index for a direct user slot.
 * @param isAllowUserSlot True to accept direct user-slot values.
 * @return The user index, or -1 when no active user matches.
 */
s32 rc::calcControlUserIdByCharacterType(GameDataHolderAccessor accessor, s32 characterType,
                                         bool isAllowUserSlot) {
    return tryCalcControlUserIdByCharacterType(accessor, characterType, isAllowUserSlot);
}

/**
 * @brief Find the active user playing a character.
 * @param accessor Accessor to a valid game-data holder.
 * @param characterType Character type, or 5 + user index for a direct user slot.
 * @param isAllowUserSlot True to accept direct user-slot values.
 * @return The user index, or -1 when no active user matches.
 */
s32 rc::tryCalcControlUserIdByCharacterType(GameDataHolderAccessor accessor, s32 characterType,
                                            bool isAllowUserSlot) {
    if (characterType >= cCharacterTypeUserSlotBase && isAllowUserSlot) {
        s32 userId = characterType - cCharacterTypeUserSlotBase;
        return getUserData(accessor, userId)->isActive() ? userId : -1;
    }

    for (s32 i = 0; i < cControlUserNum; i++) {
        const ControlUserData* pUser = getUserData(accessor, i);
        if (pUser->mCharacterType == characterType && pUser->isActive()) {
            return i;
        }
    }

    return -1;
}

/**
 * @brief Find the user controlling a player actor.
 * @param pActor Actor to check.
 * @return The user index, or -1 when the actor is not a player or no user holds its port.
 */
s32 rc::tryFindControlUserId(const al::LiveActor* pActor) {
    if (!alPlayerFunction::isPlayerActor(pActor)) {
        return -1;
    }

    return tryCalcControlUserIdFromPortNum(GameDataHolderAccessor(pActor),
                                           static_cast<const PlayerActor*>(pActor)->getInputPort());
}

/**
 * @brief Find the user controlling a player actor.
 * @param pActor Non-null player actor.
 * @return The user index, or -1 when no user holds its port.
 */
s32 rc::findControlUserId(const al::LiveActor* pActor) {
    return tryCalcControlUserIdFromPortNum(GameDataHolderAccessor(pActor),
                                           static_cast<const PlayerActor*>(pActor)->getInputPort());
}

/**
 * @brief Find the user controlling the player owning a sensor.
 * @param pSensor Sensor of a player actor.
 * @return The user index, or -1 when no user holds its port.
 */
s32 rc::findControlUserId(const al::HitSensor* pSensor) {
    const al::LiveActor* pActor = al::getSensorHost(pSensor);
    return tryCalcControlUserIdFromPortNum(GameDataHolderAccessor(pActor),
                                           static_cast<const PlayerActor*>(pActor)->getInputPort());
}

/**
 * @brief Find the user controlling the actor owning a sensor.
 * @param pSensor Sensor to check.
 * @return The user index, or -1 when the host is not a player or no user holds its port.
 */
s32 rc::tryFindControlUserId(const al::HitSensor* pSensor) {
    return tryFindControlUserId(al::getSensorHost(pSensor));
}

/**
 * @brief Ask a sensor which user it belongs to.
 * @param pSensor Sensor to ask, or nullptr.
 * @return The first user index the sensor answers for, or -1.
 */
s32 rc::findRelativeControlUserId(al::HitSensor* pSensor) {
    return tryFindRelativeControlUserId(pSensor);
}

/**
 * @brief Ask a sensor which user it belongs to.
 * @param pSensor Sensor to ask, or nullptr.
 * @return The first user index the sensor answers for, or -1.
 */
s32 rc::tryFindRelativeControlUserId(al::HitSensor* pSensor) {
    if (pSensor == nullptr) {
        return -1;
    }

    for (s32 i = 0; i < cControlUserNum; i++) {
        if (sendMsgAskControlUserId(pSensor, i)) {
            return i;
        }
    }

    return -1;
}

/**
 * @brief Ask a sensor which users it belongs to.
 * @param pSensor Non-null sensor to ask.
 * @return One bit per user index the sensor answers for.
 */
u32 rc::tryFindRelativeControlUserIdBitFlag(al::HitSensor* pSensor) {
    u32 flag = 0;
    for (s32 i = 0; i < cControlUserNum; i++) {
        if (sendMsgAskControlUserId(pSensor, i)) {
            flag |= 1 << i;
        }
    }

    return flag;
}

/**
 * @brief Figure (amiibo) type of the user a sensor belongs to.
 * @param accessor Accessor to a valid game-data holder.
 * @param pSensor Sensor to ask, or nullptr.
 * @return The figure type, or -1 when the sensor belongs to no user.
 */
s32 rc::tryGetRelativeControlUserFigureType(GameDataHolderAccessor accessor,
                                            al::HitSensor* pSensor) {
    s32 userId = tryFindRelativeControlUserId(pSensor);
    if (userId < 0) {
        return -1;
    }

    return getUserData(accessor, userId)->mFigureType;
}

/**
 * @brief Find the user whose player is touched through the GamePad screen.
 * @param pActor Actor checking for a touch.
 * @param pPointer Screen pointer of the touch.
 * @return The user index, or -1 when no player sensor is touched.
 */
s32 rc::tryFindRelativeControlUserId(const al::LiveActor* pActor, al::ScreenPointer* pPointer) {
    return tryFindRelativeControlUserId(DrcFunction::tryFindDrcPlayerSensor(pActor, pPointer));
}
