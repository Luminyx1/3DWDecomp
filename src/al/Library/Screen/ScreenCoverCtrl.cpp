#include "Library/Screen/ScreenCoverCtrl.hpp"

namespace al {
/**
 * Constructs the screen cover control with no active cover.
 */
ScreenCoverCtrl::ScreenCoverCtrl() = default;

/**
 * Requests a screen cover capture for a number of frames.
 * @param coverFrames number of frames to cover the screen
 */
void ScreenCoverCtrl::requestCaptureScreenCover(s32 coverFrames) {
    if (mCoverFrames <= 0) {
        mIsRequestCapture = true;
    }

    if (mCoverFrames < coverFrames) {
        mCoverFrames = coverFrames;
    }
}

/**
 * Counts down the cover frames and clears the capture request.
 */
void ScreenCoverCtrl::update() {
    if (mCoverFrames > 0) {
        mCoverFrames--;
    }

    mIsRequestCapture = false;
}
}  // namespace al
