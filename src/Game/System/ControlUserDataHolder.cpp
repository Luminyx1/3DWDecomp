#include "System/ControlUserDataHolder.hpp"
#include "System/GameDataConst.hpp"
#include "Util/PlayerUtil.hpp"

/**
 * @brief Creates records for the four local users.
 * @param singleMode Whether every user starts with Mario rather than a distinct character.
 */
ControlUserDataHolder::ControlUserDataHolder(bool singleMode) { initialize(singleMode); }

/**
 * @brief Assigns controller ports and initial character choices to inactive users.
 * @param singleMode True to choose Mario for all users; false to choose characters 0 through 3.
 */
void ControlUserDataHolder::initialize(bool singleMode) {
    const int* pPorts = GameDataConst::getPadPortList();
    for (s32 i = 0; i < 4; ++i) {
        auto& rUser = mUsers[i];
        rUser.mUserIndex = i;
        rUser.mPadPort = pPorts[i];
        rUser.mState = ControlUserData::Deactive;
        rUser.mCharacterType = singleMode ? 0 : i;
        rUser.mSavedCharacterType = rUser.mCharacterType;
        rUser.mFigureType = rc::getPlayerFigureTypeDefault();
    }
}

/**
 * @brief Copies all four user records.
 * @param pOther Non-null source holder; self-copy is permitted.
 */
void ControlUserDataHolder::copy(const ControlUserDataHolder* pOther) {
    for (s32 i = 0; i < 4; ++i) {
        mUsers[i] = pOther->mUsers[i];
    }
}

/**
 * @brief Counts users whose players are alive.
 * @return Number of alive users, from 0 to 4.
 */
s32 ControlUserDataHolder::calcPlayablePlayerNum() const {
    s32 count = 0;
    for (const auto& rUser : mUsers) {
        if (rUser.mState == ControlUserData::Alive) {
            ++count;
        }
    }
    return count;
}

/**
 * @brief Gets the selected user record.
 * @param userIndex User slot from 0 to 3; not bounds-checked.
 * @return Pointer to the stored user record.
 */
const ControlUserData* ControlUserDataHolder::getControlUserData(s32 userIndex) const {
    return &mUsers[userIndex];
}

/**
 * @brief Gets the selected user record.
 * @param userIndex User slot from 0 to 3; not bounds-checked.
 * @return Pointer to the stored user record.
 */
ControlUserData* ControlUserDataHolder::getControlUserDataPtr(s32 userIndex) { return &mUsers[userIndex]; }

/**
 * @brief Restores default figures and revives dead users, preserving inactive users.
 */
void ControlUserDataHolder::recoverGameOver() {
    for (auto& rUser : mUsers) {
        rUser.mFigureType = rc::getPlayerFigureTypeDefault();
        if (rUser.mState == ControlUserData::DeadInStage) {
            rUser.mState = ControlUserData::Alive;
        }
    }
}

/**
 * @brief Restores the default figure for each user.
 */
void ControlUserDataHolder::resetFigureType() {
    for (auto& rUser : mUsers) {
        rUser.mFigureType = rc::getPlayerFigureTypeDefault();
    }
}

/**
 * @brief Restores distinct character choices and default figures.
 */
void ControlUserDataHolder::resetPlayerModel() {
    for (s32 i = 0; i < 4; ++i) {
        mUsers[i].mCharacterType = i;
        mUsers[i].mFigureType = rc::getPlayerFigureTypeDefault();
    }
}

/**
 * @brief Handles character-selection startup; no action is required in this build.
 */
void ControlUserDataHolder::startCharacterSelect() {}

/**
 * @brief Activates a user if the requested character is not owned by another alive player.
 * @param userIndex User slot from 0 to 3.
 * @param characterType Desired character, or -1 to retain the current choice.
 * @return True when the user was activated.
 */
bool ControlUserDataHolder::entryPlayer(s32 userIndex, s32 characterType) {
    auto& rUser = mUsers[userIndex];
    if (characterType == -1) {
        characterType = rUser.mCharacterType;
    }
    for (s32 i = 0; i < 4; ++i) {
        auto& rOther = mUsers[i];
        if (rOther.mCharacterType == characterType) {
            if (i == userIndex) {
                break;
            }
            if (rOther.mState == ControlUserData::Alive) {
                return false;
            }
            rOther.mCharacterType = rUser.mCharacterType;
        }
    }
    rUser.mState = ControlUserData::Alive;
    rUser.mCharacterType = characterType;
    return true;
}

/**
 * @brief Marks a user inactive.
 * @param userIndex User slot from 0 to 3; not bounds-checked.
 */
void ControlUserDataHolder::retirePlayer(s32 userIndex) {
    mUsers[userIndex].mState = ControlUserData::Deactive;
}

/**
 * @brief Marks all four users inactive.
 */
void ControlUserDataHolder::resetPlayerAll() {
    for (auto& rUser : mUsers) {
        rUser.mState = ControlUserData::Deactive;
    }
}

/**
 * @brief Updates the selected player appearance identifier.
 * @param userIndex User slot from 0 to 3; not bounds-checked.
 * @param characterType New appearance identifier; stored without validation.
 */
void ControlUserDataHolder::setPlayerModel(s32 userIndex, s32 characterType) {
    mUsers[userIndex].mCharacterType = characterType;
}

/**
 * @brief Updates the selected player appearance identifier.
 * @param userIndex User slot from 0 to 3; not bounds-checked.
 * @param figureType New appearance identifier; stored without validation.
 */
void ControlUserDataHolder::setPlayerFigureType(s32 userIndex, s32 figureType) {
    mUsers[userIndex].mFigureType = figureType;
}

/**
 * @brief Records each current character as the saved character choice.
 */
void ControlUserDataHolder::onSave() {
    for (auto& rUser : mUsers) {
        rUser.mSavedCharacterType = rUser.mCharacterType;
    }
}
