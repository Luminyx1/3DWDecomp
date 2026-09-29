#pragma once

#include <basis/seadTypes.h>

namespace al {
class ICameraInput;

/// Holds the camera input of every view, falling back to a default input.
class CameraInputHolder {
public:
    CameraInputHolder(s32 inputNum);

    void initAfterPlacement();
    void setInput(const ICameraInput* pInput, s32 index);
    void updateInput();
    const ICameraInput* getInput(s32 index) const;

    ICameraInput* mDefaultInput = nullptr;  // _0
    ICameraInput** mInputs;                 // _8
    s32 mInputNum;                          // _10
};
}  // namespace al
