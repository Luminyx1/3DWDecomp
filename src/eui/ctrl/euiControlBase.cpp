#include <eui/euiControlBase.h>

namespace eui {

/** @brief Returns the base control class name. */
const char* ControlBase::getClassName() const {
    return "ControlBase";
}

/** @brief Creates an unlinked control with empty context pointers. */
ControlBase::ControlBase() : _18(nullptr), _20(nullptr) {}

/** @brief Destroys the base control. */
ControlBase::~ControlBase() = default;

/** @brief Provides the default no-op control update. */
void ControlBase::Update(float) {}

static_assert(sizeof(ControlBase) == 0x28, "ControlBase size");

}  // namespace eui
