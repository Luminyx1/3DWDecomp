#pragma once

#include <nn/oe.h>

namespace al {
    /// Receives and caches the notification messages sent to the application.
    class ApplicationMessageReceiver {
    public:
        ApplicationMessageReceiver();

        void init();
        nn::oe::OperationMode getOperationMode() const;
        nn::oe::PerformanceMode getPerformaceMode() const;
        void update();
        void procMessage(u32 message);
        void cancelBackground();

        bool mIsUpdatedOperationMode;               // _0
        bool mIsUpdatedPerformanceMode;             // _1
        bool mIsResumed;                            // _2
        bool mIsExitRequested;                      // _3
        bool mIsInBackground;                       // _4
        nn::oe::OperationMode mOperationMode;       // _8
        nn::oe::PerformanceMode mPerformanceMode;   // _C
    };
};
