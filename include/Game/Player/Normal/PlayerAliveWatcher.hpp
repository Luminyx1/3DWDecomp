#pragma once

#include <basis/seadTypes.h>

namespace al {
class IUseSceneObjHolder;
class LiveActor;
}  // namespace al

class PlayerAliveWatcherGroup;
class PlayerAmiiboDirectorWatcher;

/// Scene object that revives dead players in bubbles during multiplayer.
class PlayerAliveWatcher {
public:
    static PlayerAliveWatcher* getPlayerAliveWatcher(const al::IUseSceneObjHolder* pHolder);
    static PlayerAliveWatcher* tryGetPlayerAliveWatcher(const al::IUseSceneObjHolder* pHolder);

    void deactivatePlayer(int index);
    bool isEnableIslandWarp(int index) const;
    bool isEnableExitStage(int index) const;
    void setDisableReviveBubble(al::LiveActor* pActor);
    void resetDisableReviveBubble(al::LiveActor* pActor);
    void setDisableBubbleFrameOut(al::LiveActor* pActor);
    void resetDisableBubbleFrameOut(al::LiveActor* pActor);
    void addBubbleDelayTime(int frames);
    s32 calcAllActivePlayerNum() const;
    s32 isActivePlayerPort(s32 port) const;
    bool isEnableBubbleWithInput(PlayerAliveWatcherGroup* pGroup) const;

    u8 _0[0x30];
    bool mIsEnableBubbleRevive;                                 // 0x30
    bool mIsEnableBubbleScreenOut;                              // 0x31
    bool mIsAbyss;                                              // 0x32, last player loss was a fall
    u8 _33[0x39 - 0x33];
    bool mIsNoLifeDecrease;                                     // 0x39
    bool mIsSingleMode;                                         // 0x3a, guess: no off-screen guide
    PlayerAmiiboDirectorWatcher* mAmiiboDirectorWatcher;        // 0x40
    s32 mBubbleDelayTime;                                       // 0x48
};
