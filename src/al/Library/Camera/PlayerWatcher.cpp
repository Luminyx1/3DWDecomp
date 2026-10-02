#include "Library/Camera/PlayerWatcher.hpp"

#include <nerd/nerdMath.h>

#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorMovementUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementInfo.hpp"
#include "Library/Player/PlayerHolder.hpp"
#include "Library/Player/PlayerUtil.hpp"
#include "Project/AreaObj/AreaObj.hpp"
#include "Project/AreaObj/AreaObjUtil.hpp"
#include "Project/Collision/CollisionUtil.hpp"

namespace {

/**
 * Gets the area object user of a player actor.
 * @param pPlayerHolder The player holder.
 * @param index Index of the player.
 * @return The area object user of the player, or nullptr if there is no player actor.
 */
inline const al::IUseAreaObj* getPlayerAreaObjUser(const al::PlayerHolder* pPlayerHolder,
                                                   s32 index) {
    return al::getPlayerActor(pPlayerHolder, index);
}

}  // namespace

namespace al {

/**
 * Creates the player watcher with pointers to the state shared with the camera director.
 * @param ppTopPlayer Where the current top player actor is stored.
 * @param ppRailPlayer Where the player following the camera rail is stored.
 * @param pRailPos Per-player positions on the camera rail.
 * @param pCameraRailDir Direction of the camera rail.
 * @param pIsPlayerValid Per-player flags whether the player is targeted by the camera.
 * @param ppLookAtPosPtrs Per-player look-at position overrides.
 * @param pIsPlayerApproach Per-player flags whether the player approaches the camera.
 * @param pIsPlayerPrior Per-player flags whether the player has priority.
 * @param pIsLookAtPlayerPos Per-player flags whether to look directly at the player.
 */
PlayerWatcher::PlayerWatcher(LiveActor** ppTopPlayer, LiveActor** ppRailPlayer,
                             sead::Vector3f* pRailPos, sead::Vector3f* pCameraRailDir,
                             bool* pIsPlayerValid, const sead::Vector3f** ppLookAtPosPtrs,
                             bool* pIsPlayerApproach, bool* pIsPlayerPrior,
                             bool* pIsLookAtPlayerPos)
    : mPlayerHolder(nullptr), mTopPlayer(ppTopPlayer), mRailPlayer(ppRailPlayer),
      mIsOnGround(nullptr), mGroundPos(nullptr), mLookAtPos(nullptr), mRailPos(pRailPos),
      mCameraRailDir(pCameraRailDir), mLookAtPosPtrs(ppLookAtPosPtrs),
      mTopPlayerTargetPos(nullptr), mIsPlayerValid(pIsPlayerValid),
      mIsPlayerApproach(pIsPlayerApproach), mIsPlayerPrior(pIsPlayerPrior),
      mIsLookAtPlayerPos(pIsLookAtPlayerPos), mIsInCameraRestrictedArea(nullptr),
      mLookAtStopFrame(nullptr), mPrevLookAtPos(nullptr), mPrevPlayerPos(nullptr),
      mIsFallBothPlayerAndLookAt(nullptr) {}

/**
 * Allocates the per-player buffers and resets them.
 * @param pPlayerHolder The player holder to watch.
 */
void PlayerWatcher::init(const PlayerHolder* pPlayerHolder) {
    mPlayerHolder = pPlayerHolder;
    mIsOnGround = new bool[mPlayerHolder->getPlayerNum()];
    mLookAtStopFrame = new s32[mPlayerHolder->getPlayerNum()];
    mPrevLookAtPos = new sead::Vector3f[mPlayerHolder->getPlayerNum()];
    mPrevPlayerPos = new sead::Vector3f[mPlayerHolder->getPlayerNum()];
    mGroundPos = new sead::Vector3f[mPlayerHolder->getPlayerNum()];
    mLookAtPos = new sead::Vector3f[mPlayerHolder->getPlayerNum()];
    mIsInCameraRestrictedArea = new bool[mPlayerHolder->getPlayerNum()];
    mIsFallBothPlayerAndLookAt = new bool[mPlayerHolder->getPlayerNum()];

    for (s32 i = 0; i < mPlayerHolder->getPlayerNum(); i++) {
        mIsOnGround[i] = true;
        mLookAtStopFrame[i] = cLookAtStopFrame;
        mPrevLookAtPos[i] = sead::Vector3f::zero;
        mPrevPlayerPos[i] = sead::Vector3f::zero;
        mGroundPos[i] = sead::Vector3f::zero;
        mIsInCameraRestrictedArea[i] = false;
        mIsFallBothPlayerAndLookAt[i] = false;
    }
}

/**
 * Gets the number of registered players.
 * @return The number of players.
 */
s32 PlayerWatcher::getPlayerNum() const {
    return mPlayerHolder->getPlayerNum();
}

/**
 * Updates the top player, the ground below each player and the look-at positions.
 */
void PlayerWatcher::update() {
    if (getAlivePlayerNum() == 0) {
        return;
    }

    for (s32 i = 0; i < getPlayerNum(); i++) {
        if (isPlayerAlive(i)) {
            mLookAtStopFrame[i] = 0;
        } else {
            mLookAtStopFrame[i]++;
        }
    }

    for (s32 i = 0; i < getPlayerNum(); i++) {
        if (!isPlayerAlive(i)) {
            continue;
        }

        if (isInAreaObj(getPlayerAreaObjUser(mPlayerHolder, i), "CameraRestrictedArea",
                        al::getPlayerPos(mPlayerHolder, i))) {
            mIsInCameraRestrictedArea[i] = true;
        } else {
            mIsInCameraRestrictedArea[i] = false;
        }
    }

    if (*mRailPlayer != nullptr) {
        *mTopPlayer = *mRailPlayer;
    } else if (mTopPlayerTargetPos != nullptr) {
        s32 nearestIndex = 0;
        f32 nearestDistance = 0.0f;
        bool isFirst = true;

        for (s32 i = 0; i < getPlayerNum(); i++) {
            if (!isPlayerAlive(i)) {
                continue;
            }

            sead::Vector3f diff = al::getPlayerPos(mPlayerHolder, i) - *mTopPlayerTargetPos;
            f32 distance = nerd::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);

            if (isFirst || nearestDistance > distance) {
                nearestDistance = distance;
                nearestIndex = i;
                isFirst = false;
            }
        }

        *mTopPlayer = getPlayerActor(mPlayerHolder, nearestIndex);
    } else {
        updateTopPlayer();
    }

    sead::Vector3f groundDir(0.0f, -700.0f, 0.0f);

    for (s32 i = 0; i < getPlayerNum(); i++) {
        if (isPlayerAlive(i)) {
            sead::Vector3f start = getPlayerPos(i);
            sead::Vector3f hitPos;
            start.y += 100.0f;

            if (alCollisionUtil::getFirstPolyOnArrow(*mTopPlayer, &hitPos, nullptr, start,
                                                     groundDir, nullptr, nullptr)) {
                mIsOnGround[i] = true;
                mGroundPos[i] = hitPos;
                continue;
            }
        }

        mIsOnGround[i] = false;
    }

    for (s32 i = 0; i < getPlayerNum(); i++) {
        if (mLookAtPos[i].y + 5.0f < mPrevLookAtPos[i].y &&
            al::getPlayerPos(mPlayerHolder, i).y < mPrevPlayerPos[i].y) {
            mIsFallBothPlayerAndLookAt[i] = true;
        }

        if (mIsPlayerApproach[i]) {
            mIsFallBothPlayerAndLookAt[i] = false;
        }
    }

    for (s32 i = 0; i < getPlayerNum(); i++) {
        if (!mIsInCameraRestrictedArea[i]) {
            updatePlayerLookAtPos(i);
        }
    }

    for (s32 i = 0; i < getPlayerNum(); i++) {
        if (isPlayerAlive(i) && !mIsInCameraRestrictedArea[i]) {
            sead::Vector3f lookAtPos = mLookAtPos[i];
            mPrevLookAtPos[i] = lookAtPos;
            mPrevPlayerPos[i] = al::getPlayerPos(mPlayerHolder, i);
        }
    }
}

/**
 * Counts the players that are alive and targeted by the camera.
 * @return The number of alive players.
 */
s32 PlayerWatcher::getAlivePlayerNum() const {
    s32 num = 0;

    for (s32 i = 0; i < getPlayerNum(); i++) {
        if (!isPlayerDead(mPlayerHolder, i)) {
            num += mIsPlayerValid[i];
        }
    }

    return num;
}

/**
 * Checks whether a player is alive and targeted by the camera.
 * @param index Index of the player.
 * @return Whether the player is alive.
 */
bool PlayerWatcher::isPlayerAlive(s32 index) const {
    if (isPlayerDead(mPlayerHolder, index)) {
        return false;
    }

    return mIsPlayerValid[index];
}

/**
 * Gets the position the camera follows for a player, which can be overridden by a look-at
 * position or the target of a camera stop look-at area.
 * @param index Index of the player.
 * @return The position of the player.
 */
const sead::Vector3f& PlayerWatcher::getPlayerPos(s32 index) const {
    if (mIsInCameraRestrictedArea[index]) {
        return mPrevPlayerPos[index];
    }

    if (mLookAtPosPtrs[index] != nullptr) {
        return *mLookAtPosPtrs[index];
    }

    AreaObj* areaObj = tryFindAreaObj(getPlayerAreaObjUser(mPlayerHolder, index),
                                      "CameraStopLookAtArea", al::getPlayerPos(mPlayerHolder, index));

    if (areaObj != nullptr) {
        bool isValidSingleMode = false;
        tryGetAreaObjArg(&isValidSingleMode, areaObj, "IsValidSingleMode");

        if (isValidSingleMode) {
            sead::Vector3f* lookAtPos = mLookAtPos;
            const PlacementInfo& areaInfo = areaObj->getPlacementInfo();
            PlacementInfo targetInfo;

            if (tryGetLinksInfo(&targetInfo, areaInfo, "TargetPos")) {
                tryGetTrans(&lookAtPos[index], targetInfo);
                return mLookAtPos[index];
            }
        }
    }

    return al::getPlayerPos(mPlayerHolder, index);
}

/**
 * Gets the look-at position of a player.
 * @param pOut Where to store the look-at position.
 * @param index Index of the player.
 */
void PlayerWatcher::getPlayerLookAtPos(sead::Vector3f* pOut, s32 index) const {
    *pOut = mLookAtPos[index];
}

/**
 * Updates the look-at position of a player.
 * @param index Index of the player.
 */
void PlayerWatcher::updatePlayerLookAtPos(s32 index) {
    if (mLookAtPosPtrs[index] != nullptr) {
        mLookAtPos[index] = *mLookAtPosPtrs[index];
        return;
    }

    AreaObj* areaObj = tryFindAreaObj(getPlayerAreaObjUser(mPlayerHolder, index),
                                      "CameraStopLookAtArea", al::getPlayerPos(mPlayerHolder, index));

    if (areaObj != nullptr) {
        sead::Vector3f* lookAtPos = mLookAtPos;
        const PlacementInfo& areaInfo = areaObj->getPlacementInfo();
        PlacementInfo targetInfo;

        if (tryGetLinksInfo(&targetInfo, areaInfo, "TargetPos")) {
            tryGetTrans(&lookAtPos[index], targetInfo);
            return;
        }
    }

    if (isInWaterArea(getPlayerAreaObjUser(mPlayerHolder, index), getPlayerPos(index)) ||
        mIsLookAtPlayerPos[index]) {
        mLookAtPos[index] = getPlayerPos(index);
        return;
    }

    const sead::Vector3f offset(0.0f, 150.0f, 0.0f);

    if (mIsPlayerApproach[index]) {
        mLookAtPos[index] = getPlayerPos(index) + offset;
        return;
    }

    sead::Vector3f groundPos;

    if (getPlayerGroundPos(&groundPos, index)) {
        mLookAtPos[index] = groundPos + offset;
        return;
    }

    if (*mRailPlayer != nullptr && mRailPos[index].y > getPlayerPos(index).y) {
        sead::Vector3f lookAtPos = getPlayerPos(index);
        lookAtPos.y = mRailPos[index].y + 150.0f;
        mLookAtPos[index] = lookAtPos;
        return;
    }

    mLookAtPos[index] = getPlayerPos(index) + offset;
}

/**
 * Makes the first alive player the top player.
 */
void PlayerWatcher::updateTopPlayer() {
    for (s32 i = 0; i < getPlayerNum(); i++) {
        if (isPlayerAlive(i)) {
            *mTopPlayer = getPlayerActor(mPlayerHolder, i);
            return;
        }
    }
}

/**
 * Gets the maximum number of players.
 * @return The maximum number of players.
 */
s32 PlayerWatcher::getPlayerNumMax() const {
    return al::getPlayerNumMax(mPlayerHolder);
}

/**
 * Gets the index of the top player.
 * @return The index of the top player in the player holder.
 */
s32 PlayerWatcher::getTopPlayerIndex() const {
    return alPlayerFunction::findPlayerHolderIndex(*mTopPlayer);
}

/**
 * Gets the position of the top player.
 * @return The position of the top player.
 */
const sead::Vector3f& PlayerWatcher::getTopPlayerPos() const {
    return getPlayerPos(getTopPlayerIndex());
}

/**
 * Gets the position of the first alive player actor.
 * @return The position of the first alive player actor.
 */
const sead::Vector3f& PlayerWatcher::getAlivePlayerActorFirstPos() const {
    return getTrans(findAlivePlayerActorFirst(mPlayerHolder));
}

/**
 * Gets the position of the top player on the camera rail if a player follows the rail.
 * @return The rail position of the top player, or its position without a rail player.
 */
const sead::Vector3f& PlayerWatcher::getTopPlayerRailPos() const {
    if (*mRailPlayer != nullptr) {
        return mRailPos[getTopPlayerIndex()];
    }

    return getPlayerPos(getTopPlayerIndex());
}

/**
 * Gets the ground position below a player.
 * @param pOut Where to store the ground position.
 * @param index Index of the player.
 * @return Whether ground was found below the player.
 */
bool PlayerWatcher::getPlayerGroundPos(sead::Vector3f* pOut, s32 index) const {
    *pOut = mGroundPos[index];
    return mIsOnGround[index];
}

/**
 * Gets the ground position below the top player.
 * @param pOut Where to store the ground position.
 * @return Whether ground was found below the top player.
 */
bool PlayerWatcher::getTopPlayerGroundPos(sead::Vector3f* pOut) const {
    return getPlayerGroundPos(pOut, getTopPlayerIndex());
}

/**
 * Tries to get the look-at position of a player, keeping the last one for a while after the
 * player stops being targeted.
 * @param pOut Where to store the look-at position.
 * @param index Index of the player.
 * @return Whether a look-at position was stored.
 */
bool PlayerWatcher::tryGetPlayerLookAtPos(sead::Vector3f* pOut, s32 index) const {
    bool isAlive = isPlayerAlive(index);

    if (isAlive && !mIsInCameraRestrictedArea[index]) {
        *pOut = mLookAtPos[index];
        return true;
    }

    if (isAlive || mLookAtStopFrame[index] < cLookAtStopFrame) {
        *pOut = mPrevLookAtPos[index];
        return true;
    }

    *pOut = sead::Vector3f::zero;
    return false;
}

/**
 * Gets the look-at position of the top player.
 * @param pOut Where to store the look-at position.
 */
void PlayerWatcher::getTopPlayerLookAtPos(sead::Vector3f* pOut) const {
    getPlayerLookAtPos(pOut, getTopPlayerIndex());
}

/**
 * Gets the velocity of a player.
 * @param index Index of the player.
 * @return The velocity of the player.
 */
const sead::Vector3f& PlayerWatcher::getPlayerVelocity(s32 index) const {
    return getVelocity(mPlayerHolder->getPlayer(index));
}

/**
 * Gets the velocity of the top player.
 * @return The velocity of the top player.
 */
const sead::Vector3f& PlayerWatcher::getTopPlayerVelocity() const {
    return getPlayerVelocity(getTopPlayerIndex());
}

/**
 * Counts the players the camera still targets, including those whose look-at position is kept.
 * @return The number of camera targets.
 */
s32 PlayerWatcher::getCameraTargetNum() const {
    s32 num = 0;

    for (s32 i = 0; i < getPlayerNum(); i++) {
        if (isPlayerAlive(i) || mLookAtStopFrame[i] < cLookAtStopFrame) {
            num++;
        }
    }

    return num;
}

/**
 * Gets the direction of the camera rail.
 * @return The direction of the camera rail.
 */
const sead::Vector3f& PlayerWatcher::getCameraRailDir() const {
    return *mCameraRailDir;
}

/**
 * Checks whether the look-at position of a player is overridden.
 * @param index Index of the player.
 * @return Whether a look-at position override is set.
 */
bool PlayerWatcher::isSetLookAtPosPtr(s32 index) const {
    return mLookAtPosPtrs[index] != nullptr;
}

/**
 * Stops keeping the look-at positions of all players.
 */
void PlayerWatcher::offLookAtStop() {
    for (s32 i = 0; i < getPlayerNum(); i++) {
        mLookAtStopFrame[i] = cLookAtStopFrame;
    }
}

/**
 * Checks whether a player is in a camera restricted area.
 * @param index Index of the player.
 * @return Whether the player is in a camera restricted area.
 */
bool PlayerWatcher::isInCameraRestrictedArea(s32 index) const {
    return mIsInCameraRestrictedArea[index];
}

/**
 * Checks whether both a player and its look-at position are falling.
 * @param index Index of the player.
 * @return Whether the player and its look-at position fall.
 */
bool PlayerWatcher::isFallBothPlayerAndLookAt(s32 index) const {
    return mIsFallBothPlayerAndLookAt[index];
}

/**
 * Checks whether any player has priority.
 * @return Whether a prior player exists.
 */
bool PlayerWatcher::isExistPriorPlayer() const {
    for (s32 i = 0; i < getPlayerNum(); i++) {
        if (mIsPlayerPrior[i]) {
            return true;
        }
    }

    return false;
}

/**
 * Gets the position of a player on the camera rail.
 * @param index Index of the player.
 * @return The rail position of the player.
 */
const sead::Vector3f& PlayerWatcher::getPlayerRailPos(s32 index) const {
    return mRailPos[index];
}

/**
 * Checks whether an additional camera look-at position is set after the players' ones.
 * @return Whether the additional look-at position is set.
 */
bool PlayerWatcher::isExistAdditionalCameraLookAtPos() const {
    return mLookAtPosPtrs[getPlayerNum() + 1] != nullptr;
}

/**
 * Gets the additional camera look-at position stored after the players' ones.
 * @return The additional look-at position.
 */
const sead::Vector3f& PlayerWatcher::getAdditionalCameraLookAtPos() const {
    return *mLookAtPosPtrs[getPlayerNum() + 1];
}

/**
 * Stops keeping the look-at position of a player.
 * @param index Index of the player.
 */
void PlayerWatcher::quitPlayerLookAtStop(s32 index) {
    mLookAtStopFrame[index] = cLookAtStopFrame;
}

}  // namespace al
