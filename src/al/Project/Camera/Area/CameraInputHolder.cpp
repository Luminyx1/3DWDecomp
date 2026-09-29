#include "Project/Camera/Area/CameraInputHolder.hpp"

#include "Project/Camera/Holder/SimpleCameraInput.hpp"

namespace al {
/**
 * @brief Creates the holder with inputs for the first and second controller.
 * @param inputNum The requested number of inputs (unused).
 */
CameraInputHolder::CameraInputHolder(s32 inputNum) : mInputNum(1) {
    mInputs = new ICameraInput*[2];
    mInputs[0] = new SimpleCameraInput(1);
    mInputs[1] = new SimpleCameraInput(2);
}

/** @brief Creates the default input reading the main controller if none was set. */
void CameraInputHolder::initAfterPlacement() {
    if (mDefaultInput == nullptr) {
        mDefaultInput = new SimpleCameraInput(-1);
    }
}

/**
 * @brief Replaces an input.
 * @param pInput The new input.
 * @param index The index of the input.
 */
void CameraInputHolder::setInput(const ICameraInput* pInput, s32 index) {
    mInputs[index] = const_cast<ICameraInput*>(pInput);
}

/** @brief Updates all inputs and the default input. */
void CameraInputHolder::updateInput() {
    for (s32 i = 0; i < mInputNum; i++) {
        if (mInputs[i] != nullptr) {
            mInputs[i]->updateInput();
        }
    }
    if (mDefaultInput != nullptr) {
        mDefaultInput->updateInput();
    }
}

/**
 * @brief Gets an input.
 * @param index The index of the input.
 * @return The input at the index, or the default input if none is set there.
 */
const ICameraInput* CameraInputHolder::getInput(s32 index) const {
    if (mInputs[index] != nullptr) {
        return mInputs[index];
    }
    return mDefaultInput;
}
}  // namespace al
