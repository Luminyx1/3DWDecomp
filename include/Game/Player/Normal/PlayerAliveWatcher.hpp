#pragma once

namespace al {
class IUseSceneObjHolder;
class LiveActor;
}  // namespace al

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
};
