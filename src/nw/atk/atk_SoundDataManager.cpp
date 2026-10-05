#include <nn/atk/atk_SoundDataManager.h>
#include <nn/atk/atk_DriverCommand.h>
#include <nn/atk/atk_GroupFileReader.h>
#include <nn/atk/atk_WaveArchiveFileReader.h>

namespace nn::atk {
namespace {
struct DisposeCallbackCommand : detail::Command {
    detail::driver::DisposeCallback* pCallback;
};

/**
 * @brief Submits a registration change for an audio data-disposal callback.
 * @param rDriver Initialized driver command queue.
 * @param pCallback Callback object to register or unregister.
 * @param type Driver opcode: 0x43 registers and 0x44 unregisters the callback.
 */
inline void PushDisposeCallbackCommand(detail::DriverCommand& rDriver,
                                       detail::driver::DisposeCallback* pCallback, u32 type) {
    auto* pCommand =
        static_cast<DisposeCallbackCommand*>(rDriver.AllocMemory(sizeof(DisposeCallbackCommand), true));
    pCommand->pCallback = pCallback;
    pCommand->type = type;
    rDriver.PushCommand(pCommand);
}

/**
 * @brief Calculates the file-address table storage before final pointer alignment.
 * @param count Number of archive file entries.
 * @return Count field size plus one pointer per file.
 */
inline size_t GetTableBytes(u32 count) { return sizeof(u32) + sizeof(void*) * count; }
} // namespace

/** @brief Creates an archive data manager without a file table or external address provider. */
SoundDataManager::SoundDataManager() : mFileTable(nullptr), mFileAddressProvider(nullptr) {}

/** @brief Destroys the archive loader without freeing caller-owned table storage. */
SoundDataManager::~SoundDataManager() = default;

/**
 * @brief Calculates aligned storage for the archive's file-address table.
 * @param pArchive Non-null archive supplying the file count.
 * @return Required bytes rounded upward to pointer alignment.
 */
size_t SoundDataManager::GetRequiredMemSize(const SoundArchive* pArchive) const {
    return (GetTableBytes(pArchive->detail_GetFileCount()) + 7) & ~size_t(7);
}

/**
 * @brief Creates file tables and registers this manager for driver disposal notifications.
 * @param pArchive Archive that must outlive this manager's use of it.
 * @param pBuffer Pointer-aligned, caller-owned file-table storage.
 * @param size Available storage bytes.
 * @return True if the file table fits and the manager is initialized.
 */
bool SoundDataManager::Initialize(const SoundArchive* pArchive, void* pBuffer, size_t size) {
    void* pCursor = pBuffer;
    if (!CreateTables(&pCursor, pArchive, static_cast<u8*>(pBuffer) + size)) {
        return false;
    }
    SetSoundArchive(pArchive);
    auto& rDriver = detail::DriverCommand::GetInstance();
    PushDisposeCallbackCommand(rDriver, this, 0x43);
    return true;
}

/**
 * @brief Allocates and clears the file table from a caller-owned cursor range.
 * @param ppCursor Non-null pointer to the pointer-aligned allocation cursor; advanced on success.
 * @param pArchive Non-null archive supplying the file count.
 * @param pEnd End of the available storage range.
 * @return True if the table fits; false leaves the cursor and existing table unchanged.
 */
bool SoundDataManager::CreateTables(void** ppCursor, const SoundArchive* pArchive, void* pEnd) {
    size_t size = GetTableBytes(pArchive->detail_GetFileCount());
    auto* pNext = reinterpret_cast<u8*>((reinterpret_cast<uintptr_t>(*ppCursor) + size + 7) & ~uintptr_t(7));
    if (pNext - static_cast<u8*>(pEnd) > 0) {
        return false;
    }
    mFileTable = static_cast<FileTable*>(*ppCursor);
    *ppCursor = pNext;
    mFileTable->count = pArchive->detail_GetFileCount();
    for (size_t i = 0; i < mFileTable->count; ++i) {
        mFileTable->addresses[i] = nullptr;
    }
    return true;
}

/** @brief Unregisters driver disposal notifications, waits for acknowledgement, and clears table references.
 */
void SoundDataManager::Finalize() {
    auto& rDriver = detail::DriverCommand::GetInstance();
    PushDisposeCallbackCommand(rDriver, this, 0x44);
    rDriver.WaitCommandReply(rDriver.FlushCommand(true));
    mFileAddressProvider = nullptr;
    mFileTable = nullptr;
}

/**
 * @brief Invalidates file and individually loaded wave addresses inside a released memory range.
 * @param pStart First address of the inclusive invalidation range.
 * @param pEnd Last address of the inclusive invalidation range.
 */
void SoundDataManager::InvalidateData(const void* pStart, const void* pEnd) {
    if (mFileTable != nullptr) {
        for (size_t i = 0; i < mFileTable->count; ++i) {
            const void* pAddress = mFileTable->addresses[i];
            if (pAddress >= pStart && pAddress <= pEnd) {
                mFileTable->addresses[i] = nullptr;
            }
        }
    }
    const SoundArchive* pArchive = GetSoundArchive();
    if (pArchive != nullptr) {
        u32 count = pArchive->GetWaveArchiveCount();
        for (u32 i = 0; i < count; ++i) {
            SoundArchive::WaveArchiveInfo info;
            bool read = pArchive->ReadWaveArchiveInfo(i | 0x05000000, &info);
            if (info.isLoadIndividual && read) {
                const void* pWaveArchive = GetFileAddressFromTable(info.fileId);
                if (pWaveArchive != nullptr) {
                    detail::WaveArchiveFileReader reader(pWaveArchive, true);
                    for (u32 wave = 0; wave < info.waveCount; ++wave) {
                        const void* pWave = reader.GetWaveFile(wave);
                        if (pWave != nullptr && pWave >= pStart && pWave <= pEnd) {
                            reader.SetWaveFile(wave, nullptr);
                        }
                    }
                }
            }
        }
    }
}

/**
 * @brief Resolves a file through the manager's virtual address lookup.
 * @param fileId Archive file identifier.
 * @return File address, or nullptr when unavailable.
 */
const void* SoundDataManager::detail_GetFileAddress(SoundArchive::FileId fileId) const {
    return GetFileAddressImpl(fileId);
}

/**
 * @brief Resolves a file from the external provider, embedded archive, or loaded-file table.
 * @param fileId Archive file identifier.
 * @return First available address, or nullptr.
 */
const void* SoundDataManager::GetFileAddressImpl(SoundArchive::FileId fileId) const {
    if (mFileAddressProvider != nullptr) {
        const void* pAddress = mFileAddressProvider->GetFileAddress(fileId);
        if (pAddress != nullptr) {
            return pAddress;
        }
    }
    const void* pAddress = GetFileAddressFromSoundArchive(fileId);
    if (pAddress != nullptr) {
        return pAddress;
    }
    return GetFileAddressFromTable(fileId);
}

/**
 * @brief Replaces a loaded-file table entry.
 * @param fileId Valid table index when a table is installed.
 * @param pAddress Replacement address, or nullptr to clear the entry.
 * @return Previous address, or nullptr if no table is installed.
 */
const void* SoundDataManager::SetFileAddressToTable(SoundArchive::FileId fileId, const void* pAddress) {
    if (mFileTable == nullptr) {
        return nullptr;
    }
    const void* pPrevious = mFileTable->addresses[fileId];
    mFileTable->addresses[fileId] = pAddress;
    return pPrevious;
}

/**
 * @brief Looks up a loaded-file table entry.
 * @param fileId Archive file identifier; out-of-range values are rejected.
 * @return Stored address, or nullptr if no valid entry exists.
 */
const void* SoundDataManager::GetFileAddressFromTable(SoundArchive::FileId fileId) const {
    if (mFileTable == nullptr) {
        return nullptr;
    }
    if (fileId >= mFileTable->count) {
        return nullptr;
    }
    return mFileTable->addresses[fileId];
}

/**
 * @brief Installs a file address through the virtual table setter.
 * @param fileId Valid archive file identifier.
 * @param pAddress New file address, or nullptr to clear it.
 * @return Previously registered address.
 */
const void* SoundDataManager::SetFileAddress(SoundArchive::FileId fileId, const void* pAddress) {
    return SetFileAddressToTable(fileId, pAddress);
}

/**
 * @brief Finds the first file-table entry containing an address.
 * @param pAddress Address to locate; the file table must already be initialized.
 * @return First matching file identifier, or InvalidId if no entry matches.
 */
SoundArchive::FileId SoundDataManager::detail_GetFileIdFromTable(const void* pAddress) const {
    for (u32 i = 0; i < mFileTable->count; ++i) {
        if (mFileTable->addresses[i] == pAddress) {
            return i;
        }
    }
    return SoundArchive::InvalidId;
}

/**
 * @brief Registers each embedded file address from a group image.
 * @param pGroupFile Group image to read; nullptr is rejected.
 * @param size Unused group-image size in this implementation.
 * @return True if every group item has a readable embedded address.
 */
bool SoundDataManager::SetFileAddressInGroupFile(const void* pGroupFile, size_t size) {
    if (pGroupFile == nullptr) {
        return false;
    }
    detail::GroupFileReader reader(pGroupFile);
    u32 count = reader.GetGroupItemCount();
    for (u32 i = 0; i < count; ++i) {
        detail::GroupItemLocationInfo info;
        if (!reader.ReadGroupItemLocationInfo(&info, i) || info.address == nullptr) {
            return false;
        }
        SetFileAddressToTable(info.fileId, info.address);
    }
    return true;
}

/**
 * @brief Invalidates file addresses backed by a group image and waits for the driver.
 * @param pGroupFile Start of the discarded group image.
 * @param size Number of discarded bytes.
 */
void SoundDataManager::ClearFileAddressInGroupFile(const void* pGroupFile, size_t size) {
    InvalidateSoundData(pGroupFile, size);
}

/**
 * @brief Submits a memory-disposal notification and waits for the driver to apply it.
 * @param pMemory Start of the discarded memory range.
 * @param size Number of discarded bytes.
 */
void SoundDataManager::InvalidateSoundData(const void* pMemory, size_t size) {
    auto& rDriver = detail::DriverCommand::GetInstance();
    if (rDriver.IsInitialized()) {
        auto* pCommand = static_cast<detail::ReleaseHeapMemoryCommand*>(
            rDriver.AllocMemory(sizeof(detail::ReleaseHeapMemoryCommand), true));
        pCommand->type = 0x42;
        pCommand->pMemory = pMemory;
        pCommand->size = size;
        rDriver.PushCommand(pCommand);
        rDriver.WaitCommandReply(rDriver.FlushCommand(true));
    }
}
} // namespace nn::atk
