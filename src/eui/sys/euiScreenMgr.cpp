#include <eui/euiScreenMgr.h>
#include <eui/euiScreen.h>
#include <eui/euiFontMgr.h>
#include <eui/euiScalableFontMgr.h>
#include <heap/seadHeap.h>
namespace eui {
// NON_MATCHING: loop register allocation differs.
// layer selects the active screens to draw; pInfo supplies the destination render buffers.
void ScreenMgr::draw(s8 layer, const DrawInfoEx::RenderBufferInfo* pInfo) {
    const u32 count = mScreenLayers.size();
    const auto* layers = mScreenLayers.getBufferPtr();
    for (size_t i = 0; i != count; ++i) {
        if (layers[i] == layer) mScreens.getBufferPtr()[i < u32(mScreens.size()) ? i : 0]->draw(pInfo);
    }
}
// index selects the screen entry to clear without destroying its initialization heap.
void ScreenMgr::resetScreenId(int index) {
    mScreenLayers[index] = -1;
    mScreens[index] = nullptr;
}
// index selects a screen to detach; its initialization heap is destroyed only when owned.
void ScreenMgr::unloadScreen(int index) {
    Screen* screen = mScreens[index];
    if (screen) {
        resetScreenId(index);
        if (screen->mFlags & 1) screen->mInitializeHeap->destroy();
    }
}
// pNode is removed from navigation routes in every loaded screen.
void ScreenMgr::eraseBoxCursorNodeFromRouteNodes(const BoxCursorNode* pNode) {
    for (auto* screen : mScreens) if (screen) screen->eraseBoxCursorNodeFromRouteNodes(pNode);
}
// NON_MATCHING: creation still inherits the constructor mismatch; disposer calls await its destructor.
SEAD_SINGLETON_DISPOSER_IMPL(ScreenMgr);
// NON_MATCHING: final field store order and unresolved vtable relocations differ.
ScreenMgr::ScreenMgr() : _48(nullptr), mArcResourceMgr(nullptr), mBoxCursorMgr(nullptr),
    mAnimationStep(1), _430(nullptr), mFontMgr(nullptr), _440(true), _441(true),
    _442(false), _448(nullptr) {}
void ScreenMgr::updateViewer_() {}
// index selects the screen to remove from active drawing layers.
void ScreenMgr::inactivateScreen(int index) { mScreenLayers[index] = -1; }
// index selects the screen whose configured drawing layer becomes active.
void ScreenMgr::activateScreen(int index) { mScreenLayers[index] = mScreens[index]->mDrawLayer; }
void ScreenMgr::updateSystem() {
    _440 = true;
    _441 = true;
    if (mFontMgr->mScalableFontMgr) mFontMgr->mScalableFontMgr->update();
}
}
