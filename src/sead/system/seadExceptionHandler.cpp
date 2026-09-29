#include <system/seadExceptionHandler.h>

namespace sead
{
ExceptionHandler::InformationEvent ExceptionHandler::sEvent;

/**
 * Creates empty exception information.
 */
ExceptionHandler::Information::Information() = default;

/**
 * Installs the exception handlers; does nothing in release builds.
 */
void ExceptionHandler::initialize() {}

/**
 * Connects a slot that is notified when an exception occurs.
 * @param rSlot Slot to connect.
 */
void ExceptionHandler::registerCallback(InformationEvent::Slot& rSlot)
{
    sEvent.connect(rSlot);
}

/**
 * Disconnects a slot previously connected with registerCallback.
 * @param rSlot Slot to disconnect.
 */
void ExceptionHandler::unregisterCallback(InformationEvent::Slot& rSlot)
{
    sEvent.disconnect(rSlot);
}

/**
 * Prints abort information; does nothing in release builds.
 * @param rInfo Abort information.
 */
void ExceptionHandler::dumpAbortInfo_(const nn::diag::AbortInfo& rInfo) {}

/**
 * Returns the name of an SDK abort reason.
 * @param rReason Abort reason.
 * @return Name of the reason.
 */
const char* ExceptionHandler::getAbortReasonText_(const nn::diag::AbortReason& rReason)
{
    switch (rReason)
    {
        case nn::diag::AbortReason_SdkAssert:
            return "SdkAssert";
        case nn::diag::AbortReason_SdkRequires:
            return "SdkRequires";
        case nn::diag::AbortReason_UserAssert:
            return "UserAssert";
        case nn::diag::AbortReason_Abort:
            return "Abort";
        case nn::diag::AbortReason_UnexpectedDefault:
            return "UnexpectedDefault";
        default:
            return "unknown reason";
    }
}

/**
 * Prints assertion information; does nothing in release builds.
 * @param rInfo Assertion information.
 */
void ExceptionHandler::dumpAssertionInfo_(const nn::diag::AssertionInfo& rInfo) {}

/**
 * Returns the name of an SDK assertion type.
 * @param rType Assertion type.
 * @return Name of the type.
 */
const char* ExceptionHandler::getAssertionTypeText_(const nn::diag::AssertionType& rType)
{
    switch (rType)
    {
        case nn::diag::AssertionType_SdkAssert:
            return "SdkAssert";
        case nn::diag::AssertionType_SdkRequires:
            return "SdkRequires";
        case nn::diag::AssertionType_UserAssert:
            return "UserAssert";
        default:
            return "unknown type";
    }
}

/**
 * Formats an SDK log message, using a fallback text if the message is empty.
 * @param pOut Output string.
 * @param pMessage SDK log message.
 */
void ExceptionHandler::makeLogMessage_(BufferedSafeString* pOut,
                                       const nn::diag::LogMessage* pMessage)
{
    pOut->formatV(pMessage->format, *pMessage->args);
    if (pOut->isEmpty())
    {
        pOut->copy("SDK has been aborted or failed assertion with no message.");
    }
}
}  // namespace sead
