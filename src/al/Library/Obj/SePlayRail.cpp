#include "Library/Obj/SePlayRail.hpp"

#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/Play/Placement/PlacementUtil.hpp"
#include "Library/Rail/RailUtil.hpp"
#include "Project/Audio/SeFunction.hpp"
#include "Project/Camera/Core/CameraUtil.hpp"

namespace al {
    /**
     * @brief Constructs a rail that plays a sound effect at the point nearest to the camera.
     * @param pName The actor name.
     */
    SePlayRail::SePlayRail(const char* pName) : LiveActor(pName) {}

    /**
     * @brief Initializes the rail, reads the sound effect name and starts playing it.
     * @param rInfo The actor init info.
     */
    void SePlayRail::init(const ActorInitInfo& rInfo) {
        initActorSceneInfo(this, rInfo);
        initActorPoseTRSV(this);
        initRailKeeper(rInfo);
        setSyncRailToStart(this);
        initExecutorWatchObj(this, rInfo);
        initActorAudioKeeper(this, rInfo, "SePlayRail", nullptr);
        tryGetStringArg(&mSeName, rInfo, "SePlayName");
        _15c = true;
        initActorClipping(this, rInfo);
        setRailClippingInfo(&mRailClippingInfo, this, 100.0f, 300.0f);
        startSe(this, mSeName, nullptr);
        makeActorAppeared();
    }

    /**
     * @brief Appears and restarts the sound effect.
     */
    void SePlayRail::appear() {
        LiveActor::appear();
        startSe(this, mSeName, nullptr);
    }

    /**
     * @brief Starts the sound effect.
     */
    void SePlayRail::startFirstStepSe() {
        startSe(this, mSeName, nullptr);
    }

    /**
     * @brief Moves the sound source to the rail point nearest to the camera look-at position.
     */
    void SePlayRail::control() {
        setSyncRailToNearestPos(this, getCameraLookAt(this));
    }
}  // namespace al
