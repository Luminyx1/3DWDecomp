#pragma once

#include <atomic>
#include <basis/seadTypes.h>
#include <container/seadBuffer.h>
#include <nn/types.h>
#include <prim/seadEnum.h>
#include <thread/seadAtomic.h>
#include <thread/seadThread.h>

#include <erepo/Data/AtomicBitFlag.h>

namespace erepo {

class SendDataBase;

class SendThread : public sead::Thread {
public:
    SEAD_ENUM(EFlag, cSending)

    static constexpr sead::MessageQueue::Element cMsgSave = 'save';
    static constexpr sead::MessageQueue::Element cMsgImmediateTransmission = 'imtr';

    SendThread(sead::Heap* pHeap, s32 priority, s32 queueSize, s32 stackSize,
               s32 messageQueueSize)
        : sead::Thread("PlayReporter", pHeap, priority, sead::MessageQueue::BlockType::Blocking,
                       0x7fffffff, stackSize, messageQueueSize)
    {
        initialize_(pHeap, queueSize);
    }

    ~SendThread() override { finalize_(); }

    bool initialize_(sead::Heap* pHeap, s32 queueSize);
    bool finalize_();
    bool requestSave(SendDataBase* pSendData);

    bool isSending() const { return mFlags.isOn(EFlag::cSending); }
    s32 getQueuedNum() const { return mWriteIndex.load() - mReadIndex; }

protected:
    void calc_(sead::MessageQueue::Element msg) override;

private:
    void invokeQueue_();
    void immediateTransmission_() const;

    bool pushQueue_(SendDataBase* pSendData)
    {
        u32 index = mWriteIndex.load();
        do {
            if (static_cast<s32>(index - mReadIndex) >= mQueue.size()) {
                return false;
            }
        } while (!mWriteIndex.compareExchange(index, index + 1, &index));
        asm volatile("dmb ish" ::: "memory");
        mQueue.getBufferPtr()[(mQueue.size() - 1) & index] = pSendData;
        return true;
    }

    SendDataBase* popQueue_()
    {
        while (true) {
            const u32 index = mReadIndex;
            if (static_cast<s32>(mWriteIndex.load() - index) < 1) {
                return nullptr;
            }

            SendDataBase** pSlot = &mQueue.getBufferPtr()[(mQueue.size() - 1) & index];
            SendDataBase* pSendData = *pSlot;
            if (!pSendData) {
                return nullptr;
            }

            *pSlot = nullptr;
            asm volatile("dmb ish" ::: "memory");
            mReadIndex = index + 1;
            if (pSendData != reinterpret_cast<SendDataBase*>(-1)) {
                return pSendData;
            }
        }
    }

    sead::Buffer<SendDataBase*> mQueue;
    sead::Atomic<u32> mWriteIndex = 0;
    u32 mReadIndex = 0;
    AtomicBitFlag<EFlag> mFlags{0u};
};

}  // namespace erepo
