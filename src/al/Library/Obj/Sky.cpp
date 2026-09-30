#include "Library/Obj/Sky.hpp"

#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorMapUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/StageSwitch/StageSwitchFunc.hpp"
#include "Project/Camera/Core/CameraUtil.hpp"

namespace al {
/**
 * Constructs a sky.
 * @param pName actor name
 */
Sky::Sky(const char* pName) : LiveActor(pName) {}

/**
 * Initializes the sky model.
 * @param rInfo actor init info
 */
void Sky::init(const ActorInitInfo& rInfo) {
    bool isUseDemo = false;
    tryGetArg(&isUseDemo, rInfo, "IsUseDemo");
    initActorChangeModelSuffix(this, rInfo, isUseDemo ? "Demo" : nullptr);
    invalidateClipping(this);
    bool isOnlyCubeMap = false;
    tryGetArg(&isOnlyCubeMap, rInfo, "IsOnlyCubeMap");
    if (isOnlyCubeMap) {
        makeActorDead();
        return;
    }
    trySyncStageSwitchAppear(this);
}

/**
 * Follows the camera.
 */
void Sky::control() {
    setTrans(this, getCameraPos(this));
}
}  // namespace al
