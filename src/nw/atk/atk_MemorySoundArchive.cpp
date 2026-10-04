#include <nn/atk/atk_SoundArchive.h>
#include <nn/atk/atk_GroupFileReader.h>
#include <nn/atk/atk_MemoryFileStream.h>
#include <new>

namespace nn::atk {
/** @brief Constructs an archive with no attached memory image. */
MemorySoundArchive::MemorySoundArchive() : mArchiveData(nullptr) {}

/** @brief Destroys the archive without releasing its caller-owned image. */
MemorySoundArchive::~MemorySoundArchive() = default;

/**
 * @brief Attaches a complete memory-resident sound archive and its metadata blocks.
 * @param pSoundArchiveData Non-null pointer to the archive image, which must remain valid until Finalize.
 * @return True after the archive and its available string block have been attached.
 */
bool MemorySoundArchive::Initialize(const void* pSoundArchiveData) {
    mArchiveReader.Initialize(pSoundArchiveData);
    SoundArchive::Initialize(&mArchiveReader);
    auto* pData = static_cast<const u8*>(pSoundArchiveData);
    mArchiveReader.SetInfoBlock(pData + mArchiveReader.GetInfoBlockOffset());
    int stringOffset = mArchiveReader.GetStringBlockOffset();
    if (mArchiveReader.GetStringBlockSize() != 0xffffffff) {
        mArchiveReader.SetStringBlock(pData + stringOffset);
    }
    mArchiveData = pData;
    return true;
}

/** @brief Detaches the archive image and clears its metadata reader. */
void MemorySoundArchive::Finalize() {
    mArchiveData = nullptr;
    mArchiveReader.Finalize();
    SoundArchive::Finalize();
}

/**
 * @brief Finds a file in the archive's FILE block or in an embedded group.
 * @param fileId Archive file identifier to locate.
 * @return Pointer to the resident file data, or nullptr if no embedded copy is available.
 */
const void* MemorySoundArchive::detail_GetFileAddress(FileId fileId) const {
    FileInfo fileInfo;
    if (detail_ReadFileInfo(fileId, &fileInfo)) {
        u32 offset = fileInfo.offsetFromFileBlockHead;
        if (offset != InvalidId) {
            return GetEmbeddedFileAddress(fileInfo);
        }
    }
    u32 groupCount = GetGroupCount();
    for (u32 groupIndex = 0; groupIndex < groupCount; ++groupIndex) {
        GroupInfo groupInfo;
        if (!ReadGroupInfo(&groupInfo, groupIndex | 0x6000000)) {
            continue;
        }
        FileInfo groupFile;
        if (!detail_ReadFileInfo(groupInfo.fileId, &groupFile)) {
            continue;
        }
        detail::GroupFileReader reader(GetEmbeddedFileAddress(groupFile));
        u32 itemCount = reader.GetGroupItemCount();
        for (u32 itemIndex = 0; itemIndex < itemCount; ++itemIndex) {
            detail::GroupItemLocationInfo item;
            if (reader.ReadGroupItemLocationInfo(&item, itemIndex) && item.fileId == fileId &&
                item.address != nullptr) {
                return item.address;
            }
        }
    }
    return nullptr;
}

/**
 * @brief Constructs a stream over a slice of the attached image.
 * @param pBuffer Aligned caller-owned storage for the stream object.
 * @param size Number of bytes available in pBuffer; must hold a MemoryFileStream.
 * @param begin Byte offset of the stream data within the archive image.
 * @param length Number of readable bytes beginning at that offset.
 * @return Constructed stream, or nullptr if storage is too small or no image is attached.
 */
detail::fnd::FileStream* MemorySoundArchive::OpenStream(void* pBuffer, size_t size, s64 begin,
                                                        size_t length) const {
    if (size < sizeof(detail::MemoryFileStream) || mArchiveData == nullptr) {
        return nullptr;
    }
    auto* pStream = static_cast<detail::MemoryFileStream*>(pBuffer);
    new (pStream) detail::MemoryFileStream(mArchiveData + begin, length);
    return pStream;
}

/**
 * @brief Rejects external file streams for a memory-resident archive.
 * @param pBuffer Unused stream object storage.
 * @param size Unused storage size.
 * @param pExtFilePath Unused external path.
 * @param pCacheBuffer Unused cache storage.
 * @param cacheSize Unused cache size.
 * @return nullptr because this archive only opens embedded files.
 */
detail::fnd::FileStream* MemorySoundArchive::OpenExtStream(void* pBuffer, size_t size,
                                                           const char* pExtFilePath, void* pCacheBuffer,
                                                           size_t cacheSize) const {
    return nullptr;
}

/**
 * @brief Reports the object storage required to open a memory stream.
 * @return Size of a MemoryFileStream in bytes.
 */
size_t MemorySoundArchive::detail_GetRequiredStreamBufferSize() const {
    return sizeof(detail::MemoryFileStream);
}
} // namespace nn::atk
