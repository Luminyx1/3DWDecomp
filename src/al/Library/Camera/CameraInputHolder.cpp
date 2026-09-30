#include "Library/Camera/CameraInputHolder.hpp"

#include "Library/Camera/SimpleCameraInput.hpp"

namespace al {

CameraInputHolder::CameraInputHolder(s32 size) : mInputsSize(1) {
    mInputs = new ICameraInput*[2];
    mInputs[0] = new SimpleCameraInput(1);
    mInputs[1] = new SimpleCameraInput(2);
}

void CameraInputHolder::initAfterPlacement() {
    if (!mDefaultInput) {
        mDefaultInput = new SimpleCameraInput(-1);
    }
}

void CameraInputHolder::setInput(const ICameraInput* pInput, s32 index) {
    mInputs[index] = const_cast<ICameraInput*>(pInput);
}

void CameraInputHolder::updateInput() {
    for (s32 i = 0; i < mInputsSize; i++) {
        if (mInputs[i]) {
            mInputs[i]->updateInput();
        }
    }
    if (mDefaultInput) {
        mDefaultInput->updateInput();
    }
}

ICameraInput* CameraInputHolder::getInput(s32 index) const {
    return mInputs[index] ?: mDefaultInput;
}

}  // namespace al
