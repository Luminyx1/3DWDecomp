#include "basis/seadRawPrint.h"
#include "thread/seadEvent.h"

namespace sead
{
/**
 * Creates an event owned by the current heap; call initialize() before using it.
 */
Event::Event() : IDisposer()
{
}

/**
 * Creates and initializes an event owned by the current heap.
 * @param isManualReset whether the signal stays set until resetSignal() (otherwise a wait clears it)
 */
Event::Event(bool isManualReset) : Event()
{
    initialize(isManualReset);
}

/**
 * Sets the event up, unsignalled.
 * @param isManualReset whether the signal stays set until resetSignal() (otherwise a wait clears it)
 */
void Event::initialize(bool isManualReset)
{
    nn::os::InitializeLightEvent(&mEventInner, false,
                                 isManualReset ? nn::os::EventClearMode_ManualClear :
                                                 nn::os::EventClearMode_AutoClear);
    setInitialized(true);
}

/**
 * Creates an event; call initialize() before using it.
 * @param pDisposerHeap heap that destroys the event with itself, or nullptr for the heap containing it
 */
Event::Event(Heap* pDisposerHeap) : Event(pDisposerHeap, HeapNullOption::UseSpecifiedOrContainHeap)
{
}

/**
 * Creates an event; call initialize() before using it.
 * @param pDisposerHeap heap that destroys the event with itself
 * @param heapNullOption what to do when pDisposerHeap is nullptr
 */
Event::Event(Heap* pDisposerHeap, IDisposer::HeapNullOption heapNullOption)
    : IDisposer(pDisposerHeap, heapNullOption)
{
}

/**
 * Creates and initializes an event.
 * @param pDisposerHeap heap that destroys the event with itself, or nullptr for the heap containing it
 * @param isManualReset whether the signal stays set until resetSignal()
 */
Event::Event(Heap* pDisposerHeap, bool isManualReset) : Event(pDisposerHeap)
{
    initialize(isManualReset);
}

/**
 * Creates and initializes an event.
 * @param pDisposerHeap heap that destroys the event with itself
 * @param heapNullOption what to do when pDisposerHeap is nullptr
 * @param isManualReset whether the signal stays set until resetSignal()
 */
Event::Event(Heap* pDisposerHeap, IDisposer::HeapNullOption heapNullOption, bool isManualReset)
    : Event(pDisposerHeap, heapNullOption)
{
    initialize(isManualReset);
}

/**
 * Finalizes the event.
 */
Event::~Event()
{
    setInitialized(false);
    nn::os::FinalizeLightEvent(&mEventInner);
}

/**
 * Waits until the event is signalled.
 */
void Event::wait()
{
    nn::os::WaitLightEvent(&mEventInner);
}

/**
 * Waits until the event is signalled or the time is up.
 * @param duration longest time to wait
 * @return whether the event was signalled
 */
bool Event::wait(TickSpan duration)
{
    return nn::os::TimedWaitLightEvent(&mEventInner, nn::os::ConvertToTimeSpan(duration.toS64()));
}

/**
 * Signals the event, waking waiting threads.
 */
void Event::setSignal()
{
    nn::os::SignalLightEvent(&mEventInner);
}

/**
 * Clears the signal.
 */
void Event::resetSignal()
{
    nn::os::ClearLightEvent(&mEventInner);
}
}  // namespace sead
