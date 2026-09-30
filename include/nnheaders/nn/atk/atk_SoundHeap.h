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

    virtual ~SoundMemoryAllocatable() {}
    virtual void* Allocate(size_t size) = 0;
    virtual void* Allocate(size_t size, DisposeCallback callback, void* pCallbackArg) = 0;
    virtual size_t GetAllocateSize(size_t size, bool needMemoryPool) = 0;
};

namespace detail {
class FrameHeap {
public:
    bool IsValid() const { return m_pHeap != nullptr; }
    void Dump(const SoundDataManager& rMgr, const SoundArchive& rArchive) const;

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
    void Destroy();

    void* Allocate(size_t size) override;
    void* Allocate(size_t size, DisposeCallback callback, void* pCallbackArg) override;
    size_t GetAllocateSize(size_t size, bool needMemoryPool) override;

    bool IsValid() const { return m_FrameHeap.IsValid(); }

    void Dump(const SoundDataManager& rMgr, const SoundArchive& rArchive) const {
        os::LockMutex(&m_CriticalSection);
        m_FrameHeap.Dump(rMgr, rArchive);
        os::UnlockMutex(&m_CriticalSection);
    }

private:
    mutable os::MutexType m_CriticalSection;
    detail::FrameHeap m_FrameHeap;
    audio::MemoryPoolType m_MemoryPool;
    bool m_IsAutoMemoryPoolManagementEnabled;
};
static_assert(sizeof(SoundHeap) == 0x50);
}  // namespace nn::atk
