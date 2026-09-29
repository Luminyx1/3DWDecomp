#pragma once

#include <basis/seadTypes.h>
#include <prim/seadDelegate.h>
#include <prim/seadDelegateEventSlot.h>

namespace sead
{
class PrintConfig
{
public:
    struct PrintEventArg
    {
        const char* str;
        s32 length;
    };

    using PrintEvent = DelegateEvent<const PrintEventArg&>;

    static void registerCallback(PrintEvent::Slot& rSlot);
    static void unregisterCallback(PrintEvent::Slot& rSlot);
    static void registerFinalCallback(IDelegate1<const PrintEventArg&>* pCallback);
    static void execCallbacks(const PrintEventArg& rArg);

private:
    static bool sIsPrintEventUsed;
    static PrintEvent sPrintEvent;
    static IDelegate1<const PrintEventArg&>* sFinalCallback;
};
}  // namespace sead
