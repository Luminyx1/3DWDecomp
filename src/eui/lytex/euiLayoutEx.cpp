#include <eui/euiLayoutEx.h>
namespace eui {
// pScreen is the screen that owns this layout.
LayoutEx::LayoutEx(Screen* pScreen) : _60(nullptr), _68(nullptr), _70(nullptr), _78(nullptr),
    mScreen(pScreen), mParentLayout(nullptr), mFlags(0x200) {}
// NON_MATCHING: branch relocation awaits tryCreateAnimatorAuto reconstruction.
// pName identifies an animation; enabled sets its initial enabled state.
Animator* LayoutEx::createAnimatorAuto(const char* pName, bool enabled) {
    return tryCreateAnimatorAuto(pName, enabled);
}
// pPane and rArgs identify the newly built pane and its build context; the base hook does nothing.
void LayoutEx::afterBuildPane_(nn::ui2d::Pane* pPane, const nn::ui2d::BuildArgSet& rArgs) {}
}
