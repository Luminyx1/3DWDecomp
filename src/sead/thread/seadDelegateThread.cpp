#include "thread/seadDelegateThread.h"
#include "prim/seadDelegate.h"

namespace sead
{
DelegateThread::DelegateThread(const SafeString& rName, IDelegate2<Thread*, MessageQueue::Element>* pDelegate,
                               Heap* pHeap, s32 priority, MessageQueue::BlockType blockType,
                               MessageQueue::Element quitMsg, s32 stackSize, s32 messageQueueSize)
    : Thread(rName, pHeap, priority, blockType, quitMsg, stackSize, messageQueueSize), mDelegate(pDelegate)
{
}

DelegateThread::~DelegateThread() = default;

void DelegateThread::calc_(MessageQueue::Element msg)
{
    mDelegate->invoke(this, msg);
}
}  // namespace sead
