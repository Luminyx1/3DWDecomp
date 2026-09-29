#pragma once

#include <basis/seadTypes.h>

namespace al {
    /// Keeps the screen covered with a captured frame for a number of frames.
    class ScreenCoverCtrl {
    public:
        ScreenCoverCtrl();

        void requestCaptureScreenCover(s32 frames);
        void update();

        s32 mCoverFrames;                   // _0
        bool mIsRequestCapture;             // _4
        bool mIsRequestCaptureSceneCover;   // _5
    };
};
