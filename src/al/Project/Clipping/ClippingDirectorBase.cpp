#include "Project/Clipping/ClippingDirectorBase.hpp"
#include "Project/Clipping/ClippingFarAreaObserver.hpp"
#include "Project/Clipping/ClippingJudge.hpp"

namespace al {
    void registerExecutorUser(IUseExecutor* pUser, ExecuteDirector* pDirector, const char* pName);

    bool ClippingDirectorBase::sLODDisabled;

    /**
     * @brief Constructs the clipping director's far area observer and judge and registers it for execution.
     * @param pExecuteDirector The scene's execute director.
     * @param pAreaObjDirector The scene's area director.
     * @param pPlayerHolder The scene's player holder.
     * @param pSceneCameraInfo The scene's camera info.
     * @param pCameraDirector The scene's camera director.
     */
    ClippingDirectorBase::ClippingDirectorBase(ExecuteDirector* pExecuteDirector, const AreaObjDirector* pAreaObjDirector,
                                               const PlayerHolder* pPlayerHolder, SceneCameraInfo* pSceneCameraInfo,
                                               CameraDirector_RS* pCameraDirector)
        : mClippingJudge(nullptr), mFarAreaObserver(nullptr), _18(nullptr) {
        sLODDisabled = false;
        mFarAreaObserver = new ClippingFarAreaObserver(pAreaObjDirector, pPlayerHolder);
        mClippingJudge = new ClippingJudge(mFarAreaObserver, pSceneCameraInfo, pCameraDirector);
        registerExecutorUser(this, pExecuteDirector, "Clipping");
    }

    /** @brief Finishes initialization once all actors and areas are registered. */
    void ClippingDirectorBase::endInit() {
        mFarAreaObserver->endInit();
    }

    /** @brief Updates the far clip distance and the view frustum. */
    void ClippingDirectorBase::execute() {
        mFarAreaObserver->update();
        mClippingJudge->update();
    }

    /**
     * @brief Sets whether the camera position is used as the player's position for clipping.
     * @param isUse Whether to use the camera position.
     */
    void ClippingDirectorBase::setClippingJudgeUsClippingPosAsPlayerPos(bool isUse) {
        mClippingJudge->setIsUseClippingPosAsPlayerPos(isUse);
    }
};
