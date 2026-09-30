#pragma once

#include <nn/oe.h>

namespace al {
class ApplicationMessageReceiver {
public:
    ApplicationMessageReceiver();

    void init();
    nn::oe::OperationMode getOperationMode() const;
    nn::oe::PerformanceMode getPerformaceMode() const;
    void update();
    void procMessage(u32 message);
    void cancelBackground();

    bool isUpdatedOperationMode() const { return mIsUpdatedOperationMode; }
    bool isUpdatedPerformanceMode() const { return mIsUpdatedPerformanceMode; }
    bool isResumed() const { return mIsResumed; }
    bool isExitRequested() const { return mIsExitRequested; }
    bool isBackground() const { return mIsBackground; }
    nn::oe::OperationMode getCachedOperationMode() const { return mOperationMode; }
    nn::oe::PerformanceMode getCachedPerformanceMode() const { return mPerformanceMode; }

    bool mIsUpdatedOperationMode = false;
    bool mIsUpdatedPerformanceMode = false;
    bool mIsResumed = false;
    bool mIsExitRequested = false;
    bool mIsBackground = false;
    nn::oe::OperationMode mOperationMode = nn::oe::OperationMode_Handheld;
    nn::oe::PerformanceMode mPerformanceMode = nn::oe::PerformanceMode_Normal;
};

static_assert(sizeof(ApplicationMessageReceiver) == 0x10);
}  // namespace al
