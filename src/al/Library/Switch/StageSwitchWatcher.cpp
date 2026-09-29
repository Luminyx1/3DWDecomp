#include "Library/StageSwitch/StageSwitchWatcher.hpp"

#include "Library/StageSwitch/Core/StageSwitchDirector.hpp"
#include "Library/StageSwitch/StageSwitchAccesser.hpp"
#include "Library/StageSwitch/StageSwitchFunctorListener.hpp"
#include "Project/Camera/Main/CameraDirector_RS.hpp"

namespace al {
    /**
     * @brief Constructs a watcher that notifies a listener when a switch changes state.
     * @param pListener The listener to notify.
     * @param pAccesser The accesser of the watched switch.
     */
    StageSwitchWatcher::StageSwitchWatcher(StageSwitchListener* pListener, StageSwitchAccesser* pAccesser)
        : mListener(pListener), mAccesser(pAccesser) {
        mCameraDirector = pAccesser->mStageSwitchDirector->mCameraDirector;
    }

    /**
     * @brief Checks the switch state and notifies the listener on changes.
     */
    void StageSwitchWatcher::update() {
        if (mAccesser->mIsDisasterMode && !mCameraDirector->mIsDisasterMode) {
            return;
        }

        bool isOn = mAccesser->isOnSwitch();
        if (isOn != _18) {
            if (isOn) {
                mListener->listenOn();
            } else {
                mListener->listenOff();
            }
        }
        _18 = isOn;
    }
}  // namespace al
