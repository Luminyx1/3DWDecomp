#include "common/aglDisplayList.h"

#include <cstring>
#include <gfx/nin/seadGraphicsNvn.h>
#include <nvn/nvn_FuncPtrInline.h>
#include <thread/seadThread.h>
#include <time/seadTickSpan.h>

#include "driver/aglNVNMgr.h"

namespace agl {

/**
 * Constructs an empty display list that uses its internal control memory.
 */
DisplayList::DisplayList() : mFlags(0)
{
    mControlMemorySize = cDefaultControlMemorySize;
    mControlMemoryUsed = 0;
    mName = "untitled";
    mControlMemory = getDefaultControlMemory_();
    mNvnCommandBuffer = nullptr;
    clear();
}

/**
 * Sets the control memory used while recording.
 * @param pMemory control memory, or nullptr to use the internal control memory
 * @param size size of the control memory in bytes
 */
void DisplayList::setControlMemory(void* pMemory, u64 size)
{
    if (pMemory)
    {
        mFlags.set(cFlag_UserControlMemory);
    }
    else
    {
        mFlags.reset(cFlag_UserControlMemory);
        pMemory = getDefaultControlMemory_();
        size = cDefaultControlMemorySize;
    }
    mControlMemory = pMemory;
    mControlMemorySize = size;
    mControlMemoryUsed = 0;
}

/**
 * Removes the command buffer memory and the recorded commands.
 */
void DisplayList::clear()
{
    GPUMemAddr<u8> addr;
    setBuffer(addr, 0);
}

/**
 * Clears the display list.
 */
DisplayList::~DisplayList()
{
    clear();
}

/**
 * Sets the command buffer memory.
 * @param buffer command buffer memory
 * @param size size of the command buffer memory in bytes
 */
void DisplayList::setBuffer(GPUMemAddr<u8> buffer, u64 size)
{
    mBufferSize = buffer.isValid() ? size : 0;
    mBuffer = buffer;
    mFlags.reset(cFlag_Recorded);
    setValidSize_(0);
}

/**
 * Sets the size of the recorded commands.
 * @param size size of the recorded commands in bytes
 */
void DisplayList::setValidSize_(u64 size)
{
    mFlags.change(cFlag_Valid, size != 0 || mControlMemoryUsed != 0);
    mValidSize = size;
}

/**
 * Copies the command buffer memory and the recorded commands to another display list.
 * @param pOther destination display list
 */
void DisplayList::copyTo(DisplayList* pOther) const
{
    pOther->mBuffer = mBuffer;
    pOther->mBufferSize = mBufferSize;
    pOther->mFlags.change(cFlag_Valid, mFlags.isOn(cFlag_Valid));
    pOther->mFlags.change(cFlag_Recorded, mFlags.isOn(cFlag_Recorded));
    pOther->mValidSize = mValidSize;
    pOther->mHandle = mHandle;
}

/**
 * Copies the recorded command handle to an executable display list.
 * @param pOther destination executable display list
 */
void DisplayList::copyTo(ExecuteDisplayList* pOther) const
{
    if (!mFlags.isOn(cFlag_Valid))
    {
        pOther->mFlags.makeAllZero();
        pOther->mHandle = 0;
        return;
    }

    pOther->mFlags.set(ExecuteDisplayList::cFlag_Recorded);
    pOther->mFlags.change(ExecuteDisplayList::cFlag_Valid, mFlags.isOn(cFlag_Recorded));
    pOther->mHandle = mHandle;
}

/**
 * Starts recording commands into the command buffer memory.
 * @return true
 */
bool DisplayList::beginDisplayList()
{
    setValidSize_(0);

    nvnCommandBufferInitialize(mNvnCommandBuffer, driver::NVNMgr::instance()->getNvnDevice());
    nvnCommandBufferSetMemoryCallback(mNvnCommandBuffer, outOfMemoryCallback_);
    nvnCommandBufferSetMemoryCallbackData(mNvnCommandBuffer, this);
    nvnCommandBufferAddControlMemory(mNvnCommandBuffer, mControlMemory, mControlMemorySize);
    nvnCommandBufferAddCommandMemory(
        mNvnCommandBuffer, mBuffer.getMemoryPool()->getDriverPool(), mBuffer.getByteOffset(),
        mBufferSize > cDefaultControlMemorySize ? mBufferSize : cDefaultControlMemorySize);
    nvnCommandBufferBeginRecording(mNvnCommandBuffer);
    mFlags.set(cFlag_Recording);
    return true;
}

/**
 * Called by NVN when the command buffer runs out of memory.
 * @param pCommandBuffer command buffer
 * @param event kind of memory that ran out
 * @param minSize minimum required size
 * @param pUserData display list
 */
void DisplayList::outOfMemoryCallback_(NVNcommandBuffer* pCommandBuffer,
                                       NVNcommandBufferMemoryEvent event, u64 minSize,
                                       void* pUserData)
{
}

/**
 * Stops recording and stores the recorded command handle.
 * @return size of the recorded commands in bytes
 */
u32 DisplayList::endDisplayList()
{
    mHandle = nvnCommandBufferEndRecording(mNvnCommandBuffer);
    mControlMemoryUsed = nvnCommandBufferGetControlMemoryUsed(mNvnCommandBuffer);
    setValidSize_(nvnCommandBufferGetCommandMemoryUsed(mNvnCommandBuffer));
    nvnCommandBufferFinalize(mNvnCommandBuffer);
    mNvnCommandBuffer = nullptr;
    mFlags.reset(cFlag_Recording);
    mFlags.set(cFlag_Recorded);
    return mValidSize;
}

/**
 * Sets the command buffer memory and starts recording into it.
 * @param buffer command buffer memory
 * @param size size of the command buffer memory in bytes
 * @param invalidateCPUCache whether to invalidate the CPU cache of the memory first
 * @return true
 */
bool DisplayList::beginDisplayListBuffer(GPUMemAddr<u8> buffer, u64 size,
                                         bool invalidateCPUCache)
{
    if (invalidateCPUCache)
    {
        buffer.invalidateCPUCache(size);
    }
    setBuffer(buffer, size);
    return beginDisplayList();
}

/**
 * Stops recording and copies the recorded commands into a newly allocated buffer.
 * @param pHeap heap to allocate the buffer from
 */
void DisplayList::endDisplayListBuffer(sead::Heap* pHeap)
{
    GPUMemAddr<u8> addr;
    u32 size = endDisplayList();
    setValidSize_(size);
    if (size != 0)
    {
        auto* pBlock = new (pHeap, 8) GPUMemBlock<u8>;
        pBlock->allocBuffer_(size, pHeap, 4, MemoryAttribute::_00);
        addr = GPUMemAddr<u8>(*pBlock, 0);
        std::memcpy(addr.getPtr(), getBuffer().getPtr(), mValidSize);
        addr.flushCPUCache(mValidSize);
        invalidateCPUCache();
    }

    u32 validSize = mValidSize;
    setBuffer(addr, validSize);
    setValidSize_(validSize);
}

/**
 * Does nothing.
 */
void DisplayList::adjustValidSize() {}

/**
 * Invalidates the CPU cache of the recorded commands.
 */
void DisplayList::invalidateCPUCache() const
{
    if (mValidSize == 0)
    {
        return;
    }

    uintptr_t end =
        (reinterpret_cast<uintptr_t>(GPUMemAddr<u8>(mBuffer, mValidSize).getPtr()) + 0x3f) &
        ~uintptr_t(0x3f);
    u64 size = end - reinterpret_cast<uintptr_t>(getBuffer().getPtr());
    getBuffer().invalidateCPUCache(size);
}

/**
 * Submits the recorded commands to the queue and flushes it.
 * @param pDrawContext unused
 */
void DisplayList::callDirect(DrawContext* pDrawContext) const
{
    NVNqueue* pQueue = driver::NVNMgr::instance()->getNvnQueue();
    sead::CriticalSection* pCS = sead::GraphicsNvn::instance()->getCriticalSection1();
    pCS->lock();
    nvnQueueSubmitCommands(pQueue, 1, &mHandle);
    nvnQueueFlush(pQueue);
    pCS->unlock();
}

/**
 * Dumps the recorded commands (debug output is compiled out).
 */
void DisplayList::dump() const
{
    if (!mBuffer.isValid())
    {
        return;
    }

    mBuffer.getPtr();
    for (u32 i = 0; mValidSize / 4 + 4 > i; i++)
    {
        sead::Thread::sleep(sead::TickSpan::makeFromMilliSeconds(2));
    }
}

/**
 * Suspends recording (unsupported).
 * @param ppMemory receives nullptr
 * @return 0
 */
u64 DisplayList::suspend(void** ppMemory)
{
    *ppMemory = nullptr;
    return 0;
}

/**
 * Resumes recording (unsupported).
 * @param pMemory unused
 * @param size unused
 */
void DisplayList::resume(void* pMemory, u64 size) {}

/**
 * Calculates the remaining command memory (unsupported).
 * @return 0
 */
u64 DisplayList::calcRemainingSize()
{
    return 0;
}

/**
 * Gets the size of the command buffer memory.
 * @return size of the command buffer memory in bytes
 */
u32 DisplayList::calcUsedSize() const
{
    return mBufferSize;
}

/**
 * Does nothing.
 * @param pContext unused
 */
void DisplayList::genMessage(sead::hostio::Context* pContext) {}

}  // namespace agl
