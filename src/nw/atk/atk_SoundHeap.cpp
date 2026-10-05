#include <nn/atk/atk_SoundHeap.h>
#include <nn/atk/atk_SoundSystem.h>
#include <nn/atk/atk_DriverCommand.h>
#include <nn/atk/atk_StreamBufferPool.h>
#include <nn/atk/atkfnd_ScopedLock.h>

namespace nn::atk {
namespace {
/** @brief Flushes pending driver commands and waits for completion when the driver is initialized. */
inline void WaitForDriverCommands() {
    auto& rDriver = detail::DriverCommand::GetInstance();
    if (rDriver.IsInitialized()) {
        u32 sequence = rDriver.FlushCommand(true);
        rDriver.WaitCommandReply(sequence);
    }
}
} // namespace

/** @brief Creates an empty sound heap with a recursive allocation mutex and no attached memory pool. */
SoundHeap::SoundHeap() : m_CriticalSection(true), m_IsAutoMemoryPoolManagementEnabled(false) {}

/** @brief Destroys the heap before releasing its frame-heap and mutex resources. */
SoundHeap::~SoundHeap() { Destroy(); }

/** @brief Clears sound allocations, detaches an automatically managed pool, and destroys the frame heap. */
void SoundHeap::Destroy() {
    detail::fnd::ScopedMutexLock lock(m_CriticalSection);
    if (m_FrameHeap.IsValid()) {
        Clear();
        if (m_IsAutoMemoryPoolManagementEnabled) {
            SoundSystem::DetachMemoryPool(&m_MemoryPool);
            m_IsAutoMemoryPoolManagementEnabled = false;
        }
        m_FrameHeap.Destroy();
    }
}

/**
 * @brief Creates a sound heap with automatic audio memory-pool management.
 * @param pStartAddress Caller-owned backing storage that must outlive the heap.
 * @param size Available bytes, reduced to complete 4096-byte pages after alignment.
 * @return True if the aligned storage can host a frame heap.
 */
bool SoundHeap::Create(void* pStartAddress, size_t size) { return Create(pStartAddress, size, true); }

/**
 * @brief Creates a sound heap with optional audio memory-pool management.
 * @param pStartAddress Caller-owned backing storage that must outlive the heap.
 * @param size Available bytes in the storage.
 * @param manageMemoryPool Whether to align the storage to 4096-byte pages and attach an audio memory pool.
 * @return True if frame-heap creation succeeds after any required alignment.
 */
bool SoundHeap::Create(void* pStartAddress, size_t size, bool manageMemoryPool) {
    detail::Util::IsValidMemoryForDsp(pStartAddress, size);
    detail::fnd::ScopedMutexLock lock(m_CriticalSection);
    if (manageMemoryPool) {
        uintptr_t start = reinterpret_cast<uintptr_t>(pStartAddress);
        uintptr_t aligned = (start + 4095) & ~uintptr_t(4095);
        size_t padding = aligned - start;
        if (padding > size) {
            return false;
        }
        pStartAddress = reinterpret_cast<void*>(aligned);
        size = (size - padding) & ~size_t(4095);
    }
    bool created = m_FrameHeap.Create(pStartAddress, size);
    if (created && manageMemoryPool) {
        SoundSystem::AttachMemoryPool(&m_MemoryPool, pStartAddress, size);
        m_IsAutoMemoryPoolManagementEnabled = true;
    }
    return created;
}

/** @brief Disposes all frame-heap allocations and waits for queued audio-driver releases. */
void SoundHeap::Clear() {
    if (m_FrameHeap.IsValid()) {
        {
            detail::fnd::ScopedMutexLock lock(m_CriticalSection);
            m_FrameHeap.Clear();
        }
        WaitForDriverCommands();
    }
}

/**
 * @brief Allocates sound memory with driver disposal tracking.
 * @param size Requested payload size in bytes.
 * @return Allocation start, or nullptr if the frame heap cannot satisfy the request.
 */
void* SoundHeap::Allocate(size_t size) { return SoundHeap::Allocate(size, nullptr, nullptr); }

/**
 * @brief Allocates sound memory with driver tracking and a caller disposal callback.
 * @param size Requested payload size in bytes.
 * @param callback Optional callback invoked when this allocation is discarded.
 * @param pCallbackArg Context passed unchanged to callback.
 * @return Allocation start, or nullptr if the frame heap cannot satisfy the request.
 */
void* SoundHeap::Allocate(size_t size, DisposeCallback callback, void* pCallbackArg) {
    detail::fnd::ScopedMutexLock lock(m_CriticalSection);
    return m_FrameHeap.Alloc(size, DisposeCallbackFunc, nullptr, callback, pCallbackArg);
}

/**
 * @brief Queues a driver notification for discarded heap memory.
 * @param pMemory Start of the allocation being discarded.
 * @param size Number of discarded payload bytes.
 * @param pArg Unused block-disposal context; sound-heap allocations supply nullptr.
 */
void SoundHeap::DisposeCallbackFunc(void* pMemory, size_t size, void* pArg) {
    auto& rDriver = detail::DriverCommand::GetInstance();
    if (rDriver.IsInitialized()) {
        auto* pCommand = static_cast<detail::ReleaseHeapMemoryCommand*>(
            rDriver.AllocMemory(sizeof(detail::ReleaseHeapMemoryCommand), true));
        pCommand->type = 0x42;
        pCommand->pMemory = pMemory;
        pCommand->size = size;
        rDriver.PushCommand(pCommand);
    }
}

/**
 * @brief Reports the requested payload size without memory-pool overhead.
 * @param size Requested payload byte count.
 * @param needMemoryPool Unused in this implementation.
 * @return The requested size unchanged.
 */
size_t SoundHeap::GetAllocateSize(size_t size, bool needMemoryPool) { return size; }

/** @brief Saves the current frame-heap allocation state. @return New saved-state level, or failure from the
 * frame heap. */
s32 SoundHeap::SaveState() {
    detail::fnd::ScopedMutexLock lock(m_CriticalSection);
    return m_FrameHeap.SaveState();
}

/**
 * @brief Restores a saved allocation state and waits for discarded audio data to be released.
 * @param level Previously saved frame-heap level to restore.
 */
void SoundHeap::LoadState(s32 level) {
    detail::fnd::ScopedMutexLock lock(m_CriticalSection);
    m_FrameHeap.LoadState(level);
    WaitForDriverCommands();
}
} // namespace nn::atk
