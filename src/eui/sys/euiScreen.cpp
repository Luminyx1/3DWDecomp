#include <eui/euiScreen.h>
namespace eui {
bool Screen::isOpened() const { return mState == 2 && mOpenRequest >= 0; }
bool Screen::isClosed() const { return mState == 0 && mOpenRequest < 1; }
// NON_MATCHING: the state/request expression compiles to different branches.
bool Screen::isOpening() const {
    return mState == 1 || ((mState == 0 || mState == 3) && mOpenRequest >= 1);
}
// NON_MATCHING: the state/request expression compiles to different branches.
bool Screen::isClosing() const {
    return mState == 3 || ((mState == 1 || mState == 2) && mOpenRequest < 0);
}
// own determines whether this screen owns its initialization heap.
void Screen::setOwnInitializeHeap(bool own) {
    if (own) mFlags |= 1;
    else mFlags &= ~1;
}
void Screen::muteNextNoOperationButtonOnSE_() { mNoOperationButtonOnSE = 0; }
}
