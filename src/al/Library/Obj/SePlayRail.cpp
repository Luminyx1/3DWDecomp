#include "Library/Obj/SePlayRail.hpp"

#include "Library/Camera/CameraUtil.hpp"
#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Rail/RailUtil.hpp"
#include "Library/Se/Function/SeFunction.hpp"

namespace al {
/**
 * Constructs a rail playing a sound at the point nearest to the camera.
 * @param pName actor name
 */
SePlayRail::SePlayRail(const char* pName) : LiveActor(pName) {}

/**
 * Initializes the rail and starts its sound.
 * @param rInfo actor init info
 */
void SePlayRail::init(const ActorInitInfo& rInfo) {
    initActorSceneInfo(this, rInfo);
    initActorPoseTRSV(this);
    initRailKeeper(rInfo);
    setSyncRailToStart(this);
    initExecutorWatchObj(this, rInfo);
    initActorAudioKeeper(this, rInfo, "SePlayRail", nullptr);
    tryGetStringArg(&mSeName, rInfo, "SePlayName");
    mIsValidSe = true;
    initActorClipping(this, rInfo);
    setRailClippingInfo(&mRailClippingPos, this, 100.0f, 300.0f);
    startSe(this, mSeName);
    makeActorAppeared();
}

/**
 * Appears and restarts the sound.
 */
void SePlayRail::appear() {
    LiveActor::appear();
    startSe(this, mSeName);
}

/**
 * Starts the sound.
 */
void SePlayRail::startFirstStepSe() {
    startSe(this, mSeName);
}

/**
 * Moves to the rail point nearest to the camera.
 */
void SePlayRail::control() {
    setSyncRailToNearestPos(this, getCameraLookAt(this));
}
}  // namespace al
