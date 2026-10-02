#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
class LiveActor;
class PlayerHolder;

/**
 * Tracks the players for the cameras: which one is on top, their look-at positions, the ground
 * below them and whether they are falling or inside a camera restricted area.
 */
class PlayerWatcher {
public:
    PlayerWatcher(LiveActor** ppTopPlayer, LiveActor** ppRailPlayer, sead::Vector3f* pRailPos,
                  sead::Vector3f* pCameraRailDir, bool* pIsPlayerValid,
                  const sead::Vector3f** ppLookAtPosPtrs, bool* pIsPlayerApproach,
                  bool* pIsPlayerPrior, bool* pIsLookAtPlayerPos);

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
    bool getPlayerGroundPos(sead::Vector3f* pOut, s32 index) const;
    bool getTopPlayerGroundPos(sead::Vector3f* pOut) const;
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

    const PlayerHolder* getPlayerHolder() const { return mPlayerHolder; }

    bool isPlayerApproach(s32 index) const { return mIsPlayerApproach[index]; }

    bool isPlayerPrior(s32 index) const { return mIsPlayerPrior[index]; }

    /** Number of frames the look-at position is kept after a player becomes invalid. */
    static constexpr s32 cLookAtStopFrame = 120;

private:
    const PlayerHolder* mPlayerHolder;
    LiveActor** mTopPlayer;
    LiveActor** mRailPlayer;
    bool* mIsOnGround;
    sead::Vector3f* mGroundPos;
    sead::Vector3f* mLookAtPos;
    sead::Vector3f* mRailPos;
    sead::Vector3f* mCameraRailDir;
    const sead::Vector3f** mLookAtPosPtrs;
    const sead::Vector3f* mTopPlayerTargetPos;
    bool* mIsPlayerValid;
    bool* mIsPlayerApproach;
    bool* mIsPlayerPrior;
    bool* mIsLookAtPlayerPos;
    bool* mIsInCameraRestrictedArea;
    s32* mLookAtStopFrame;
    sead::Vector3f* mPrevLookAtPos;
    sead::Vector3f* mPrevPlayerPos;
    bool* mIsFallBothPlayerAndLookAt;
};

static_assert(sizeof(PlayerWatcher) == 0x98);

}  // namespace al
