#include "Library/LiveActor/Util/ActorSceneUtil.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorSceneInfo.hpp"
#include "Library/Scene/SceneStopCtrl.hpp"
#include "Library/Screen/ScreenCoverCtrl.hpp"
#include "Library/Sequence/DemoDirector.hpp"
#include "Project/Camera/Main/CameraDirector_RS.hpp"
#include "Project/Controller/PadRumbleDirector.hpp"

namespace al {
    /**
     * @brief Requests a scene stop (hit stop).
     * @param pActor The actor requesting the stop.
     * @param stopFrames The number of frames to stop the scene for.
     * @param delayFrames The number of frames to wait before stopping.
     * @param flag8 Stored in SceneStopCtrl::_8 (meaning unknown).
     * @param flag9 Stored in SceneStopCtrl::_9 (meaning unknown).
     */
    void stopScene(const LiveActor* pActor, int stopFrames, int delayFrames, bool flag8, bool flag9) {
        pActor->getSceneInfo()->sceneStopCtrl->reqeustStopScene(stopFrames, delayFrames, flag8, flag9);
    }

    /**
     * @brief Checks whether the scene is currently stopped.
     * @param pActor Any actor in the scene.
     * @return True if a stop is running and no longer waiting for its delay.
     */
    bool isStopScene(const LiveActor* pActor) {
        const SceneStopCtrl* pCtrl = pActor->getSceneInfo()->sceneStopCtrl;

        if (pCtrl->_4 != 0) {
            return false;
        }

        return pCtrl->_0 > 0;
    }

    /**
     * @brief Requests that the screen be covered with a captured frame.
     * @param pActor Any actor in the scene.
     * @param coverFrames The number of frames to keep the cover.
     */
    void requestCaptureScreenCover(const LiveActor* pActor, int coverFrames) {
        pActor->getSceneInfo()->screenCoverCtrl->requestCaptureScreenCover(coverFrames);
    }

    /**
     * @brief Requests that the scene be covered with a captured frame.
     * @param pActor Any actor in the scene.
     */
    void requestCaptureScreenSceneCover(const LiveActor* pActor) {
        pActor->getSceneInfo()->screenCoverCtrl->mIsRequestCaptureSceneCover = true;
    }

    /**
     * @brief Cancels a pending scene cover request.
     * @param pActor Any actor in the scene.
     */
    void resetRequestCaptureScreenSceneCover(const LiveActor* pActor) {
        pActor->getSceneInfo()->screenCoverCtrl->mIsRequestCaptureSceneCover = false;
    }

    /**
     * @brief Requests the start of a demo (cutscene).
     * @param pActor The actor starting the demo.
     * @param pDemoName The name of the demo.
     * @return True if the demo could be started.
     */
    bool requestStartDemo(const LiveActor* pActor, const char* pDemoName) {
        return pActor->getSceneInfo()->demoDirector->tryRequestStartDemo(pActor, pDemoName);
    }

    /**
     * @brief Requests the end of a demo (cutscene).
     * @param pActor The actor that started the demo.
     * @param pDemoName The name of the demo.
     */
    void requestEndDemo(const LiveActor* pActor, const char* pDemoName) {
        pActor->getSceneInfo()->demoDirector->requestEndDemo(pActor, pDemoName);
    }

    /**
     * @brief Adds an actor to the ones that keep running during demos.
     * @param pActor The actor to add.
     */
    void addDemoActor(LiveActor* pActor) {
        pActor->getSceneInfo()->demoDirector->addDemoActor(pActor);
    }

    /**
     * @brief Turns the camera's disaster mode on or off, if the scene has a camera director.
     * @param pActor Any actor in the scene.
     * @param isDisaster Whether disaster mode should be on.
     */
    void setDisasterMode(LiveActor* pActor, bool isDisaster) {
        if (pActor->getSceneInfo()->cameraDirector != nullptr) {
            CameraDirector_RS* pDirector = pActor->getSceneInfo()->cameraDirector;
            pDirector->mIsDisasterMode = isDisaster;
            pDirector->setDisasterAreaCheck(isDisaster);
        }
    }

    /**
     * @brief Checks whether the camera is in disaster mode.
     * @param pActor Any actor in the scene.
     * @return True if the scene has a camera director in disaster mode.
     */
    bool isDisasterMode(LiveActor* pActor) {
        if (pActor->getSceneInfo()->cameraDirector != nullptr) {
            return pActor->getSceneInfo()->cameraDirector->mIsDisasterMode;
        }

        return false;
    }

    /**
     * @brief Stops every pad rumble, if the scene has a rumble director.
     * @param pActor Any actor in the scene.
     */
    void stopAllPadRumble(LiveActor* pActor) {
        PadRumbleDirector* pDirector = pActor->getSceneInfo()->padRumbleDirector;

        if (pDirector != nullptr) {
            pDirector->stopAllRumble();
        }
    }

    /**
     * @brief Checks whether the game runs in single-player mode.
     * @param pActor Any actor in the scene.
     * @return The scene's single mode flag.
     */
    bool isSingleMode(const LiveActor* pActor) {
        return pActor->getSceneInfo()->isSingleMode;
    }
};
