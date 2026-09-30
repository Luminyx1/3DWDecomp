#include "Library/Player/PlayerHolder.hpp"

namespace al {

PlayerHolder::PlayerHolder(s32 bufferSize) {
    mBufferSize = bufferSize;
    mPlayers = new Player[bufferSize];
    clear();
}

/**
 * Clears all registered players.
 */
void PlayerHolder::clear() {
    for (s32 i = 0; i < mBufferSize; i++) {
        mPlayers[i].mActor = nullptr;
        mPlayers[i].mPadRumbleKeeper = nullptr;
    }
}

void PlayerHolder::registerPlayer(LiveActor* pActor, PadRumbleKeeper* pPadRumbleKeeper,
                                  bool isPlayerActorClass) {
    s32 index = mPlayerNumComplete;
    if (mNotPlayerActorClassNum > 0 && isPlayerActorClass) {
        index = mPlayerNumComplete - mNotPlayerActorClassNum;
        for (s32 i = mPlayerNumComplete; i > index; i--) {
            mPlayers[i] = mPlayers[i - 1];
        }
    }
    mPlayers[index].mActor = pActor;
    mPlayers[index].mRegisteredActor = pActor;
    mPlayers[index].mPadRumbleKeeper = pPadRumbleKeeper;
    mPlayers[index].mIsPlayerActorClass = isPlayerActorClass;
    mPlayerNumComplete++;
    if (!isPlayerActorClass) {
        mNotPlayerActorClassNum++;
    }
}

/**
 * Gets a registered player.
 * @param index the index of the player
 * @return the player actor
 */
LiveActor* PlayerHolder::getPlayer(s32 index) const {
    return mPlayers[index].mActor;
}

/**
 * Gets a registered player if the index is valid.
 * @param index the index of the player
 * @return the player actor, or nullptr
 */
LiveActor* PlayerHolder::tryGetPlayer(s32 index) const {
    if (mBufferSize <= index) {
        return nullptr;
    }
    if (mPlayerNumComplete <= index) {
        return nullptr;
    }
    return mPlayers[index].mActor;
}

/**
 * Checks whether a registered player is a player actor class.
 * @param index the index of the player
 * @return true if the player is a player actor class
 */
bool PlayerHolder::isPlayerActorClass(s32 index) const {
    return mPlayers[index].mIsPlayerActorClass;
}

/**
 * Gets the number of registered player actor class players.
 * @return the number of players
 */
s32 PlayerHolder::getPlayerNum() const {
    return mPlayerNumComplete - mNotPlayerActorClassNum;
}

/**
 * Gets the number of all registered players.
 * @return the number of players
 */
s32 PlayerHolder::getPlayerNumComplete() const {
    return mPlayerNumComplete;
}

/**
 * Gets the size of the player buffer.
 * @return the buffer size
 */
s32 PlayerHolder::getBufferSize() const {
    return mBufferSize;
}

/**
 * Checks whether the player buffer is full.
 * @return true if no more players can be registered
 */
bool PlayerHolder::isFull() const {
    return mBufferSize <= mPlayerNumComplete;
}

/**
 * Checks whether a registered player has a pad rumble keeper.
 * @param index the index of the player
 * @return true if the pad rumble keeper exists
 */
bool PlayerHolder::isExistPadRumbleKeeper(s32 index) const {
    return mPlayers[index].mPadRumbleKeeper != nullptr;
}

/**
 * Gets the pad rumble keeper of a registered player.
 * @param index the index of the player
 * @return the pad rumble keeper
 */
PadRumbleKeeper* PlayerHolder::getPadRumbleKeeper(s32 index) const {
    return mPlayers[index].mPadRumbleKeeper;
}

/**
 * Replaces a registered player.
 * @param pActor the new player actor
 * @param pPadRumbleKeeper the new pad rumble keeper
 * @param index the index of the player
 */
void PlayerHolder::swapPlayer(LiveActor* pActor, PadRumbleKeeper* pPadRumbleKeeper, s32 index) {
    mPlayers[index].mActor = pActor;
    mPlayers[index].mPadRumbleKeeper = pPadRumbleKeeper;
}

/**
 * Checks whether an actor is the originally registered player.
 * @param pActor the actor
 * @param index the index of the player
 * @return true if the actor was registered at the index
 */
bool PlayerHolder::isPlayerActor(const LiveActor* pActor, s32 index) const {
    return mPlayers[index].mRegisteredActor == pActor;
}

}  // namespace al
