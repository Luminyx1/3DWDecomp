#pragma once

#include <nn/audio.h>
#include <nn/os.h>

namespace nn::atk {
class SoundArchive;
class SoundDataManager;

class SoundMemoryAllocatable {
public:
    /** @brief Releases an allocation using the caller-provided context.
     * @param pArg Context passed to Allocate as pCallbackArg.
     */
    typedef void (*DisposeCallback)(void* pArg);

    /** @brief Destroys the allocation interface without releasing caller-owned memory. */
    virtual ~SoundMemoryAllocatable() {}
    virtual void* Allocate(size_t size) = 0;
    virtual void* Allocate(size_t size, DisposeCallback callback, void* pCallbackArg) = 0;
    virtual size_t GetAllocateSize(size_t size, bool needMemoryPool) = 0;
};

namespace detail {
class FrameHeap {
public:
    using BlockDisposeCallback = void (*)(void* pMemory, size_t size, void* pArg);
    FrameHeap();
    ~FrameHeap();
    bool Create(void* pMemory, size_t size);
    void Destroy();
    void Clear();
    void* Alloc(size_t size, BlockDisposeCallback blockCallback, void* pBlockArg,
                SoundMemoryAllocatable::DisposeCallback callback, void* pCallbackArg);
    s32 SaveState();
    void LoadState(s32 level);
    /** @brief Checks whether a frame heap has been created. @return True when the heap handle is non-null. */
    bool IsValid() const { return m_pHeap != nullptr; }
    void Dump(const SoundDataManager& rMgr, const SoundArchive& rArchive) const;
    s32 GetCurrentLevel() const;
    size_t GetSize() const;
    size_t GetFreeSize() const;

private:
    void* m_pHeap;
    u8 _8[0x18 - 0x8];
};
}  // namespace detail

class SoundHeap : public SoundMemoryAllocatable {
public:
    SoundHeap();
    ~SoundHeap() override;

    bool Create(void* pStartAddress, size_t size);
    bool Create(void* pStartAddress, size_t size, bool manageMemoryPool);
    void Destroy();
    void Clear();

    void* Allocate(size_t size) override;
    void* Allocate(size_t size, DisposeCallback callback, void* pCallbackArg) override;
    size_t GetAllocateSize(size_t size, bool needMemoryPool) override;

    /** @brief Checks whether backing memory is attached. @return True when the frame heap is valid. */
    bool IsValid() const { return m_FrameHeap.IsValid(); }

    s32 SaveState();
    void LoadState(s32 level);

    /** @brief Reads the saved-state depth under the heap mutex. @return Current frame-heap level. */
    s32 GetCurrentLevel() const {
        m_CriticalSection.Lock();
        s32 level = m_FrameHeap.GetCurrentLevel();
        m_CriticalSection.Unlock();
        return level;
    }

    /** @brief Reads the backing heap size under its mutex. @return Heap size in bytes. */
    size_t GetSize() const {
        m_CriticalSection.Lock();
        size_t size = m_FrameHeap.GetSize();
        m_CriticalSection.Unlock();
        return size;
    }

    /** @brief Reads the remaining allocation capacity under the mutex. @return Free bytes. */
    size_t GetFreeSize() const {
        m_CriticalSection.Lock();
        size_t size = m_FrameHeap.GetFreeSize();
        m_CriticalSection.Unlock();
        return size;
    }

    /**
     * @brief Dumps heap allocations with their archive resource identities.
     * @param rMgr Resource manager used to identify heap allocations.
     * @param rArchive Archive containing the resource metadata.
     */
    void Dump(const SoundDataManager& rMgr, const SoundArchive& rArchive) const {
        m_CriticalSection.Lock();
        m_FrameHeap.Dump(rMgr, rArchive);
        m_CriticalSection.Unlock();
    }

private:
    static void DisposeCallbackFunc(void* pMemory, size_t size, void* pArg);
    mutable os::Mutex m_CriticalSection;
    detail::FrameHeap m_FrameHeap;
    audio::MemoryPoolType m_MemoryPool;
    bool m_IsAutoMemoryPoolManagementEnabled;
};
static_assert(sizeof(SoundHeap) == 0x50);
}  // namespace nn::atk
