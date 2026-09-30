#include "Library/Movement/RailMoveMovement.hpp"

#include "Library/Actor/ActorInitInfo.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorActionUtil.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Rail/RailUtil.hpp"
#include "Library/Se/Function/SeFunction.hpp"

namespace {
using namespace al;

NERVE_DECL(RailMoveMovement, Move)
NERVE_DECL(RailMoveMovement, Standby)

NERVES_MAKE_NOSTRUCT(RailMoveMovement, Move, Standby)
}  // namespace

namespace al {

/**
 * Constructs a state that moves the host along its rail using placement parameters.
 * @param pHost Actor to move.
 * @param rInfo Placement information.
 */
RailMoveMovement::RailMoveMovement(LiveActor* pHost, const ActorInitInfo& rInfo)
    : HostStateBase("レール移動挙動", pHost) {
    tryGetArg(&mSpeed, *rInfo.mPlacementInfo, "Speed");
    tryGetArg(reinterpret_cast<s32*>(&mMoveType), *rInfo.mPlacementInfo, "MoveType");
    tryGetArg(&mWaitTime, *rInfo.mPlacementInfo, "WaitTime");

    if (static_cast<u32>(mMoveType) >= 4) {
        mMoveType = MoveType::Loop;
    }

    initNerve(&NrvRailMoveMovementMove, 0);
}

/**
 * Moves the host along the rail according to the move type.
 */
void RailMoveMovement::exeMove() {
    if (!isExistRail(getHost())) {
        return;
    }

    switch (mMoveType) {
    case MoveType::Loop:
        moveSyncRailLoop(getHost(), mSpeed);
        return;
    case MoveType::Turn:
        if (!moveSyncRailTurn(getHost(), mSpeed)) {
            return;
        }

        break;
    case MoveType::Stop:
        moveSyncRail(getHost(), mSpeed);
        return;
    case MoveType::Restart:
        if (!moveSyncRailPause(getHost(), mSpeed)) {
            return;
        }

        break;
    default:
        return;
    }

    if (mWaitTime > 0) {
        setNerve(this, &NrvRailMoveMovementStandby);
    }
}

/**
 * Waits at the end of the rail before moving again.
 */
void RailMoveMovement::exeStandby() {
    if (isFirstStep(this)) {
        if (mMoveType == MoveType::Restart) {
            startNerveAction(getHost(), "MoveSign");
        }

        tryStopSe(getHost(), "Move");
        tryStopSe(getHost(), "MoveLv");
        tryStartSe(getHost(), "MoveEnd");
    } else if (isGreaterEqualStep(this, mWaitTime)) {
        if (mMoveType == MoveType::Restart) {
            startNerveAction(getHost(), "MoveSign");
            tryStartSe(getHost(), "MoveSign");
        }

        setNerve(this, &NrvRailMoveMovementMove);
        tryStartSe(getHost(), "Move");
        tryStartSe(getHost(), "MoveLv");
    }
}

/**
 * Creates a rail movement if the host has a rail.
 * @param pHost Actor to move.
 * @param rInfo Placement information.
 * @return Created movement, or nullptr.
 */
RailMoveMovement* tryCreateRailMoveMovement(LiveActor* pHost, const ActorInitInfo& rInfo) {
    if (!isExistRail(pHost)) {
        return nullptr;
    }

    setSyncRailToNearestPos(pHost);
    return new RailMoveMovement(pHost, rInfo);
}

}  // namespace al
