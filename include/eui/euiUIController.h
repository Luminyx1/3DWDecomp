#pragma once

#include <controller/seadMaskControllerWrapper.h>

namespace eui {

class UIController : public sead::MaskControllerWrapper {
    SEAD_RTTI_OVERRIDE(UIController, sead::MaskControllerWrapper)

public:
    UIController();
    /** @brief Releases the controller wrapper and its disposer linkage. */
    ~UIController() override = default;
};

}  // namespace eui
