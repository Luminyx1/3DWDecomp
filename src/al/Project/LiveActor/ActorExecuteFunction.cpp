#include "Project/LiveActor/ActorExecuteFunction.hpp"

#include "Library/Camera/CameraDirector.hpp"
#include "Library/Camera/CameraUtil.hpp"
#include "Library/Controller/PadRumbleDirector.hpp"
#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Execute/ExecuteDirector.hpp"
#include "Library/LiveActor/Common/LiveActorGroup.hpp"
#include "Library/LiveActor/Common/LiveActorKit.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/Model/ModelKeeper.hpp"
#include "Project/Camera/Main/CameraDirector_RS.hpp"
#include "Library/Shadow/ShadowDirector.hpp"

namespace al {
/**
 * Updates the actor kit.
 * @param pKit The actor kit.
 */
void executeUpdate(LiveActorKit* pKit) {
    pKit->update();
}

/**
 * Forces the highest level of detail on all actors.
 * @param pKit The actor kit.
 */
void setLODForceLevel0(LiveActorKit* pKit) {
    LiveActorGroup* group = pKit->getActorGroup();

    for (s32 i = 0; i < group->mNumActors; i++) {
        setLODForceLevel0(group->mActors[i]);
    }
}

/**
 * Updates the level of detail of all actor models from the camera position.
 * @param pKit The actor kit.
 * @param isForceLevel0 Whether to force the highest level of detail first.
 */
void forceUpdateLOD(LiveActorKit* pKit, bool isForceLevel0) {
    LiveActorGroup* group = pKit->getActorGroup();
    const SceneCameraInfo* cameraInfo = (pKit->getCameraDirector_RS() != nullptr) ?
                                            pKit->getCameraDirector_RS()->getSceneCameraInfo() :
                                            pKit->getCameraDirector()->mSceneCameraInfo;
    const sead::Vector3f& cameraPos = getCameraPos(cameraInfo);

    for (s32 i = 0; i < group->mNumActors; i++) {
        LiveActor* actor = group->mActors[i];

        if (isForceLevel0) {
            setLODForceLevel0(actor);
        }

        if (actor->getModelKeeper() != nullptr) {
            actor->getModelKeeper()->updateLod(cameraPos, false);
        }
    }
}

/**
 * Executes one update list.
 * @param pKit The actor kit.
 * @param pListName The list name.
 */
void executeUpdateList(LiveActorKit* pKit, const char* pListName) {
    pKit->getExecuteDirector()->executeList(pListName);
}

/**
 * Executes one update list while paused.
 * @param pKit The actor kit.
 * @param pListName The list name.
 */
void executeUpdateListPaused(LiveActorKit* pKit, const char* pListName) {
    pKit->getExecuteDirector()->executeListPaused(pListName);
}

/**
 * Executes one update list while stalled.
 * @param pKit The actor kit.
 * @param pListName The list name.
 */
void executeUpdateListStall(LiveActorKit* pKit, const char* pListName) {
    pKit->getExecuteDirector()->executeListStall(pListName);
}

/**
 * Draws an execute table.
 * @param pKit The actor kit.
 * @param pTableName The execute table name.
 */
void executeDraw(const LiveActorKit* pKit, const char* pTableName) {
    pKit->getExecuteDirector()->draw(pTableName);
}

/**
 * Draws one list of an execute table.
 * @param pKit The actor kit.
 * @param pTableName The execute table name.
 * @param pListName The draw list name.
 */
void executeDrawList(const LiveActorKit* pKit, const char* pTableName, const char* pListName) {
    pKit->getExecuteDirector()->drawList(pTableName, pListName);
}

/**
 * Checks whether an execute table has anything to draw.
 * @param pKit The actor kit.
 * @param pTableName The execute table name.
 * @return Whether the table is active.
 */
bool isActiveDraw(const LiveActorKit* pKit, const char* pTableName) {
    return pKit->getExecuteDirector()->isActiveDraw(pTableName);
}

/**
 * Gets the depth shadow drawer.
 * @param pKit The actor kit.
 * @return The depth shadow drawer.
 */
DepthShadowDrawer* getDepthShadowDrawer(LiveActorKit* pKit) {
    return pKit->getGraphicsSystemInfo()->getShadowDirector()->mDepthShadowDrawer;
}

/**
 * Updates the pad rumble director if it exists.
 * @param pKit The actor kit.
 */
void updatePadRumbleDirector(LiveActorKit* pKit) {
    if (pKit->getPadRumbleDirector() != nullptr) {
        pKit->getPadRumbleDirector()->update();
    }
}
}  // namespace al
