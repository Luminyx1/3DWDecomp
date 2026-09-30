#include "Library/Obj/SkyProjection.hpp"

#include <gfx/seadCamera.h>
#include <gfx/seadProjection.h>

#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/StageSwitch/StageSwitchFunc.hpp"
#include "Library/Thread/Functor.hpp"

namespace al {
/**
 * Constructs a sky projected with its own cameras.
 * @param pName actor name
 */
SkyProjection::SkyProjection(const char* pName) : Sky(pName) {
    mCamera0 = new sead::LookAtCamera(sead::Vector3f::zero, sead::Vector3f::ey, sead::Vector3f::ez);
    mCamera1 = new sead::LookAtCamera(sead::Vector3f::zero, sead::Vector3f::ey, sead::Vector3f::ez);
    mProjection0 = new sead::PerspectiveProjection(1.0f, 100000.0f, sead::Mathf::piHalf() / 2, 1.0f);
    mProjection1 = new sead::PerspectiveProjection(1.0f, 100000.0f, sead::Mathf::piHalf() / 2, 1.0f);
}

/**
 * Initializes the sky and its texture matrix parameters.
 * @param rInfo actor init info
 */
void SkyProjection::init(const ActorInitInfo& rInfo) {
    using SkyProjectionFunctor = FunctorV0M<SkyProjection*, void (SkyProjection::*)()>;

    Sky::init(rInfo);

    if (isSingleMode(rInfo)) {
        listenStageSwitchOnAppear(this, SkyProjectionFunctor(this, &SkyProjection::appear));
        listenStageSwitchOn(this, "SwitchDeadOn", SkyProjectionFunctor(this, &SkyProjection::kill));
    }

    tryGetArg(&mIsEnableTexMtxSet, rInfo, "IsEnableTexMtxSet");
    tryGetArg(&mIsEnableTexMtxSetUnder, rInfo, "IsEnableTexMtxSetUnder");
    tryGetArg(&mSkyTexMtxV, rInfo, "SkyTexMtxV");
    tryGetArg(&mSkyTexMtxVUnder, rInfo, "SkyTexMtxVUnder");
    tryGetArg(&mCloudTexMtxV, rInfo, "CloudTexMtxV");
    tryGetArg(&mCloudTexMtxVUnder, rInfo, "CloudTexMtxVUnder");
}

/**
 * Appears.
 */
void SkyProjection::appear() {
    LiveActor::appear();
}

/**
 * Kills the sky.
 */
void SkyProjection::kill() {
    LiveActor::kill();
}

/**
 * Follows the camera and updates the projection uniforms.
 */
void SkyProjection::control() {
    Sky::control();
    updateUniform();
}
}  // namespace al
