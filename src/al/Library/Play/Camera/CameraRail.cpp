#include "Library/Play/Camera/CameraRail.hpp"

#include "Library/Camera/CameraUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Player/PlayerUtil.hpp"
#include "Library/Rail/Rail.hpp"
#include "Library/Rail/RailKeeper.hpp"
#include "Library/Rail/RailUtil.hpp"

namespace al {
/**
 * Creates the rail keeper for the camera rail from the placement.
 * @param rInfo Placement info of the rail.
 */
CameraRail::CameraRail(const PlacementInfo& rInfo) {
    mRailKeeper = new RailKeeper(rInfo);
    mRailKeeper->getRail()->isIncludeBezierRailPart();
}

/**
 * Finds the camera target player that is furthest along the rail.
 * @param pActor Actor used to access the players.
 * @param pTopPlayerIndex Receives the index of the furthest player.
 * @return Distance between that player and its nearest rail position.
 */
f32 CameraRail::calcTopPlayer(LiveActor* pActor, s32* pTopPlayerIndex) {
    f32 maxCoord = 0.0f;

    for (s32 i = 0; i < getPlayerNumMax(pActor); i++) {
        if (!isCameraCalcTarget(pActor, i)) {
            continue;
        }

        sead::Vector3f trans = getTrans(getPlayerActor(pActor, i));
        f32 coord = calcNearestRailCoord(mRailKeeper, trans);

        if (maxCoord <= coord) {
            maxCoord = coord;
            *pTopPlayerIndex = i;
        }
    }

    sead::Vector3f railPos;
    sead::Vector3f trans = getTrans(getPlayerActor(pActor, *pTopPlayerIndex));
    calcNearestRailPos(&railPos, mRailKeeper, trans);
    return (railPos - trans).length();
}

/**
 * Calculates the rail position nearest to a player.
 * @param pActor Actor used to access the players.
 * @param playerIndex Player index.
 * @return Nearest rail position.
 */
sead::Vector3f CameraRail::calcPlayerRailPos(LiveActor* pActor, s32 playerIndex) {
    sead::Vector3f trans = getTrans(getPlayerActor(pActor, playerIndex));
    sead::Vector3f railPos;
    calcNearestRailPos(&railPos, mRailKeeper, trans);
    return railPos;
}

/**
 * Stores the nearest rail position of every player.
 * @param pActor Actor used to access the players.
 */
void CameraRail::setPlayerRailPos(LiveActor* pActor) {
    for (s32 i = 0; i < getPlayerNumMax(pActor); i++) {
        sead::Vector3f railPos = calcPlayerRailPos(pActor, i);
        al::setPlayerRailPos(pActor, i, railPos);
    }
}

/**
 * Stores the rail direction at the position nearest to the actor.
 * @param pActor Actor used to access the players.
 */
void CameraRail::setPlayerRailDir(LiveActor* pActor) {
    f32 coord = calcNearestRailCoord(mRailKeeper, getTrans(pActor));
    sead::Vector3f dir;
    calcRailDirAtCoord(&dir, mRailKeeper, coord);
    al::setPlayerRailDir(pActor, dir);
}
}  // namespace al
