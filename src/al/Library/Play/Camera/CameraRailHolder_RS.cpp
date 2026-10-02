#include "Library/Play/Camera/CameraRailHolder_RS.hpp"

#include "Library/Actor/ActorInitInfo.hpp"
#include "Library/Camera/CameraLimitRailKeeper.hpp"
#include "Library/Camera/CameraUtil.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorMapUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementInfo.hpp"
#include "Library/StageSwitch/StageSwitchFunc.hpp"
#include "Project/Camera/Main/CameraDirector_RS.hpp"

namespace al {
/**
 * Creates an empty camera rail holder.
 * @param pName Actor name.
 */
CameraRailHolder_RS::CameraRailHolder_RS(const char* pName) : LiveActor(pName) {}

/**
 * Registers the holder with the camera director and creates one limit rail keeper per linked
 * "Rail" placement.
 * @param rInfo Actor init info.
 */
void CameraRailHolder_RS::init(const ActorInitInfo& rInfo) {
    initActorSceneInfo(this, rInfo);
    initExecutorWatchObj(this, rInfo);
    initStageSwitch(this, rInfo);
    rInfo.mActorSceneInfo.cameraDirector->registerCameraRailHolder(this);

    mRailCount = calcLinkChildNum(rInfo, "Rail");

    if (mRailCount > 0) {
        mRails = new CameraLimitRailKeeper*[mRailCount];

        for (s32 i = 0; i < mRailCount; i++) {
            PlacementInfo placementInfo;
            getLinksInfoByIndex(&placementInfo, rInfo, "Rail", i);
            mRails[i] = new CameraLimitRailKeeper();
            mRails[i]->init(placementInfo, getViewNumMax(this));
        }
    }

    tryListenStageSwitchKill(this);
    makeActorDead();
}

/**
 * Deactivates the holder and kills the actor.
 */
void CameraRailHolder_RS::kill() {
    mIsActive = false;
    LiveActor::kill();
}
}  // namespace al
