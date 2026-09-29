#pragma once

#include <nn/atk/atk_SoundArchive.h>
#include <nn/atk/atk_SoundHeap.h>
#include <nn/util/util_IntrusiveList.h>

namespace nn::atk {
namespace detail {
namespace driver {
class DisposeCallback {
public:
    util::IntrusiveListNode m_DisposeLink;

    virtual ~DisposeCallback() {}
    virtual void InvalidateData(const void* pStart, const void* pEnd) = 0;
};
}  // namespace driver

class SoundArchiveLoader {
public:
    SoundArchiveLoader();
    virtual ~SoundArchiveLoader();

    bool IsAvailable() const;
    bool LoadData(SoundArchive::ItemId id, SoundMemoryAllocatable* pAllocator, u32 loadFlag,
                  size_t loadBlockSize);
    bool LoadData(const char* pItemName, SoundMemoryAllocatable* pAllocator, u32 loadFlag,
                  size_t loadBlockSize);

protected:
    virtual const void* SetFileAddressToTable(SoundArchive::FileId fileId, const void* pAddress) = 0;
    virtual const void* GetFileAddressFromTable(SoundArchive::FileId fileId) const = 0;
    virtual const void* GetFileAddressImpl(SoundArchive::FileId fileId) const = 0;

private:
    u8 _8[0x228 - 0x8];
};
}  // namespace detail

class SoundDataManager : public detail::driver::DisposeCallback, public detail::SoundArchiveLoader {
public:
    SoundDataManager();
    ~SoundDataManager() override;

    size_t GetRequiredMemSize(const SoundArchive* pArchive) const;
    bool Initialize(const SoundArchive* pArchive, void* pBuffer, size_t size);
    void Finalize();

    void InvalidateData(const void* pStart, const void* pEnd) override;

protected:
    const void* SetFileAddressToTable(SoundArchive::FileId fileId, const void* pAddress) override;
    const void* GetFileAddressFromTable(SoundArchive::FileId fileId) const override;
    const void* GetFileAddressImpl(SoundArchive::FileId fileId) const override;
};
static_assert(sizeof(SoundDataManager) == 0x240);
}  // namespace nn::atk
