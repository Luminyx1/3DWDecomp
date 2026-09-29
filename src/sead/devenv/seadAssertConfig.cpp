#include <devenv/seadAssertConfig.h>

namespace sead
{
AssertConfig::AssertEvent AssertConfig::sAssertEvent;
IDelegate1<const char*>* AssertConfig::sFinalCallback = nullptr;

/**
 * Connects a slot that is notified when an assertion fails.
 * @param slot Slot to connect.
 */
void AssertConfig::registerCallback(AssertEvent::Slot& slot)
{
    sAssertEvent.connect(slot);
}

/**
 * Disconnects a slot previously connected with registerCallback.
 * @param slot Slot to disconnect.
 */
void AssertConfig::unregisterCallback(AssertEvent::Slot& slot)
{
    sAssertEvent.disconnect(slot);
}

/**
 * Sets the callback that is invoked after all assert slots.
 * @param cb Final callback (may be null).
 */
void AssertConfig::registerFinalCallback(IDelegate1<const char*>* cb)
{
    sFinalCallback = cb;
}

/**
 * Notifies all assert slots and then the final callback.
 * @param assertMessage Assertion message.
 */
void AssertConfig::execCallbacks(const char* assertMessage)
{
    sAssertEvent.emit(assertMessage);
    if (sFinalCallback)
    {
        sFinalCallback->invoke(assertMessage);
    }
}
}  // namespace sead
