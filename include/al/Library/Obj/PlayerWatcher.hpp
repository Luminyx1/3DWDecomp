#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
class LiveActor;
class PlayerHolder;
class PlayerWatcher {
public:
    PlayerWatcher(LiveActor** pPlayers, LiveActor** pLookAtActors, sead::Vector3f* pPlayerPos,
                  sead::Vector3f* pLookAtPos, bool*, const sead::Vector3f**, bool*, bool*, bool*);

    void init(const PlayerHolder* pPlayerHolder);
    s32 getPlayerNum() const;
    void update();
    s32 getAlivePlayerNum() const;
    bool isPlayerAlive(s32 index) const;
    const sead::Vector3f& getPlayerPos(s32 index) const;
    void getPlayerLookAtPos(sead::Vector3f* pOut, s32 index) const;
    void updatePlayerLookAtPos(s32 index);
    void updateTopPlayer();
    s32 getPlayerNumMax() const;
    s32 getTopPlayerIndex() const;
    const sead::Vector3f& getTopPlayerPos() const;
    const sead::Vector3f& getAlivePlayerActorFirstPos() const;
    const sead::Vector3f& getTopPlayerRailPos() const;
    void getPlayerGroundPos(sead::Vector3f* pOut, s32 index) const;
    void getTopPlayerGroundPos(sead::Vector3f* pOut) const;
    bool tryGetPlayerLookAtPos(sead::Vector3f* pOut, s32 index) const;
    void getTopPlayerLookAtPos(sead::Vector3f* pOut) const;
    const sead::Vector3f& getPlayerVelocity(s32 index) const;
    const sead::Vector3f& getTopPlayerVelocity() const;
    s32 getCameraTargetNum() const;
    const sead::Vector3f& getCameraRailDir() const;
    bool isSetLookAtPosPtr(s32 index) const;
    void offLookAtStop();
    bool isInCameraRestrictedArea(s32 index) const;
    bool isFallBothPlayerAndLookAt(s32 index) const;
    bool isExistPriorPlayer() const;
    const sead::Vector3f& getPlayerRailPos(s32 index) const;
    bool isExistAdditionalCameraLookAtPos() const;
    const sead::Vector3f& getAdditionalCameraLookAtPos() const;
    void quitPlayerLookAtStop(s32 index);
};
}  // namespace al
