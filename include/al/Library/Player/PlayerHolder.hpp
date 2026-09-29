#pragma once

#include <basis/seadTypes.h>

namespace al {
class LiveActor;
class PadRumbleKeeper;

class PlayerHolder {
public:
    PlayerHolder(s32);

    void clear();
    void registerPlayer(LiveActor*, PadRumbleKeeper*, bool);
    LiveActor* getPlayer(s32) const;
    LiveActor* tryGetPlayer(s32) const;
    bool isPlayerActorClass(s32) const;
    s32 getPlayerNum() const;
    s32 getPlayerNumComplete() const;
    s32 getBufferSize() const;
    bool isFull() const;
    bool isExistPadRumbleKeeper(s32) const;
    PadRumbleKeeper* getPadRumbleKeeper(s32) const;
    void swapPlayer(LiveActor*, PadRumbleKeeper*, s32);
    bool isPlayerActor(const LiveActor*, s32) const;

private:
    struct Player {
        LiveActor* mActor = nullptr;                   // _0
        PadRumbleKeeper* mPadRumbleKeeper = nullptr;   // _8
        LiveActor* mRegisteredActor = nullptr;         // _10
        bool mIsPlayerActorClass = true;               // _18
    };

    Player* mPlayers = nullptr;  // _0
    s32 mBufferSize = 0;         // _8
    s32 mPlayerNum = 0;          // _c
    s32 mNonActorClassNum = 0;   // _10
};
}  // namespace al
