#include <filedevice/seadFileDeviceMgr.h>
#include <heap/seadHeapMgr.h>
#include <math/seadMathCalcCommon.h>
#include <resource/seadParallelSZSDecompressor.h>

namespace sead
{
/**
 * Creates the decompressor and starts its worker thread.
 * @param workSize Size of each streamed read.
 * @param threadPriority Priority of the worker thread.
 * @param pHeap Heap for the worker thread.
 * @param pWorkBuffer Read buffer (twice workSize), or null to allocate one per load.
 * @param rMask Cores the worker thread may run on.
 */
ParallelSZSDecompressor::ParallelSZSDecompressor(u32 workSize, s32 threadPriority, Heap* pHeap,
                                                 u8* pWorkBuffer, const CoreIdMask& rMask)
    : Decompressor("szs"), mWorkBuffer(pWorkBuffer), mThread(threadPriority, pHeap)
{
    mWorkSize = Mathu::roundUpPow2(workSize, FileDevice::cBufferMinAlignment);
    mThread.setAffinity(rMask);
    mThread.start();
}

/**
 * Stops the worker thread.
 */
ParallelSZSDecompressor::~ParallelSZSDecompressor()
{
    mThread.quitAndDestroySingleThread(false);
}

// NON_MATCHING: loop induction variable strength reduction
u8* ParallelSZSDecompressor::tryDecompFromDevice(const ResourceMgr::LoadArg& rLoadArg,
                                                 Resource* pResource, u32* pOutSize,
                                                 u32* pOutAllocSize, bool* pOutAllocated)
{
    Heap* heap = rLoadArg.load_data_heap;

    if (heap == nullptr)
    {
        heap = HeapMgr::sInstancePtr->getCurrentHeap();
    }

    if ((rLoadArg.load_data_buffer_alignment & 0x1f) != 0)
    {
        return nullptr;
    }

    FileHandle handle;
    FileDevice* device;
    u8* src;

    if (rLoadArg.device != nullptr)
    {
        device = rLoadArg.device->tryOpen(&handle, rLoadArg.path,
                                          FileDevice::cFileOpenFlag_ReadOnly, rLoadArg.div_size);
    }
    else
    {
        device = FileDeviceMgr::instance()->tryOpen(
            &handle, rLoadArg.path, FileDevice::cFileOpenFlag_ReadOnly, rLoadArg.div_size);
    }

    if (device == nullptr)
    {
        return nullptr;
    }

    const u32 fileSize = handle.getFileSize();
    src = mWorkBuffer;

    if (src == nullptr)
    {
        src = new (heap, -FileDevice::cBufferMinAlignment, std::nothrow) u8[mWorkSize * 2];
    }

    if (src == nullptr)
    {
        return nullptr;
    }

    u32 bytesRead = 0;

    if (!handle.tryRead(&bytesRead, src, Mathu::min(mWorkSize, fileSize)))
    {
        if (mWorkBuffer == nullptr)
        {
            delete[] src;
        }

        return nullptr;
    }

    if (bytesRead < 0x10)
    {
        if (mWorkBuffer == nullptr)
        {
            delete[] src;
        }

        return nullptr;
    }

    u32 decompSize = SZSDecompressor::getDecompSize(src);
    s32 decompAlignment = SZSDecompressor::getDecompAlignment(src);

    u32 bufferSize = rLoadArg.load_data_buffer_size;

    if (!(decompSize <= bufferSize || bufferSize == 0))
    {
        decompSize = bufferSize;
    }

    u32 allocSize;
    s32 bufferAlignment = rLoadArg.load_data_buffer_alignment;

    if (bufferAlignment != 0)
    {
        allocSize = (decompSize + bufferAlignment - 1) / bufferAlignment * bufferAlignment;
    }
    else
    {
        allocSize = Mathu::roundUpPow2(decompSize, 0x20);
    }

    u8* dst = rLoadArg.load_data_buffer;
    bool allocated = false;

    if (dst == nullptr)
    {
        s32 alignment;
        DirectResource* directResource = DynamicCast<DirectResource>(pResource);

        if (directResource != nullptr)
        {
            if (rLoadArg.load_data_alignment != 0)
            {
                alignment = Mathi::max(rLoadArg.load_data_alignment, 0x20);
            }
            else
            {
                if (decompAlignment == 0)
                {
                    decompAlignment = directResource->getLoadDataAlignment();
                }

                alignment =
                    (rLoadArg.instance_alignment >= 0 ? 1 : -1) * Mathi::max(decompAlignment, 0x20);
            }
        }
        else
        {
            alignment = (rLoadArg.instance_alignment >= 0 ? 1 : -1) * -0x20;
        }

        dst = new (heap, alignment, std::nothrow) u8[allocSize];

        if (dst == nullptr)
        {
            if (mWorkBuffer == nullptr)
            {
                delete[] src;
            }

            return nullptr;
        }

        allocated = true;
    }

    if (bytesRead < mWorkSize)
    {
        if (SZSDecompressor::decomp(dst, allocSize, src, mWorkSize) < 0)
        {
            if (allocated)
            {
                delete[] dst;
            }

            if (mWorkBuffer == nullptr)
            {
                delete[] src;
            }

            return nullptr;
        }
    }
    else
    {
        mThread.initialize(dst, decompSize, src, fileSize, mWorkSize);

        u8* nextBuffer = src + mWorkSize;
        u32 part = 0;

        while (bytesRead < fileSize)
        {
            mThread.requestDecompPart(part);

            u32 readSize = 0;
            u8* buffer = ((part + 1) & 1) != 0 ? nextBuffer : src;

            if (!handle.tryRead(&readSize, buffer, Mathu::min(mWorkSize, fileSize - bytesRead)))
            {
                if (allocated)
                {
                    delete[] dst;
                }

                if (mWorkBuffer == nullptr)
                {
                    delete[] src;
                }

                return nullptr;
            }

            bytesRead += readSize;
            mThread.waitDecompPart();
            part++;

            if (bytesRead >= fileSize)
            {
                break;
            }

            if (mThread.getError() < 0)
            {
                if (allocated)
                {
                    delete[] dst;
                }

                if (mWorkBuffer == nullptr)
                {
                    delete[] src;
                }

                return nullptr;
            }
        }

        mThread.requestDecompPart(part);
        mThread.waitDecompPart();
    }

    if (mWorkBuffer == nullptr)
    {
        delete[] src;
    }

    if (pOutSize != nullptr)
    {
        *pOutSize = decompSize;
    }

    if (pOutAllocSize != nullptr)
    {
        *pOutAllocSize = allocSize;
    }

    if (pOutAllocated != nullptr)
    {
        *pOutAllocated = allocated;
    }

    return dst;
}

/**
 * Prepares the thread for a new decompression.
 * @param pDst Output buffer.
 * @param dstSize Maximum number of bytes to decompress.
 * @param pSrc Double buffer holding the compressed parts.
 * @param srcSize Size of the compressed file.
 * @param divSize Size of each compressed part.
 */
void ParallelSZSDecompressor::DecompThread::initialize(void* pDst, u32 dstSize, const void* pSrc,
                                                       u32 srcSize, u32 divSize)
{
    mFinishEvent.wait();
    mFinishEvent.resetSignal();
    mContext.initialize(pDst);
    mContext.forceDestCount = dstSize;
    mSrc = static_cast<const u8*>(pSrc);
    mSrcSize = srcSize;
    mDivSize = divSize;
    mError = 0;
}

/**
 * Asks the thread to decompress a part of the source.
 * @param part Index of the part.
 */
void ParallelSZSDecompressor::DecompThread::requestDecompPart(u32 part)
{
    mPartEvent.resetSignal();
    sendMessage(part + 1, MessageQueue::BlockType::Blocking);
}

/**
 * Sets the size of each streamed read.
 * @param divSize Read size, rounded up to 0x20 bytes.
 */
void ParallelSZSDecompressor::setDivSize(u32 divSize)
{
    mWorkSize = Mathu::roundUpPow2(divSize, FileDevice::cBufferMinAlignment);
}

/**
 * Creates the worker thread.
 * @param priority Thread priority.
 * @param pHeap Heap for the thread.
 */
ParallelSZSDecompressor::DecompThread::DecompThread(s32 priority, Heap* pHeap)
    : Thread("DecompThread", pHeap, priority, MessageQueue::BlockType::Blocking, 0x7fffffff, 0x1000,
             0x20)
{
    mPartEvent.initialize(false);
    mFinishEvent.initialize(false);
    mFinishEvent.setSignal();
}

// NON_MATCHING: scheduling
void ParallelSZSDecompressor::DecompThread::calc_(MessageQueue::Element msg)
{
    if (mError < 0)
    {
        return;
    }

    const u32 part = msg - 1;
    const u8* src = (part & 1) == 0 ? mSrc : mSrc + mDivSize;
    const u32 size = Mathu::min(mDivSize, mSrcSize - mDivSize * part);
    const s32 result = SZSDecompressor::streamDecomp(&mContext, src, size);

    if (result == 0)
    {
        mFinishEvent.setSignal();
    }
    else if (result < 0)
    {
        mError = result;
        mFinishEvent.setSignal();
    }

    mPartEvent.setSignal();
}
}  // namespace sead
