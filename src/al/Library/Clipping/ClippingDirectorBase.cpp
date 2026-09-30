#include "Library/Clipping/ClippingDirectorBase.hpp"

#include "Library/Clipping/ClippingJudge.hpp"
#include "Library/Execute/ExecuteUtil.hpp"
#include "Project/Clipping/ClippingFarAreaObserver.hpp"

namespace al {
bool ClippingDirectorBase::sLODDisabled = false;

/**
 * Constructs the clipping director and registers it for execution.
 * @param pExecuteDirector execute director
 * @param pAreaObjDirector area director
 * @param pPlayerHolder player holder
 * @param pSceneCameraInfo scene camera info
 * @param pCameraDirector camera director
 */
ClippingDirectorBase::ClippingDirectorBase(ExecuteDirector* pExecuteDirector,
                                           const AreaObjDirector* pAreaObjDirector,
                                           const PlayerHolder* pPlayerHolder,
                                           SceneCameraInfo* pSceneCameraInfo,
                                           CameraDirector_RS* pCameraDirector) {
    sLODDisabled = false;
    mFarAreaObserver = new ClippingFarAreaObserver(pAreaObjDirector, pPlayerHolder);
    mClippingJudge = new ClippingJudge(mFarAreaObserver, pSceneCameraInfo, pCameraDirector);
    registerExecutorUser(this, pExecuteDirector, "Clipping");
}

/**
 * Finishes initialization.
 */
void ClippingDirectorBase::endInit() {
    mFarAreaObserver->endInit();
}

/**
 * Updates the far clip area and the clipping frustum.
 */
void ClippingDirectorBase::execute() {
    mFarAreaObserver->update();
    mClippingJudge->update();
}

/**
 * Sets whether the clipping position is used as the player position.
 * @param isUse whether to use the clipping position
 */
void ClippingDirectorBase::setClippingJudgeUsClippingPosAsPlayerPos(bool isUse) {
    mClippingJudge->setUseClippingPosAsPlayerPos(isUse);
}
}  // namespace al
