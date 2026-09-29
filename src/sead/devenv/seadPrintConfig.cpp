#include <basis/seadRawPrint.h>
#include <devenv/seadPrintConfig.h>

namespace sead
{
bool PrintConfig::sIsPrintEventUsed = false;
PrintConfig::PrintEvent PrintConfig::sPrintEvent;
IDelegate1<const PrintConfig::PrintEventArg&>* PrintConfig::sFinalCallback = nullptr;

/**
 * Connects a slot that receives every printed string.
 * @param rSlot Slot to connect.
 */
void PrintConfig::registerCallback(PrintEvent::Slot& rSlot)
{
    sPrintEvent.connect(rSlot);
    sIsPrintEventUsed = true;
}

/**
 * Disconnects a slot previously connected with registerCallback.
 * @param rSlot Slot to disconnect.
 */
void PrintConfig::unregisterCallback(PrintEvent::Slot& rSlot)
{
    sPrintEvent.disconnect(rSlot);
}

/**
 * Sets the callback that replaces the default output.
 * @param pCallback Final callback (may be null).
 */
void PrintConfig::registerFinalCallback(IDelegate1<const PrintEventArg&>* pCallback)
{
    sFinalCallback = pCallback;
}

/**
 * Notifies the print slots, then outputs the string through the final callback or the platform.
 * @param rArg String to print.
 */
void PrintConfig::execCallbacks(const PrintEventArg& rArg)
{
    if (sIsPrintEventUsed)
    {
        sPrintEvent.emit(rArg);
    }

    if (sFinalCallback)
    {
        sFinalCallback->invoke(rArg);
    }
    else
    {
        system::PrintStringImpl(rArg.str, rArg.length);
    }
}
}  // namespace sead
