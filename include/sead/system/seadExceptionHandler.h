#pragma once

#include <cstdarg>

#include <basis/seadTypes.h>
#include <prim/seadDelegateEventSlot.h>
#include <prim/seadSafeString.h>

namespace nn::diag
{
enum AbortReason
{
    AbortReason_SdkAssert,
    AbortReason_SdkRequires,
    AbortReason_UserAssert,
    AbortReason_Abort,
    AbortReason_UnexpectedDefault,
};

enum AssertionType
{
    AssertionType_SdkAssert,
    AssertionType_SdkRequires,
    AssertionType_UserAssert,
};

struct AbortInfo;
struct AssertionInfo;

struct LogMessage
{
    const char* format;
    std::va_list* args;
};
}  // namespace nn::diag

namespace sead
{
class ExceptionHandler
{
public:
    struct Information
    {
        Information();

        SafeString message;
        const void* _10 = nullptr;
        const void* _18 = nullptr;
    };
    static_assert(sizeof(Information) == 0x20);

    using InformationEvent = DelegateEvent<const Information&>;

    static void initialize();
    static void registerCallback(InformationEvent::Slot& rSlot);
    static void unregisterCallback(InformationEvent::Slot& rSlot);

private:
    static void dumpAbortInfo_(const nn::diag::AbortInfo& rInfo);
    static const char* getAbortReasonText_(const nn::diag::AbortReason& rReason);
    static void dumpAssertionInfo_(const nn::diag::AssertionInfo& rInfo);
    static const char* getAssertionTypeText_(const nn::diag::AssertionType& rType);
    static void makeLogMessage_(BufferedSafeString* pOut, const nn::diag::LogMessage* pMessage);

    static InformationEvent sEvent;
};
}  // namespace sead
