#include "Library/Player/PlayerHolder.hpp"

namespace al {
    /**
     * @brief Constructs a holder with room for a fixed number of players.
     * @param maxPlayers The maximum number of players.
     */
    PlayerHolder::PlayerHolder(s32 maxPlayers) {
        mBufferSize = maxPlayers;
        mPlayers = new Player[maxPlayers];
        clear();
    }

    /**
     * @brief Clears the actor and pad rumble keeper of every player slot.
     */
    void PlayerHolder::clear() {
        for (s32 i = 0; i < mBufferSize; i++) {
            mPlayers[i].mActor = nullptr;
            mPlayers[i].mPadRumbleKeeper = nullptr;
        }
    }

    /**
     * @brief Registers a player; players of the actor class are kept in front of the others.
     * @param pActor The player actor.
     * @param pRumbleKeeper The pad rumble keeper of the player.
     * @param isPlayerActorClass Whether the actor is a real player actor.
     */
    void PlayerHolder::registerPlayer(LiveActor* pActor, PadRumbleKeeper* pRumbleKeeper, bool isPlayerActorClass) {
        s32 index = mPlayerNum;
        if (mNonActorClassNum > 0 && isPlayerActorClass) {
            index = mPlayerNum - mNonActorClassNum;
            for (s64 i = mPlayerNum; i > index; i--) {
                mPlayers[i] = mPlayers[i - 1];
            }
        }

        mPlayers[index].mActor = pActor;
        mPlayers[index].mRegisteredActor = pActor;
        mPlayers[index].mPadRumbleKeeper = pRumbleKeeper;
        mPlayers[index].mIsPlayerActorClass = isPlayerActorClass;
        mPlayerNum++;

        if (!isPlayerActorClass) {
            mNonActorClassNum++;
        }
    }

    /**
     * @brief Gets a player actor.
     * @param index The player index.
     * @return The player actor.
     */
    LiveActor* PlayerHolder::getPlayer(s32 index) const {
        return mPlayers[index].mActor;
    }

    /**
     * @brief Gets a player actor if the index is registered.
     * @param index The player index.
     * @return The player actor, or nullptr if the index is out of range.
     */
    LiveActor* PlayerHolder::tryGetPlayer(s32 index) const {
        if (mBufferSize <= index) {
            return nullptr;
        }
        if (mPlayerNum <= index) {
            return nullptr;
        }
        return mPlayers[index].mActor;
    }

    /**
     * @brief Checks whether a player is a real player actor.
     * @param index The player index.
     * @return Whether the player is of the player actor class.
     */
    bool PlayerHolder::isPlayerActorClass(s32 index) const {
        return mPlayers[index].mIsPlayerActorClass;
    }

    /**
     * @brief Gets the number of real player actors.
     * @return The number of registered players of the player actor class.
     */
    s32 PlayerHolder::getPlayerNum() const {
        return mPlayerNum - mNonActorClassNum;
    }

    /**
     * @brief Gets the number of registered players, including non player actor classes.
     * @return The total number of registered players.
     */
    s32 PlayerHolder::getPlayerNumComplete() const {
        return mPlayerNum;
    }

    /**
     * @brief Gets the maximum number of players.
     * @return The buffer size.
     */
    s32 PlayerHolder::getBufferSize() const {
        return mBufferSize;
    }

    /**
     * @brief Checks whether no more players can be registered.
     * @return Whether the holder is full.
     */
    bool PlayerHolder::isFull() const {
        return mBufferSize <= mPlayerNum;
    }

    /**
     * @brief Checks whether a player has a pad rumble keeper.
     * @param index The player index.
     * @return Whether the player has a pad rumble keeper.
     */
    bool PlayerHolder::isExistPadRumbleKeeper(s32 index) const {
        return mPlayers[index].mPadRumbleKeeper != nullptr;
    }

    /**
     * @brief Gets a player's pad rumble keeper.
     * @param index The player index.
     * @return The pad rumble keeper.
     */
    PadRumbleKeeper* PlayerHolder::getPadRumbleKeeper(s32 index) const {
        return mPlayers[index].mPadRumbleKeeper;
    }

    /**
     * @brief Replaces the actor and pad rumble keeper of a player slot.
     * @param pActor The new player actor.
     * @param pRumbleKeeper The new pad rumble keeper.
     * @param index The player index.
     */
    void PlayerHolder::swapPlayer(LiveActor* pActor, PadRumbleKeeper* pRumbleKeeper, s32 index) {
        mPlayers[index].mActor = pActor;
        mPlayers[index].mPadRumbleKeeper = pRumbleKeeper;
    }

    /**
     * @brief Checks whether an actor is the one originally registered for a player slot.
     * @param pActor The actor to check.
     * @param index The player index.
     * @return Whether the actor was registered at that index.
     */
    bool PlayerHolder::isPlayerActor(const LiveActor* pActor, s32 index) const {
        return mPlayers[index].mRegisteredActor == pActor;
    }
}  // namespace al
