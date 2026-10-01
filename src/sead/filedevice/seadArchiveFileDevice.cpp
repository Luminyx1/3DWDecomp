#include "filedevice/seadArchiveFileDevice.h"
#include "basis/seadRawPrint.h"
#include "math/seadMathCalcCommon.h"
#include "prim/seadPtrUtil.h"
#include "resource/seadArchiveRes.h"

namespace sead
{
struct ArchiveFileDevice::ArchiveFileHandle
{
    const u8* mFileData;
    ArchiveRes::FileInfo mFileInfo;
    u32 mPos;
};

/**
 * Loads an archive file by entry ID if the device has permission.
 * @param id the entry ID
 * @param rArg the load settings
 * @return the loaded data, or null on failure
 */
u8* ArchiveFileDevice::tryLoadWithEntryID(s32 id, FileDevice::LoadArg& rArg)
{
    SEAD_ASSERT_MSG(mPermission, "Device permission error.");

    if (!mPermission)
    {
        return nullptr;
    }

    return doLoadWithEntryID_(id, rArg);
}

/**
 * Opens an archive file by entry ID if the device has permission.
 * @param pHandle the handle to open
 * @param id the entry ID
 * @param flag the open mode
 * @param divSize the read division size
 * @return the device that opened the file, or null on failure
 */
FileDevice* ArchiveFileDevice::tryOpenWithEntryID(FileHandle* pHandle, s32 id,
                                                  FileDevice::FileOpenFlag flag, u32 divSize)
{
    SEAD_ASSERT_MSG(mPermission, "Device permission error.");

    if (!mPermission)
    {
        return nullptr;
    }

    setFileHandleDivSize_(pHandle, divSize);
    FileDevice* ret = doOpenWithEntryID_(pHandle, id, flag);
    setHandleBaseFileDevice_(pHandle, ret);
    return ret;
}

/**
 * Converts a path to an archive entry ID.
 * @param rPath the file path
 * @return the entry ID
 */
s32 ArchiveFileDevice::tryConvertPathToEntryID(const SafeString& rPath)
{
    return doConvertPathToEntryID_(rPath);
}

/**
 * Sets the archive's current directory if the device has permission.
 * @param rDir the directory path
 * @return true on success
 */
bool ArchiveFileDevice::setCurrentDirectory(const SafeString& rDir)
{
    SEAD_ASSERT_MSG(mPermission, "Device permission error.");

    if (!mPermission)
    {
        return false;
    }

    return doSetCurrentDirectory_(rDir);
}

/**
 * Gets the size of the archive file at a path.
 * @param pFileSize receives the file size in bytes
 * @param rPath the file path
 * @return true on success
 */
bool ArchiveFileDevice::doGetFileSize_(u32* pFileSize, const SafeString& rPath)
{
    if (mArchive == nullptr)
    {
        SEAD_ASSERT_MSG(false, "no archive mounted");
        return false;
    }

    if (rPath.cstr() == nullptr)
    {
        SEAD_ASSERT_MSG(false, "invalid rPath");
        return false;
    }

    ArchiveRes::FileInfo info{};

    if (mArchive->getFile(rPath, &info) == nullptr)
    {
        return false;
    }

    *pFileSize = info.mLength;
    return true;
}

/**
 * Gets the size of an open archive file.
 * @param pFileSize receives the file size in bytes
 * @param pHandle the file handle
 * @return true on success
 */
bool ArchiveFileDevice::doGetFileSize_(u32* pFileSize, FileHandle* pHandle)
{
    if (pHandle == nullptr)
    {
        SEAD_ASSERT_MSG(false, "invalid pHandle");
        return false;
    }

    if (mArchive == nullptr)
    {
        SEAD_ASSERT_MSG(false, "no archive mounted");
        return false;
    }

    *pFileSize = getArchiveFileHandle_(pHandle)->mFileInfo.mLength;
    return true;
}

/**
 * Gets the archive handle data stored in a file handle.
 * @param pHandle the file handle
 * @return the archive file handle
 */
ArchiveFileDevice::ArchiveFileHandle* ArchiveFileDevice::getArchiveFileHandle_(FileHandle* pHandle)
{
    return reinterpret_cast<ArchiveFileHandle*>(getHandleBaseHandleBuffer_(pHandle).getBufferPtr());
}

/**
 * Constructs archive handle data in a file handle's buffer.
 * @param pHandle the file handle
 * @return the constructed archive file handle
 */
ArchiveFileDevice::ArchiveFileHandle*
ArchiveFileDevice::constructArchiveFileHandle_(FileHandle* pHandle) const
{
    return new (getHandleBaseHandleBuffer_(pHandle).getBufferPtr()) ArchiveFileHandle;
}

/**
 * Checks whether a file exists in the archive.
 * @param pExists receives whether the file exists
 * @param rPath the file path
 * @return true on success
 */
bool ArchiveFileDevice::doIsExistFile_(bool* pExists, const SafeString& rPath)
{
    if (mArchive == nullptr)
    {
        SEAD_ASSERT_MSG(false, "no archive mounted");
        return false;
    }

    if (rPath.cstr() == nullptr)
    {
        SEAD_ASSERT_MSG(false, "invalid rPath");
        return false;
    }

#if SEAD_ARCHIVERES_ISEXISTFILEIMPL
    *pExists = mArchive->isExistFile(rPath);
#else
    *pExists = mArchive->getFile(rPath) != nullptr;
#endif
    return true;
}

/**
 * Reports that no directory exists, since archives do not support this check.
 * @param pExists receives false
 * @param rPath the directory path
 * @return true on success
 */
bool ArchiveFileDevice::doIsExistDirectory_(bool* pExists, const SafeString& rPath)
{
    if (mArchive == nullptr)
    {
        SEAD_ASSERT_MSG(false, "no archive mounted");
        return false;
    }

    if (rPath.cstr() == nullptr)
    {
        SEAD_ASSERT_MSG(false, "invalid rPath");
        return false;
    }

    *pExists = false;
    return true;
}

/**
 * Loads an archive file by entry ID, copying it into a buffer when one or a heap is given, otherwise returning the archive data directly.
 * @param entryId the entry ID
 * @param rArg the load settings, updated with the read and rounded-up sizes
 * @return the loaded data, or null on failure
 */
u8* ArchiveFileDevice::doLoadWithEntryID_(s32 entryId, LoadArg& rArg)
{
    if (entryId == -1)
    {
        SEAD_ASSERT_MSG(false, "Invalid entryId");
        return nullptr;
    }

    if (rArg.buffer_size_alignment % 32 != 0)
    {
        SEAD_ASSERT_MSG(false, "rArg.buffer_size_alignment[%u] is not multipe of 32",
                        rArg.buffer_size_alignment);
        return nullptr;
    }

    if (rArg.buffer || rArg.heap != nullptr)
    {
        FileHandle handle;

        if (tryOpenWithEntryID(&handle, entryId, {}, rArg.div_size) == nullptr)
        {
            return nullptr;
        }

        // Determine the buffer size.
        u32 buffer_size = rArg.buffer_size;

        if (buffer_size == 0)
        {
            u32 file_size = 0;

            if (!tryGetFileSize(&file_size, &handle))
            {
                return nullptr;
            }

            SEAD_ASSERT(file_size != 0);

            if (rArg.buffer_size_alignment)
            {
                buffer_size = Mathu::roundUp(file_size, rArg.buffer_size_alignment);
            }
            else
            {
                buffer_size = Mathi::roundUpPow2(file_size, cBufferMinAlignment);
            }
        }

        // Allocate the buffer if need be.
        u8* buffer = rArg.buffer;
        bool buffer_allocated = false;

        if (!buffer)
        {
            const s32 aligment_sign = Mathi::sign(rArg.alignment);
            const s32 alignment = std::max(Mathi::abs(rArg.alignment), 32);
            buffer = new (rArg.heap, alignment * aligment_sign) u8[buffer_size];
            buffer_allocated = true;
        }

        u32 bytes_read = 0;

        if (!tryRead(&bytes_read, &handle, buffer, buffer_size) || !tryClose(&handle))
        {
            // Clean up the allocation on failure.
            if (buffer && buffer_allocated)
            {
                delete[] buffer;
            }

            return nullptr;
        }

        rArg.read_size = bytes_read;
        rArg.need_unload = buffer_allocated;
        rArg.roundup_size = buffer_size;
        return buffer;
    }

    ArchiveRes::FileInfo info{};
    auto* ret = mArchive->getFileFast(entryId, &info);

    if (ret == nullptr)
    {
        return nullptr;
    }

    SEAD_ASSERT(rArg.alignment == 0 || PtrUtil::isAligned(ret, Mathi::abs(rArg.alignment)));

    if (rArg.buffer_size_alignment && info.mLength % rArg.buffer_size_alignment != 0)
    {
        SEAD_WARN("archive file size[%u] is not multipe of rArg.buffer_size_alignment[%u]",
                  info.mLength, rArg.buffer_size_alignment);
        return nullptr;
    }

    rArg.read_size = info.mLength;
    rArg.roundup_size = info.mLength;
    rArg.need_unload = false;
    return const_cast<u8*>(static_cast<const u8*>(ret));
}

/**
 * Loads an archive file by path, copying it into a buffer when one or a heap is given, otherwise returning the archive data directly.
 * @param rArg the load settings, updated with the read and rounded-up sizes
 * @return the loaded data, or null on failure
 */
u8* ArchiveFileDevice::doLoad_(LoadArg& rArg)
{
    if (mArchive == nullptr)
    {
        SEAD_ASSERT_MSG(false, "no archive mounted");
        return nullptr;
    }

    if (rArg.buffer || rArg.heap != nullptr)
    {
        return FileDevice::doLoad_(rArg);
    }

    ArchiveRes::FileInfo info{};
    auto* ret = mArchive->getFile(rArg.path, &info);

    if (ret == nullptr)
    {
        return nullptr;
    }

    SEAD_ASSERT(rArg.alignment == 0 || PtrUtil::isAligned(ret, Mathi::abs(rArg.alignment)));

    if (rArg.buffer_size_alignment && info.mLength % rArg.buffer_size_alignment != 0)
    {
        SEAD_WARN("archive file size[%u] is not multipe of rArg.buffer_size_alignment[%u]",
                  info.mLength, rArg.buffer_size_alignment);
        return nullptr;
    }

    rArg.read_size = info.mLength;
    rArg.roundup_size = info.mLength;
    rArg.need_unload = false;
    return const_cast<u8*>(static_cast<const u8*>(ret));
}

/**
 * Opens an archive file by path.
 * @param pHandle the handle to open
 * @param rPath the file path
 * @return this device, or null on failure
 */
FileDevice* ArchiveFileDevice::doOpen_(FileHandle* pHandle, const SafeString& rPath,
                                       FileDevice::FileOpenFlag)
{
    if (pHandle == nullptr)
    {
        SEAD_ASSERT_MSG(false, "invalid pHandle");
        return nullptr;
    }

    if (mArchive == nullptr)
    {
        SEAD_ASSERT_MSG(false, "no archive mounted");
        return nullptr;
    }

    if (rPath.cstr() == nullptr)
    {
        SEAD_ASSERT_MSG(false, "invalid filename");
        return nullptr;
    }

    auto* inner = constructArchiveFileHandle_(pHandle);

    auto* file_data = static_cast<const u8*>(mArchive->getFile(rPath, &inner->mFileInfo));

    if (!file_data)
    {
        return nullptr;
    }

    inner->mFileData = file_data;
    inner->mPos = 0;
    return this;
}

/**
 * Opens an archive file by entry ID.
 * @param pHandle the handle to open
 * @param id the entry ID
 * @return this device, or null on failure
 */
FileDevice* ArchiveFileDevice::doOpenWithEntryID_(FileHandle* pHandle, s32 id,
                                                  FileDevice::FileOpenFlag)
{
    if (pHandle == nullptr)
    {
        SEAD_ASSERT_MSG(false, "invalid pHandle");
        return nullptr;
    }

    if (mArchive == nullptr)
    {
        SEAD_ASSERT_MSG(false, "no archive mounted");
        return nullptr;
    }

    auto* inner = constructArchiveFileHandle_(pHandle);

    auto* file_data = static_cast<const u8*>(mArchive->getFileFast(id, &inner->mFileInfo));

    if (!file_data)
    {
        return nullptr;
    }

    inner->mFileData = file_data;
    inner->mPos = 0;
    return this;
}

/**
 * Converts a path to an archive entry ID.
 * @param rPath the file path
 * @return the entry ID
 */
s32 ArchiveFileDevice::doConvertPathToEntryID_(const SafeString& rPath)
{
    if (mArchive == nullptr)
    {
        SEAD_ASSERT_MSG(false, "no archive mounted");
        return 0;
    }

    return mArchive->convertPathToEntryID(rPath);
}

/**
 * Closes an archive file, which needs no work.
 * @return true
 */
bool ArchiveFileDevice::doClose_(FileHandle*)
{
    return true;
}

/**
 * Fails because flushing is not supported.
 * @return false
 */
bool ArchiveFileDevice::doFlush_(FileHandle*)
{
    SEAD_ASSERT_MSG(false, "not supported");
    return false;
}

/**
 * Fails because removing files is not supported.
 * @return false
 */
bool ArchiveFileDevice::doRemove_(const SafeString&)
{
    SEAD_ASSERT_MSG(false, "not supported");
    return false;
}

/**
 * Copies data from the archive file at the current position, clamped to the file end.
 * @param pBytesRead receives the number of bytes read
 * @param pHandle the file handle
 * @param pOutBuffer the buffer that receives the data
 * @param bytesToRead the number of bytes to read
 * @return true
 */
bool ArchiveFileDevice::doRead_(u32* pBytesRead, FileHandle* pHandle, u8* pOutBuffer,
                                u32 bytesToRead)
{
    ArchiveFileHandle* inner = getArchiveFileHandle_(pHandle);

    u32 read_size;

    if (inner->mPos + bytesToRead <= inner->mFileInfo.mLength)
    {
        read_size = bytesToRead;
    }
    else
    {
        read_size = inner->mFileInfo.mLength - inner->mPos;
    }

    MemUtil::copy(pOutBuffer, inner->mFileData + inner->mPos, read_size);

    inner->mPos += read_size;

    if (pBytesRead)
    {
        *pBytesRead = read_size;
    }

    return true;
}

/**
 * Moves the seek position of an archive file, failing if it would pass the file end.
 * @param pHandle the file handle
 * @param offset the offset relative to the origin
 * @param origin the seek origin
 * @return true on success
 */
bool ArchiveFileDevice::doSeek_(FileHandle* pHandle, s32 offset, FileDevice::SeekOrigin origin)
{
    ArchiveFileHandle* inner = getArchiveFileHandle_(pHandle);
    u32 new_position;

    switch (origin)
    {
    case cSeekOrigin_Begin:
        new_position = offset;
        break;
    case cSeekOrigin_Current:
        new_position = inner->mPos + offset;
        break;
    case cSeekOrigin_End:
        new_position = inner->mFileInfo.mLength + offset;
        break;
    default:
        SEAD_ASSERT_MSG(false, "Unexpected origin");
        return false;
    }

    if (new_position > inner->mFileInfo.mLength)
    {
        return false;
    }

    inner->mPos = new_position;
    return true;
}

/**
 * Gets the seek position of an archive file.
 * @param pSeekPos receives the current seek position
 * @param pHandle the file handle
 * @return true on success
 */
bool ArchiveFileDevice::doGetCurrentSeekPos_(u32* pSeekPos, FileHandle* pHandle)
{
    if (pHandle == nullptr)
    {
        SEAD_ASSERT_MSG(false, "invalid pHandle");
        return false;
    }

    if (mArchive == nullptr)
    {
        SEAD_ASSERT_MSG(false, "no archive mounted");
        return false;
    }

    ArchiveFileHandle* inner = getArchiveFileHandle_(pHandle);
    *pSeekPos = inner->mPos;
    return true;
}

/**
 * Opens a directory in the archive.
 * @param pHandle the handle to open
 * @param rPath the directory path
 * @return this device, or null on failure
 */
FileDevice* ArchiveFileDevice::doOpenDirectory_(DirectoryHandle* pHandle, const SafeString& rPath)
{
    if (pHandle == nullptr)
    {
        SEAD_ASSERT_MSG(false, "invalid pHandle");
        return nullptr;
    }

    if (mArchive == nullptr)
    {
        SEAD_ASSERT_MSG(false, "no archive mounted");
        return nullptr;
    }

    if (!mArchive->openDirectory(&getHandleBaseHandleBuffer_(pHandle), rPath))
    {
        return nullptr;
    }

    return this;
}

/**
 * Closes a directory in the archive.
 * @param pHandle the directory handle
 * @return true on success
 */
bool ArchiveFileDevice::doCloseDirectory_(DirectoryHandle* pHandle)
{
    if (mArchive == nullptr)
    {
        SEAD_ASSERT_MSG(false, "no archive mounted");
        return false;
    }

    return mArchive->closeDirectory(&getHandleBaseHandleBuffer_(pHandle));
}

/**
 * Reads entries from a directory in the archive.
 * @param pEntriesRead receives the number of entries read
 * @param pHandle the directory handle
 * @param pEntry the array that receives the entries
 * @param entriesToRead the maximum number of entries to read
 * @return true on success
 */
bool ArchiveFileDevice::doReadDirectory_(u32* pEntriesRead, DirectoryHandle* pHandle,
                                         DirectoryEntry* pEntry, u32 entriesToRead)
{
    auto* archive = mArchive;

    if (archive == nullptr)
    {
        SEAD_ASSERT_MSG(false, "no archive mounted");
        return false;
    }

    auto* buffer = &getHandleBaseHandleBuffer_(pHandle);
    SEAD_ASSERT(pEntry);
    const u32 actual_read_count = archive->readDirectory(buffer, pEntry, entriesToRead);

    if (pEntriesRead)
    {
        *pEntriesRead = actual_read_count;
    }

    return true;
}

/**
 * Sets the archive's current directory.
 * @param rPath the directory path
 * @return true on success
 */
bool ArchiveFileDevice::doSetCurrentDirectory_(const SafeString& rPath)
{
    if (mArchive == nullptr)
    {
        SEAD_ASSERT_MSG(false, "no archive mounted");
        return false;
    }

    if (rPath.cstr() == nullptr)
    {
        SEAD_ASSERT_MSG(false, "invalid filename");
        return false;
    }

    return mArchive->setCurrentDirectory(rPath);
}

/**
 * Fails because creating directories is not supported.
 * @return false
 */
bool ArchiveFileDevice::doMakeDirectory_(const SafeString&, u32)
{
    return false;
}

/**
 * Fails because raw errors are not implemented.
 * @return 0
 */
s32 ArchiveFileDevice::doGetLastRawError_() const
{
    SEAD_ASSERT_MSG(false, "not impremented");
    return 0;
}
}  // namespace sead
