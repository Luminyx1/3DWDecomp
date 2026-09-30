#include "Library/StageSwitch/StageSwitchWatcher.hpp"

#include "Library/StageSwitch/Core/StageSwitchDirector.hpp"
#include "Library/StageSwitch/StageSwitchAccesser.hpp"
#include "Library/StageSwitch/StageSwitchFunctorListener.hpp"
#include "Project/Camera/Main/CameraDirector_RS.hpp"

namespace al {
/**
 * Constructs a watcher for the switch of an accesser.
 * @param pListener listener to notify
 * @param pAccesser switch accesser
 */
StageSwitchWatcher::StageSwitchWatcher(StageSwitchListener* pListener,
                                       StageSwitchAccesser* pAccesser)
    : mListener(pListener), mAccesser(pAccesser) {
    mCameraDirector = pAccesser->mStageSwitchDirector->mCameraDirector;
}

/**
 * Notifies the listener when the switch state changed.
 */
void StageSwitchWatcher::update() {
    if (mAccesser->isDisasterMode() && !mCameraDirector->_102) {
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
