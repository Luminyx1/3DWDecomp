#pragma once

#include <basis/seadTypes.h>

namespace al {
class LiveActor;
class PadRumbleKeeper;

class PlayerHolder {
public:
    PlayerHolder(s32 bufferSize);

    void clear();
    void registerPlayer(LiveActor* pActor, PadRumbleKeeper* pPadRumbleKeeper,
                        bool isPlayerActorClass);
    LiveActor* getPlayer(s32 index) const;
    LiveActor* tryGetPlayer(s32 index) const;
    bool isPlayerActorClass(s32 index) const;
    s32 getPlayerNum() const;
    s32 getPlayerNumComplete() const;
    s32 getBufferSize() const;
    bool isFull() const;
    bool isExistPadRumbleKeeper(s32 index) const;
    PadRumbleKeeper* getPadRumbleKeeper(s32 index) const;
    void swapPlayer(LiveActor* pActor, PadRumbleKeeper* pPadRumbleKeeper, s32 index);
    bool isPlayerActor(const LiveActor* pActor, s32 index) const;

private:
    struct Player {
        LiveActor* mActor = nullptr;
        PadRumbleKeeper* mPadRumbleKeeper = nullptr;
        LiveActor* mRegisteredActor = nullptr;
        bool mIsPlayerActorClass = true;
    };

    Player* mPlayers = nullptr;
    s32 mBufferSize = 0;
    s32 mPlayerNumComplete = 0;
    s32 mNotPlayerActorClassNum = 0;
};
}  // namespace al
