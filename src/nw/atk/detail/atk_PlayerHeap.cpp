#include <nn/atk/atk_PlayerHeap.h>
#include <nn/atk/atk_DriverCommand.h>
#include <nn/atk/atk_StreamBufferPool.h>
#include <new>

namespace nn::atk::detail {
namespace {
/**
 * @brief Round a heap cursor upward to the next audio-memory page boundary.
 * @param pAddress Cursor to align; must allow rounding without address overflow.
 * @return Address aligned upward to 4096 bytes.
 */
inline u8* AlignHeapPage(u8* pAddress) {
    return reinterpret_cast<u8*>((reinterpret_cast<uintptr_t>(pAddress) + 4095) & ~uintptr_t(4095));
}
} // namespace

/** @brief Construct an unattached heap with no owner or disposal callbacks. */
PlayerHeap::PlayerHeap()
    : mOwner(nullptr), mStart(nullptr), mEnd(nullptr), mCurrent(nullptr), mInUse(false) {}

/** @brief Clear outstanding allocations and mark this heap as unused. */
PlayerHeap::~PlayerHeap() {
    Destroy();
    mInUse = false;
}

/**
 * @brief Attach a caller-owned buffer and align its allocation start to an audio page.
 * @param pMemory Writable audio-memory buffer.
 * @param size Available byte count; must reach the first 4096-byte boundary in the buffer.
 * @return Whether the aligned buffer can be used as a heap.
 */
bool PlayerHeap::Create(void* pMemory, size_t size) {
    Util::IsValidMemoryForDsp(pMemory, size);
    u8* pEnd = static_cast<u8*>(pMemory) + size;
    u8* pStart = AlignHeapPage(static_cast<u8*>(pMemory));
    if (pEnd < pStart) {
        return false;
    }
    mStart = pStart;
    mEnd = pEnd;
    mCurrent = pStart;
    return true;
}

/** @brief Clear allocations and invalidate the allocation cursor. */
void PlayerHeap::Destroy() {
    Clear();
    mCurrent = nullptr;
}

/** @brief Notify the audio driver, reset the cursor, and invoke all disposal callbacks. */
void PlayerHeap::Clear() {
    DriverCommand& rDriver = DriverCommand::GetInstanceForTaskThread();
    auto* pCommand =
        static_cast<ReleaseHeapMemoryCommand*>(rDriver.AllocMemory(sizeof(ReleaseHeapMemoryCommand), false));
    pCommand->type = 0x42;
    pCommand->pMemory = mStart;
    u8* pStart = mStart;
    pCommand->size = mCurrent - pStart;
    rDriver.PushCommand(pCommand);
    rDriver.FlushCommand(false, false);
    mCurrent = mStart;
    for (auto& rRecord : mCallbacks) {
        if (rRecord.callback != nullptr) {
            rRecord.callback(rRecord.pArg);
        }
    }
    mCallbacks.clear();
}

/**
 * @brief Reserve bytes and advance the next allocation to an audio page boundary.
 * @param size Requested byte count; zero leaves an already aligned cursor unchanged.
 * @return Allocation start, or null when the requested bytes exceed the buffer.
 */
void* PlayerHeap::Allocate(size_t size) {
    u8* pBlock = mCurrent;
    u8* pEnd = pBlock + size;
    if (pEnd > mEnd) {
        return nullptr;
    }
    mCurrent = AlignHeapPage(pEnd);
    return pBlock;
}

/**
 * @brief Reserve bytes together with a disposal callback record.
 * @param size Requested payload byte count; callback storage is added after the payload.
 * @param callback Callback invoked when the heap is cleared; null is allowed.
 * @param pCallbackArg Context passed unchanged to callback.
 * @return Allocation start, or null when payload and callback storage do not fit.
 */
void* PlayerHeap::Allocate(size_t size, DisposeCallback callback, void* pCallbackArg) {
    u8* pBlock = mCurrent;
    u8* pRecordMemory = pBlock + size;
    u8* pEnd = pRecordMemory + sizeof(CallbackRecord);
    if (pEnd > mEnd) {
        return nullptr;
    }
    mCurrent = AlignHeapPage(pEnd);
    auto* pRecord = new (pRecordMemory) CallbackRecord(callback, pCallbackArg);
    mCallbacks.push_back(*pRecord);
    return pBlock;
}

/**
 * @brief Report the requested payload size without adding memory-pool overhead.
 * @param size Requested allocation size in bytes.
 * @param needMemoryPool Unused for player heaps.
 * @return The supplied size unchanged.
 */
size_t PlayerHeap::GetAllocateSize(size_t size, bool needMemoryPool) { return size; }

/** @brief Calculate bytes after the allocation cursor. @return Remaining buffer size. */
size_t PlayerHeap::GetFreeSize() const { return mEnd - mCurrent; }
} // namespace nn::atk::detail
