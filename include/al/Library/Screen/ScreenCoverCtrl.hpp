#pragma once

#include <basis/seadTypes.h>

namespace al {
class ScreenCoverCtrl {
public:
    ScreenCoverCtrl();

    void requestCaptureScreenCover(s32 coverFrames);
    void update();

    s32 mCoverFrames = -1;
    bool mIsRequestCapture = false;
    bool mIsRequestCaptureScene = false;
};
}  // namespace al
