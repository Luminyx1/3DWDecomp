#include <erepo/Data/SendThread.h>

#include <basis/seadNew.h>
#include <erepo/Data/SendDataBase.h>
#include <erepo/Manager.h>
#include <nn/prepo.h>

namespace erepo {

/**
 * Allocates the send queue.
 * @param pHeap Heap to allocate from.
 * @param queueSize Queue capacity (a power of two).
 * @return Always true.
 */
bool SendThread::initialize_(sead::Heap* pHeap, s32 queueSize)
{
    mQueue.tryAllocBuffer(queueSize, pHeap);
    mQueue.fill(nullptr);
    return true;
}

/**
 * Saves all queued data and frees the send queue.
 * @return Always true.
 */
bool SendThread::finalize_()
{
    invokeQueue_();
    mQueue.freeBuffer();
    return true;
}

/**
 * Saves and deletes all queued data once the manager has started up.
 */
void SendThread::invokeQueue_()
{
    if (!Manager::instance()->isFlagOn(Manager::EFlag::cStartupFinished)) {
        return;
    }

    while (SendDataBase* pSendData = popQueue_()) {
        SendDataBase::ESendResult result;
        if (pSendData->save(&result) || result != SendDataBase::ESendResult::cBusy) {
            delete pSendData;
        }
    }

    mFlags.setOff(EFlag::cSending);
}

/**
 * Queues data to be saved on the send thread.
 * @param pSendData Data to save.
 * @return Whether the data was queued.
 */
bool SendThread::requestSave(SendDataBase* pSendData)
{
    mFlags.setOn(EFlag::cSending);
    if (!pSendData) {
        return false;
    }

    return pushQueue_(pSendData);
}

/**
 * Requests immediate transmission of saved play reports.
 */
void SendThread::immediateTransmission_() const
{
    nn::prepo::RequestImmediateTransmission();
}

/**
 * Handles a thread message.
 * @param msg Message to handle.
 */
void SendThread::calc_(sead::MessageQueue::Element msg)
{
    switch (msg) {
    case cMsgSave:
        invokeQueue_();
        break;
    case cMsgImmediateTransmission:
        immediateTransmission_();
        break;
    default:
        break;
    }
}

}  // namespace erepo
