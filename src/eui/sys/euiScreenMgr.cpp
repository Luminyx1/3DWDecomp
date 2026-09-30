#include <eui/euiScreenMgr.h>
namespace eui {
// NON_MATCHING: final field store order and unresolved vtable relocations differ.
ScreenMgr::ScreenMgr() : _48(nullptr), mArcResourceMgr(nullptr), mBoxCursorMgr(nullptr),
    mAnimationStep(1), _430(nullptr), mFontMgr(nullptr), _440(true), _441(true),
    _442(false), _448(nullptr) {}
void ScreenMgr::updateViewer_() {}
// index selects the screen to remove from active drawing layers.
void ScreenMgr::inactivateScreen(int index) { mScreenLayers[index] = -1; }
}
