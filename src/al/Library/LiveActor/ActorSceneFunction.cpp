#include "Library/LiveActor/Util/ActorSceneUtil.hpp"

#include "Library/Controller/PadRumbleDirector.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorSceneInfo.hpp"
#include "Library/Scene/SceneStopCtrl.hpp"
#include "Library/Screen/ScreenCoverCtrl.hpp"
#include "Library/Sequence/DemoDirector.hpp"
#include "Project/Camera/Main/CameraDirector_RS.hpp"

namespace al {
/**
 * Checks whether the scene is currently stopped.
 * @param pActor An actor in the scene.
 * @return Whether the scene is stopped.
 */
bool isStopScene(const LiveActor* pActor) {
    SceneStopCtrl* ctrl = pActor->getSceneInfo()->sceneStopCtrl;

    if (ctrl->_4 != 0) {
        return false;
    }

    return ctrl->_0 > 0;
}

/**
 * Requests a captured screen cover for a number of frames.
 * @param pActor The requesting actor.
 * @param coverFrames The number of frames to show the cover.
 */
void requestCaptureScreenCover(const LiveActor* pActor, s32 coverFrames) {
    pActor->getSceneInfo()->screenCoverCtrl->requestCaptureScreenCover(coverFrames);
}

/**
 * Requests a captured screen cover of the scene.
 * @param pActor The requesting actor.
 */
void requestCaptureScreenSceneCover(const LiveActor* pActor) {
    pActor->getSceneInfo()->screenCoverCtrl->mIsRequestCaptureScene = true;
}

/**
 * Cancels the request for a captured screen cover of the scene.
 * @param pActor The requesting actor.
 */
void resetRequestCaptureScreenSceneCover(const LiveActor* pActor) {
    pActor->getSceneInfo()->screenCoverCtrl->mIsRequestCaptureScene = false;
}

/**
 * Requests a demo to start.
 * @param pActor The requesting actor.
 * @param pName The demo name.
 * @return Whether the demo started.
 */
bool requestStartDemo(const LiveActor* pActor, const char* pName) {
    return pActor->getSceneInfo()->demoDirector->tryRequestStartDemo(pActor, pName);
}

/**
 * Requests a demo to end.
 * @param pActor The requesting actor.
 * @param pName The demo name.
 */
void requestEndDemo(const LiveActor* pActor, const char* pName) {
    pActor->getSceneInfo()->demoDirector->requestEndDemo(pActor, pName);
}

/**
 * Registers an actor for demos.
 * @param pActor The actor.
 */
void addDemoActor(LiveActor* pActor) {
    pActor->getSceneInfo()->demoDirector->addDemoActor(pActor);
}

/**
 * Enables or disables the camera's disaster mode.
 * @param pActor An actor in the scene.
 * @param isDisaster Whether disaster mode is enabled.
 */
void setDisasterMode(LiveActor* pActor, bool isDisaster) {
    if (pActor->getSceneInfo()->cameraDirector == nullptr) {
        return;
    }

    CameraDirector_RS* director = pActor->getSceneInfo()->cameraDirector;
    director->_102 = isDisaster;
    director->setDisasterAreaCheck(isDisaster);
}

/**
 * Checks whether the camera's disaster mode is enabled.
 * @param pActor An actor in the scene.
 * @return Whether disaster mode is enabled.
 */
bool isDisasterMode(LiveActor* pActor) {
    if (pActor->getSceneInfo()->cameraDirector == nullptr) {
        return false;
    }

    return pActor->getSceneInfo()->cameraDirector->_102;
}

/**
 * Stops all pad rumbles, if there is a rumble director.
 * @param pActor An actor in the scene.
 */
void stopAllPadRumble(LiveActor* pActor) {
    PadRumbleDirector* director = pActor->getSceneInfo()->padRumbleDirector;

    if (director != nullptr) {
        director->stopAllRumble();
    }
}

/**
 * Checks whether the scene is in single player mode.
 * @param pActor An actor in the scene.
 * @return Whether the scene is in single player mode.
 */
bool isSingleMode(const LiveActor* pActor) {
    return pActor->getSceneInfo()->isSingleMode;
}
}  // namespace al
