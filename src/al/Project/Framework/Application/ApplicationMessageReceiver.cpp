#include "Project/Framework/Application/ApplicationMessageReceiver.hpp"
#include <nn/am.h>

namespace al {
    /** @brief Constructs the receiver in handheld mode with no pending messages. */
    ApplicationMessageReceiver::ApplicationMessageReceiver()
        : mIsUpdatedOperationMode(false), mIsUpdatedPerformanceMode(false), mIsResumed(false), mIsExitRequested(false),
          mIsInBackground(false), mOperationMode(nn::oe::OperationMode_Handheld),
          mPerformanceMode(nn::oe::PerformanceMode_Normal) {}

    /** @brief Initializes the oe library and enables the notifications the application handles. */
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
     * @brief Gets the current operation mode.
     * @return The current operation mode.
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
     * @brief Gets the current performance mode.
     * @return The current performance mode.
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

    /** @brief Clears the per-frame flags and handles the next pending message. */
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
     * @brief Handles a notification message.
     * @param message The message to handle.
     */
    void ApplicationMessageReceiver::procMessage(u32 message) {
        switch (message) {
        case nn::am::AppletMessage_ExitRequested:
            mIsExitRequested = true;
            break;
        case nn::am::AppletMessage_Resume:
            mIsResumed = true;
            break;
        case nn::am::AppletMessage_FocusStateChanged: {
            nn::oe::FocusState focusState = nn::oe::GetCurrentFocusState();
            if (focusState == nn::oe::FocusState_InFocus) {
                mIsInBackground = false;
            } else if (focusState == nn::oe::FocusState_Background) {
                mIsInBackground = true;
            }
            break;
        }
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

    /** @brief Clears the background state. */
    void ApplicationMessageReceiver::cancelBackground() {
        mIsInBackground = false;
    }
};
