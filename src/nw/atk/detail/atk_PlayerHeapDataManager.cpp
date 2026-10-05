#include <nn/atk/atk_PlayerHeapDataManager.h>

namespace nn::atk::detail {
/** @brief Constructs an inactive manager without initializing its file table. */
PlayerHeapDataManager::PlayerHeapDataManager() : mInitialized(false), mFinalized(true) {}

/** @brief Detaches the archive before destroying the loader. */
PlayerHeapDataManager::~PlayerHeapDataManager() { Finalize(); }

/**
 * @brief Initializes the nine-entry cache and associates its archive once.
 * @param pArchive Archive supplying file information; retained without ownership.
 */
void PlayerHeapDataManager::Initialize(const SoundArchive* pArchive) {
    if (mInitialized) {
        return;
    }
    mInitialized = true;
    mFinalized = false;
    for (auto& entry : mFiles) {
        entry.id = SoundArchive::InvalidId;
    }
    for (auto& entry : mFiles) {
        entry.pAddress = nullptr;
    }
    SetSoundArchive(pArchive);
}

/** @brief Detaches the archive once and permits subsequent initialization. */
void PlayerHeapDataManager::Finalize() {
    if (mFinalized) {
        return;
    }
    mInitialized = false;
    mFinalized = true;
    SetSoundArchive(nullptr);
}

/**
 * @brief Registers a loaded file through the cache's virtual setter.
 * @param id File identifier to insert or replace.
 * @param pAddress Loaded file address to retain without ownership.
 * @return Previous address, or nullptr for a new entry or a full cache.
 */
const void* PlayerHeapDataManager::SetFileAddress(SoundArchive::FileId id, const void* pAddress) {
    return SetFileAddressToTable(id, pAddress);
}

/**
 * @brief Looks up a file through the cache's virtual getter.
 * @param id File identifier to locate.
 * @return Cached address, or nullptr if absent.
 */
const void* PlayerHeapDataManager::GetFileAddress(SoundArchive::FileId id) const {
    return GetFileAddressFromTable(id);
}

/**
 * @brief Clears every cache entry when player-heap storage is invalidated.
 * @param pStart Unused; invalidation always clears the entire cache.
 * @param pEnd Unused; invalidation always clears the entire cache.
 */
void PlayerHeapDataManager::InvalidateData(const void* pStart, const void* pEnd) {
    for (auto& entry : mFiles) {
        entry.id = SoundArchive::InvalidId;
        entry.pAddress = nullptr;
    }
}

/**
 * @brief Replaces a cached address or inserts it into the first free entry.
 * @param id File identifier to insert or replace.
 * @param pAddress Loaded file address, retained without ownership.
 * @return Previous address, or nullptr for insertion or a full cache.
 */
const void* PlayerHeapDataManager::SetFileAddressToTable(SoundArchive::FileId id, const void* pAddress) {
    for (int i = 0; i < 9; ++i) {
        if (mFiles[i].id == id) {
            const void* previous = mFiles[i].pAddress;
            mFiles[i].pAddress = pAddress;
            return previous;
        }
    }
    for (int i = 0; i < 9; ++i) {
        if (mFiles[i].id == SoundArchive::InvalidId) {
            mFiles[i].id = id;
            mFiles[i].pAddress = pAddress;
            return nullptr;
        }
    }
    return nullptr;
}

/**
 * @brief Searches the cache for a file identifier.
 * @param id File identifier to locate.
 * @return Cached address, or nullptr if absent.
 */
const void* PlayerHeapDataManager::GetFileAddressFromTable(SoundArchive::FileId id) const {
    for (int i = 0; i < 9; ++i) {
        if (mFiles[i].id == id) {
            return mFiles[i].pAddress;
        }
    }
    return nullptr;
}

/**
 * @brief Resolves a loader request exclusively through the player-heap cache.
 * @param id File identifier to locate.
 * @return Cached address, or nullptr if absent.
 */
const void* PlayerHeapDataManager::GetFileAddressImpl(SoundArchive::FileId id) const {
    return GetFileAddressFromTable(id);
}
} // namespace nn::atk::detail
