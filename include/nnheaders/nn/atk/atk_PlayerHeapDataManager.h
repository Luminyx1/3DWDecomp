#pragma once
#include <nn/atk/atk_SoundDataManager.h>

namespace nn::atk::detail {
class PlayerHeapDataManager : public driver::DisposeCallback, public SoundArchiveLoader {
  public:
    PlayerHeapDataManager();
    ~PlayerHeapDataManager() override;
    void Initialize(const SoundArchive* pArchive);
    void Finalize();
    const void* SetFileAddress(SoundArchive::FileId id, const void* pAddress);
    const void* GetFileAddress(SoundArchive::FileId id) const;
    void InvalidateData(const void* pStart, const void* pEnd) override;

  protected:
    const void* SetFileAddressToTable(SoundArchive::FileId id, const void* pAddress) override;
    const void* GetFileAddressFromTable(SoundArchive::FileId id) const override;
    const void* GetFileAddressImpl(SoundArchive::FileId id) const override;

  private:
    struct FileEntry {
        SoundArchive::FileId id;
        const void* pAddress;
    };
    FileEntry mFiles[9];
    bool mInitialized;
    bool mFinalized;
};
static_assert(sizeof(PlayerHeapDataManager) == 0x2c8, "PlayerHeapDataManager size");
} // namespace nn::atk::detail
