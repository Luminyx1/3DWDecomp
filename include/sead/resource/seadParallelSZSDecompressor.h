#pragma once

#include <mc/seadCoreInfo.h>
#include <resource/seadDecompressor.h>
#include <resource/seadSZSDecompressor.h>
#include <thread/seadEvent.h>
#include <thread/seadThread.h>

namespace sead
{
class ParallelSZSDecompressor : public Decompressor
{
public:
    class DecompThread : public Thread
    {
    public:
        DecompThread(s32 priority, Heap* pHeap);
        ~DecompThread() override = default;

        void initialize(void* pDst, u32 dstSize, const void* pSrc, u32 srcSize, u32 divSize);
        void requestDecompPart(u32 part);
        void waitDecompPart() { mPartEvent.wait(); }
        s32 getError() const { return mError; }

    protected:
        void calc_(MessageQueue::Element msg) override;

    private:
        SZSDecompressor::DecompContext mContext;
        Event mFinishEvent;
        Event mPartEvent;
        const u8* mSrc = nullptr;
        u32 mSrcSize = 0;
        u32 mDivSize = 0;
        s32 mError = 0;
    };
    static_assert(sizeof(DecompThread) == 0x198);

    ParallelSZSDecompressor(u32 workSize, s32 threadPriority, Heap* pHeap, u8* pWorkBuffer,
                            const CoreIdMask& rMask);
    ~ParallelSZSDecompressor() override;

    u8* tryDecompFromDevice(const ResourceMgr::LoadArg& rLoadArg, Resource* pResource,
                            u32* pOutSize, u32* pOutAllocSize, bool* pOutAllocated) override;

    void setDivSize(u32 divSize);

private:
    u32 mWorkSize;
    u8* mWorkBuffer;
    DecompThread mThread;
};

static_assert(sizeof(ParallelSZSDecompressor) == 0x220);

}  // namespace sead
