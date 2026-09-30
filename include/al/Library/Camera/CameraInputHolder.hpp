#pragma once

#include <basis/seadTypes.h>

namespace al {
class ICameraInput;

class CameraInputHolder {
public:
    CameraInputHolder(s32 size);

    void initAfterPlacement();
    void setInput(const ICameraInput* pInput, s32 index);
    void updateInput();
    ICameraInput* getInput(s32 index) const;

    s32 getInputNum() const { return mInputsSize; }

private:
    ICameraInput* mDefaultInput = nullptr;
    ICameraInput** mInputs;
    s32 mInputsSize;
};

}  // namespace al
