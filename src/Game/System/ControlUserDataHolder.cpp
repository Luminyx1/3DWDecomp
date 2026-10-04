#include "System/ControlUserDataHolder.hpp"
#include "Library/Math/MathUtil.hpp"
#include "System/GameDataConst.hpp"
#include "System/GameDataHolder.hpp"
#include "Util/PlayerUtil.hpp"
#include <stream/seadStream.h>

namespace {

/// Number of character slots tracked while resolving duplicate character choices.
constexpr s32 cCharacterSlotNum = 5;

/// Number of retries when drawing a random unused character before falling back to a linear scan.
constexpr s32 cShuffleRetryNum = 1023;

/// Number of four-element derangements used when exactly four characters are available.
constexpr s32 cDerangementNum = 9;

/// Every permutation of four users that moves each user to a different character.
const s32 cDerangements[cDerangementNum][4] = {
    {1, 0, 3, 2}, {1, 2, 3, 0}, {1, 3, 0, 2}, {2, 0, 3, 1}, {2, 3, 0, 1},
    {2, 3, 1, 0}, {3, 0, 1, 2}, {3, 2, 0, 1}, {3, 2, 1, 0},
};

/**
 * @brief Serialized form of one user's character choice.
 */
struct ControlUserSaveData {
    s32 mCharacterType;
    s32 mFigureType;
    u64 mReserved = 0;
};

static_assert(sizeof(ControlUserSaveData) == 0x10);

} // namespace

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
 * @brief Gives every user a different character than the one they currently use.
 * @param pHolder Game data used to count unlocked characters, or nullptr to use the global maximum.
 */
void ControlUserDataHolder::shufflePlayerModel(GameDataHolder* pHolder) {
    al::initRandomSeedByTick();
    al::initRandomSeedByTickNonSync();
    s32 characterNum =
        pHolder != nullptr ? pHolder->calcCharacterTypeNumMax() : rc::getPlayerCharacterNumMax();

    // Marks which characters are taken, then becomes scratch storage for the later passes.
    s32 slots[cCharacterSlotNum] = {};
    s32 duplicateNum = 0;
    for (auto& rUser : mUsers) {
        if (slots[rUser.mCharacterType] != 0) {
            rUser.mCharacterType = -1;
            ++duplicateNum;
        } else {
            slots[rUser.mCharacterType] = 1;
        }
    }

    if (duplicateNum != 0) {
        for (auto& rUser : mUsers) {
            if (rUser.mCharacterType != -1) {
                continue;
            }

            for (s32 i = 0; i < cCharacterSlotNum; ++i) {
                if (slots[i] == 0) {
                    rUser.mCharacterType = i;
                    slots[i] = 1;
                    break;
                }
            }
        }
    }

    if (characterNum == 4) {
        s32 index = al::getRandom(cDerangementNum);
        for (s32 i = 0; i < 4; ++i) {
            slots[i] = mUsers[i].mCharacterType;
        }

        for (s32 i = 0; i < 4; ++i) {
            mUsers[i].mCharacterType = slots[cDerangements[index][i]];
        }

        return;
    }

    for (s32 i = 0; i < cCharacterSlotNum; ++i) {
        slots[i] = -1;
    }

    for (s32 i = 0; i < 4; ++i) {
        auto& rUser = mUsers[i];
        s32 current = rUser.mCharacterType;
        bool isFound = false;
        s32 retry = cShuffleRetryNum + 1;
        while (--retry != 0) {
            s32 candidate = al::getRandom(characterNum - 1);
            if (current <= candidate) {
                ++candidate;
            }

            if (slots[candidate] == -1) {
                slots[candidate] = i;
                rUser.mCharacterType = candidate;
                isFound = true;
                break;
            }
        }

        if (!isFound) {
            for (s32 j = 0; j < characterNum; ++j) {
                if (slots[j] == -1) {
                    slots[j] = i;
                    rUser.mCharacterType = j;
                    break;
                }
            }
        }
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
 * @brief Finds the alive user playing a character.
 * @param characterType Character to search for.
 * @return Index of the first alive user with that character, or -1 if none.
 */
s32 ControlUserDataHolder::tryCalcControlUserIdByCharacterType(s32 characterType) {
    for (s32 i = 0; i < 4; ++i) {
        if (mUsers[i].mCharacterType == characterType && mUsers[i].mState == ControlUserData::Alive) {
            return i;
        }
    }

    return -1;
}

/**
 * @brief Loads each user's character and figure from save data.
 * @param pStream Non-null input stream positioned at four 16-byte user records.
 * @return Always true.
 */
bool ControlUserDataHolder::readFromStream(sead::ReadStream* pStream) {
    ControlUserSaveData data;
    for (auto& rUser : mUsers) {
        pStream->readMemBlock(&data, sizeof(data));
        rUser.mCharacterType = data.mCharacterType;
        rUser.mFigureType = data.mFigureType;
    }

    return true;
}

/**
 * @brief Stores each user's character and figure as save data.
 * @param pStream Non-null output stream.
 * @param isSkip True to advance past the four records without writing them.
 */
void ControlUserDataHolder::writeToStream(sead::WriteStream* pStream, bool isSkip) const {
    if (isSkip) {
        for (s32 i = 0; i < 4; ++i) {
            pStream->skip(sizeof(ControlUserSaveData));
        }

        return;
    }

    ControlUserSaveData data;
    for (const auto& rUser : mUsers) {
        data.mCharacterType = rUser.mCharacterType;
        data.mFigureType = rUser.mFigureType;
        pStream->writeMemBlock(&data, sizeof(data));
    }
}

/**
 * @brief Records each current character as the saved character choice.
 */
void ControlUserDataHolder::onSave() {
    for (auto& rUser : mUsers) {
        rUser.mSavedCharacterType = rUser.mCharacterType;
    }
}
