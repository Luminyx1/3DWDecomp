#include <eui/euiScreen.h>

#include "Library/Layout/LayoutKeeper.hpp"
#include "Library/Layout/LayoutPaneGroup.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
namespace {
inline nn::ui2d::Pane* getRootPane(const nn::ui2d::Layout* pLayout) {
    return *reinterpret_cast<nn::ui2d::Pane* const*>(reinterpret_cast<const u8*>(pLayout) + 0x18);
}
}  // namespace

/**
 * Sets the screen the layout belongs to.
 * @param pScreen screen
 */
void LayoutKeeper::initScreen(eui::Screen* pScreen) {
    mScreen = pScreen;
}

/**
 * Sets the draw info used to draw the layout.
 * @param pDrawInfo draw info
 */
void LayoutKeeper::initDrawInfo(nn::ui2d::DrawInfo* pDrawInfo) {
    mDrawInfo = pDrawInfo;
}

/**
 * Finds a pane group by name.
 * @param pGroupName group name
 * @return the group, or nullptr
 */
LayoutPaneGroup* LayoutKeeper::getGroup(const char* pGroupName) const {
    for (s32 i = 0; i < mGroupNum; i++) {
        if (isEqualString(pGroupName, mGroups[i]->getGroupName())) {
            return mGroups[i];
        }
    }

    return nullptr;
}

/**
 * Returns a pane group by index.
 * @param index group index
 * @return the group
 */
LayoutPaneGroup* LayoutKeeper::getGroup(s32 index) const {
    return mGroups[index];
}

/**
 * Returns the number of pane groups.
 * @return number of groups
 */
s32 LayoutKeeper::getGroupNum() const {
    return mGroupNum;
}

/**
 * Advances the layout's animations.
 * @param isRecursive whether to let the screen update all of its animators
 */
void LayoutKeeper::calcAnim(bool isRecursive) {
    if (isRecursive) {
        if (mScreen) {
            mScreen->updateAnimator_();
        }

        return;
    }

    if (mScreen) {
        requestCaptureRecursive(getRootPane(mLayout));
    }

    s32 groupNum = mGroupNum;
    for (s32 i = 0; i < groupNum; i++) {
        mGroups[i]->animate(false);
    }
}
}  // namespace al
