#include "Library/Application/ApplicationMessageReceiver.hpp"

#include <nn/am.h>

namespace al {
/**
 * Constructs the application message receiver.
 */
ApplicationMessageReceiver::ApplicationMessageReceiver() = default;

/**
 * Initializes the OE library and enables the notification messages.
 */
void ApplicationMessageReceiver::init() {
    nn::oe::Initialize();
    mOperationMode = getOperationMode();
    mPerformanceMode = getPerformaceMode();
    nn::oe::SetResumeNotificationEnabled(true);
    nn::oe::SetOperationModeChangedNotificationEnabled(true);
    nn::oe::SetPerformanceModeChangedNotificationEnabled(true);
    nn::oe::SetFocusHandlingMode(nn::oe::FocusHandlingMode_AlwaysSuspend);
}

/**
 * Gets the current operation mode.
 * @return operation mode
 */
nn::oe::OperationMode ApplicationMessageReceiver::getOperationMode() const {
    switch (nn::oe::GetOperationMode()) {
    case nn::oe::OperationMode_Handheld:
        return nn::oe::OperationMode_Handheld;
    case nn::oe::OperationMode_Docked:
        return nn::oe::OperationMode_Docked;
    default:
        return nn::oe::OperationMode_Handheld;
    }
}

/**
 * Gets the current performance mode.
 * @return performance mode
 */
nn::oe::PerformanceMode ApplicationMessageReceiver::getPerformaceMode() const {
    switch (nn::oe::GetPerformanceMode()) {
    case nn::oe::PerformanceMode_Normal:
        return nn::oe::PerformanceMode_Normal;
    case nn::oe::PerformanceMode_Boost:
        return nn::oe::PerformanceMode_Boost;
    default:
        return nn::oe::PerformanceMode_Normal;
    }
}

/**
 * Resets the per-frame flags and processes one notification message.
 */
void ApplicationMessageReceiver::update() {
    mIsUpdatedOperationMode = false;
    mIsUpdatedPerformanceMode = false;
    mIsResumed = false;
    u32 message;

    if (nn::oe::TryPopNotificationMessage(&message)) {
        procMessage(message);
    }
}

/**
 * Processes a notification message.
 * @param message applet message
 */
void ApplicationMessageReceiver::procMessage(u32 message) {
    switch (message) {
    case nn::am::AppletMessage_ExitRequested:
        mIsExitRequested = true;
        break;
    case nn::am::AppletMessage_FocusStateChanged:
        switch (nn::oe::GetCurrentFocusState()) {
        case nn::oe::FocusState_InFocus:
            mIsBackground = false;
            break;
        case nn::oe::FocusState_Background:
            mIsBackground = true;
            break;
        default:
            break;
        }

        break;
    case nn::am::AppletMessage_Resume:
        mIsResumed = true;
        break;
    case nn::am::AppletMessage_OperationModeChanged:
        mIsUpdatedOperationMode = true;
        mOperationMode = getOperationMode();
        break;
    case nn::am::AppletMessage_PerformanceModeChanged:
        mIsUpdatedPerformanceMode = true;
        mPerformanceMode = getPerformaceMode();
        break;
    default:
        break;
    }
}

/**
 * Clears the background flag.
 */
void ApplicationMessageReceiver::cancelBackground() {
    mIsBackground = false;
}
}  // namespace al
