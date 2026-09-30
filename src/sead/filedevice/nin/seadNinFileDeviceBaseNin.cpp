#include "filedevice/nin/seadNinFileDeviceBaseNin.h"
#include "filedevice/seadPath.h"

#include <nn/fs/fs_directories.h>
#include <nn/fs/fs_files.h>

namespace sead
{
struct NinFileDeviceBase::FileHandleInner
{
    nn::fs::FileHandle mHandle;
    s64 mOffset;
    bool mIsWriteMode;
    bool mDoNotFlushOnClose;
};

struct NinFileDeviceBase::DirectoryHandleInner
{
    nn::fs::DirectoryHandle mHandle;
};

/**
 * Constructs the device with a name and a file system mount point.
 * @param rName the device name
 * @param rMountPoint the mount point name
 */
NinFileDeviceBase::NinFileDeviceBase(const SafeString& rName, const SafeString& rMountPoint)
    : FileDevice(rName), mMountPoint(rMountPoint)
{
}

/**
 * Opens a file through nn::fs, creating or truncating it as the open flag requires.
 * @param pHandle the handle to open
 * @param rPath the file path
 * @param flag the open mode
 * @return this device, or null on failure
 */
FileDevice* NinFileDeviceBase::doOpen_(FileHandle* pHandle, const SafeString& rPath,
                                       FileDevice::FileOpenFlag flag)
{
    u32 mode;

    switch (flag)
    {
    case cFileOpenFlag_WriteOnly:
    case cFileOpenFlag_Create:
        mode = nn::fs::OpenMode_Write | nn::fs::OpenMode_Append;
        break;
    case cFileOpenFlag_ReadWrite:
        mode = nn::fs::OpenMode_ReadWrite | nn::fs::OpenMode_Append;
        break;
    default:
        mode = nn::fs::OpenMode_Read;
        break;
    }

    FixedSafeString<256> fs_path;

    if (!formatPathForFS_(&fs_path, rPath))
    {
        mLastError = nn::fs::ResultUnexpected();
        SEAD_WARN("invalid rPath. rPath = %s", fs_path.cstr());
        return nullptr;
    }

    bool is_new_file = true;

    if ((flag | cFileOpenFlag_ReadWrite) == cFileOpenFlag_Create)
    {
        bool is_file = false;
        nn::fs::DirectoryEntryType type;
        const auto result = nn::fs::GetEntryType(&type, fs_path.cstr());

        if (result.IsSuccess())
        {
            is_file = type == nn::fs::DirectoryEntryType_File;
        }
        else if (!nn::fs::ResultPathNotFound().Includes(result))
        {
            SEAD_WARN("nn::fs::GetEntryType failed. module = %d desc = %d inner_value = 0x%08x "
                      "rPath = %s",
                      result.GetModule(), result.GetDescription(), result.GetInnerValueForDebug(),
                      fs_path.cstr());
            mLastError = result;
            return nullptr;
        }

        if (flag == cFileOpenFlag_Create)
        {
            if (is_file)
            {
                mLastError = nn::fs::ResultPathAlreadyExists();
                return nullptr;
            }
        }
        else if (is_file)
        {
            is_new_file = false;
        }

        if (is_new_file)
        {
            const auto create_result = nn::fs::CreateFile(fs_path.cstr(), 0);

            if (create_result.IsFailure())
            {
                mLastError = create_result;
                return nullptr;
            }
        }
    }

    auto* handle_inner = new (getHandleBaseHandleBuffer_(pHandle).getBufferPtr()) FileHandleInner;
    handle_inner->mOffset = 0;
    handle_inner->mIsWriteMode = (mode >> 1) & 1;
    handle_inner->mDoNotFlushOnClose = false;

    const auto open_result = nn::fs::OpenFile(&handle_inner->mHandle, fs_path.cstr(), mode);
    mLastError = open_result;

    if (open_result.IsFailure())
    {
        if (!nn::fs::ResultPathNotFound().Includes(open_result))
        {
            SEAD_WARN(
                "nn::fs::OpenFile failed. module = %d desc = %d inner_value = 0x%08x rPath = %s",
                open_result.GetModule(), open_result.GetDescription(),
                open_result.GetInnerValueForDebug(), fs_path.cstr());
        }

        return nullptr;
    }

    if (flag == cFileOpenFlag_WriteOnly && !is_new_file)
    {
        const auto set_result = nn::fs::SetFileSize(handle_inner->mHandle, 0);

        if (set_result.IsFailure())
        {
            SEAD_WARN(
                "nn::fs::SetFileSize failed. module = %d desc = %d inner_value = 0x%08x rPath "
                "= %s",
                set_result.GetModule(), set_result.GetDescription(),
                set_result.GetInnerValueForDebug(), fs_path.cstr());
            nn::fs::CloseFile(handle_inner->mHandle);
            mLastError = set_result;
            return nullptr;
        }
    }

    return this;
}

/**
 * Closes a file, flushing it first if it was opened for writing.
 * @param pHandle the file handle
 * @return true on success
 */
bool NinFileDeviceBase::doClose_(FileHandle* pHandle)
{
    const auto* inner = getFileHandleInner_(pHandle);

    if (inner->mIsWriteMode && !inner->mDoNotFlushOnClose)
    {
        const auto result = nn::fs::FlushFile(inner->mHandle);

        if (result.IsFailure())
        {
            mLastError = result;
            nn::fs::CloseFile(inner->mHandle);
            return false;
        }
    }

    nn::fs::CloseFile(inner->mHandle);
    mLastError = nn::ResultSuccess();
    return true;
}

/**
 * Flushes a file, disabling the flush on close if it fails.
 * @param pHandle the file handle
 * @return true on success
 */
bool NinFileDeviceBase::doFlush_(FileHandle* pHandle)
{
    auto* inner = getFileHandleInner_(pHandle);

    mLastError = nn::fs::FlushFile(inner->mHandle);

    if (mLastError.IsFailure())
    {
        inner->mDoNotFlushOnClose = true;
        return false;
    }

    return true;
}

/**
 * Deletes a file.
 * @param rPath the file path
 * @return true on success
 */
bool NinFileDeviceBase::doRemove_(const SafeString& rPath)
{
    FixedSafeString<256> fs_path;

    if (!formatPathForFS_(&fs_path, rPath))
    {
        mLastError = nn::fs::ResultUnexpected();
        SEAD_WARN("invalid rPath. rPath = %s.", rPath.cstr());
        return false;
    }

    mLastError = nn::fs::DeleteFile(fs_path.cstr());

    if (mLastError.IsFailure())
    {
        SEAD_WARN(
            "nn::fs::DeleteFile failed. module = %d desc = %d inner_value = 0x%08x rPath = %s",
            mLastError.GetModule(), mLastError.GetDescription(), mLastError.GetInnerValueForDebug(),
            fs_path.cstr());
        return false;
    }

    return true;
}

/**
 * Reads from a file at the current offset and advances it.
 * @param pBytesRead receives the number of bytes read
 * @param pHandle the file handle
 * @param pOutBuffer the buffer that receives the data
 * @param bytesToRead the number of bytes to read
 * @return true on success
 */
bool NinFileDeviceBase::doRead_(u32* pBytesRead, FileHandle* pHandle, u8* pOutBuffer,
                                u32 bytesToRead)
{
    auto* inner = getFileHandleInner_(pHandle);

    u64 out_size = 0;
    mLastError = nn::fs::ReadFile(&out_size, inner->mHandle, inner->mOffset, pOutBuffer,
                                  bytesToRead, nn::fs::ReadOption{});
    if (mLastError.IsFailure())
    {
        SEAD_WARN("nn::fs::ReadFile failed. module = %d desc = %d inner_value = 0x%08x",
                  mLastError.GetModule(), mLastError.GetDescription(),
                  mLastError.GetInnerValueForDebug());
        return false;
    }

    inner->mOffset += out_size;

    if (pBytesRead)
    {
        *pBytesRead = out_size;
    }

    return true;
}

/**
 * Writes to a file at the current offset and advances it.
 * @param pBytesWritten receives the number of bytes written
 * @param pHandle the file handle
 * @param pInBuffer the data to write
 * @param bytesToWrite the number of bytes to write
 * @return true on success
 */
bool NinFileDeviceBase::doWrite_(u32* pBytesWritten, FileHandle* pHandle, const u8* pInBuffer,
                                 u32 bytesToWrite)
{
    auto* inner = getFileHandleInner_(pHandle);

    mLastError = nn::fs::WriteFile(inner->mHandle, inner->mOffset, pInBuffer, bytesToWrite,
                                   nn::fs::WriteOption{});
    if (mLastError.IsSuccess())
    {
        inner->mOffset += bytesToWrite;

        if (pBytesWritten)
        {
            *pBytesWritten = bytesToWrite;
        }

        return true;
    }

    SEAD_WARN("nn::fs::WriteFile failed. module = %d desc = %d inner_value = 0x%08x",
              mLastError.GetModule(), mLastError.GetDescription(),
              mLastError.GetInnerValueForDebug());
    inner->mDoNotFlushOnClose = true;
    return false;
}

/**
 * Moves the file offset.
 * @param pHandle the file handle
 * @param offset the offset relative to the origin
 * @param origin the seek origin
 * @return true on success
 */
bool NinFileDeviceBase::doSeek_(FileHandle* pHandle, s32 offset, FileDevice::SeekOrigin origin)
{
    auto* inner = getFileHandleInner_(pHandle);

    switch (origin)
    {
    case FileDevice::cSeekOrigin_Begin:
        inner->mOffset = offset;
        return true;
    case FileDevice::cSeekOrigin_Current:
        inner->mOffset += offset;
        return true;
    case FileDevice::cSeekOrigin_End:
    {
        SEAD_ASSERT(offset <= 0);
        u32 file_size = 0;

        if (!doGetFileSize_(&file_size, pHandle))
        {
            break;
        }

        inner->mOffset = file_size + offset;
        return true;
    }
    }

    return false;
}

/**
 * Gets the current file offset.
 * @param pSeekPos receives the current offset
 * @param pHandle the file handle
 * @return true
 */
bool NinFileDeviceBase::doGetCurrentSeekPos_(u32* pSeekPos, FileHandle* pHandle)
{
    *pSeekPos = getFileHandleInner_(pHandle)->mOffset;
    return true;
}

/**
 * Gets the size of the file at a path by opening it temporarily.
 * @param pFileSize receives the file size in bytes
 * @param rPath the file path
 * @return true on success
 */
bool NinFileDeviceBase::doGetFileSize_(u32* pFileSize, const SafeString& rPath)
{
    FileHandle handle;

    if (!doOpen_(&handle, rPath, cFileOpenFlag_ReadOnly))
    {
        return false;
    }

    const bool ret = doGetFileSize_(pFileSize, &handle);
    doClose_(&handle);
    return ret;
}

/**
 * Gets the size of an open file.
 * @param pFileSize receives the file size in bytes
 * @param pHandle the file handle
 * @return true on success
 */
bool NinFileDeviceBase::doGetFileSize_(u32* pFileSize, FileHandle* pHandle)
{
    const auto* inner = getFileHandleInner_(pHandle);
    s64 size = 0;
    mLastError = nn::fs::GetFileSize(&size, inner->mHandle);

    if (mLastError.IsSuccess())
    {
        *pFileSize = size;
        return true;
    }

    SEAD_WARN("nn::fs::GetFileSize failed. module = %d desc = %d inner_value = 0x%08x",
              mLastError.GetModule(), mLastError.GetDescription(),
              mLastError.GetInnerValueForDebug());
    return false;
}

/**
 * Checks whether a file exists.
 * @param pExists receives whether the file exists
 * @param rPath the file path
 * @return true on success
 */
bool NinFileDeviceBase::doIsExistFile_(bool* pExists, const SafeString& rPath)
{
    FixedSafeString<256> fs_path;

    if (!formatPathForFS_(&fs_path, rPath))
    {
        mLastError = nn::fs::ResultUnexpected();
        SEAD_WARN("invalid rPath. rPath = %s.", fs_path.cstr());
        return false;
    }

    nn::fs::DirectoryEntryType type;
    mLastError = nn::fs::GetEntryType(&type, fs_path.cstr());

    if (mLastError.IsSuccess())
    {
        *pExists = type == nn::fs::DirectoryEntryType_File;
        return true;
    }

    if (nn::fs::ResultPathNotFound().Includes(mLastError))
    {
        *pExists = false;
        return true;
    }

    SEAD_WARN("nn::fs::GetEntryType failed. module = %d desc = %d inner_value = 0x%08x rPath = %s",
              mLastError.GetModule(), mLastError.GetDescription(),
              mLastError.GetInnerValueForDebug(), fs_path.cstr());
    return false;
}

/**
 * Checks whether a directory exists.
 * @param pExists receives whether the directory exists
 * @param rPath the directory path
 * @return true on success
 */
bool NinFileDeviceBase::doIsExistDirectory_(bool* pExists, const SafeString& rPath)
{
    FixedSafeString<256> fs_path;

    if (!formatPathForFS_(&fs_path, rPath))
    {
        mLastError = nn::fs::ResultUnexpected();
        SEAD_WARN("invalid rPath. rPath = %s.", fs_path.cstr());
        return false;
    }

    nn::fs::DirectoryEntryType type;
    mLastError = nn::fs::GetEntryType(&type, fs_path.cstr());

    if (mLastError.IsSuccess())
    {
        *pExists = type == nn::fs::DirectoryEntryType_Directory;
        return true;
    }

    if (nn::fs::ResultPathNotFound().Includes(mLastError))
    {
        *pExists = false;
        return true;
    }

    SEAD_WARN("nn::fs::GetEntryType failed. module = %d desc = %d inner_value = 0x%08x rPath = %s",
              mLastError.GetModule(), mLastError.GetDescription(),
              mLastError.GetInnerValueForDebug(), fs_path.cstr());
    return false;
}

/**
 * Opens a directory for reading all entries.
 * @param pHandle the handle to open
 * @param rPath the directory path
 * @return this device, or null on failure
 */
FileDevice* NinFileDeviceBase::doOpenDirectory_(DirectoryHandle* pHandle, const SafeString& rPath)
{
    auto* inner = new (getHandleBaseHandleBuffer_(pHandle).getBufferPtr()) DirectoryHandleInner;

    FixedSafeString<256> fs_path;

    if (!formatPathForFS_(&fs_path, rPath))
    {
        mLastError = nn::fs::ResultUnexpected();
        SEAD_WARN("invalid rPath. rPath = %s.", fs_path.cstr());
        return nullptr;
    }

    mLastError =
        nn::fs::OpenDirectory(&inner->mHandle, fs_path.cstr(), nn::fs::OpenDirectoryMode_All);
    if (mLastError.IsSuccess())
    {
        return this;
    }

    if (nn::fs::ResultPathNotFound().Includes(mLastError))
    {
        return nullptr;
    }

    SEAD_WARN("nn::fs::OpenDirectory failed. module = %d desc = %d inner_value = 0x%08x rPath = %s",
              mLastError.GetModule(), mLastError.GetDescription(),
              mLastError.GetInnerValueForDebug(), fs_path.cstr());
    return nullptr;
}

/**
 * Closes a directory.
 * @param pHandle the directory handle
 * @return true
 */
bool NinFileDeviceBase::doCloseDirectory_(DirectoryHandle* pHandle)
{
    nn::fs::CloseDirectory(getDirHandleInner_(pHandle)->mHandle);
    return true;
}

/**
 * Reads directory entries one at a time until the count is reached or none remain.
 * @param pEntriesRead receives the number of entries read
 * @param pHandle the directory handle
 * @param pEntries the array that receives the entries
 * @param numEntries the maximum number of entries to read
 * @return true on success
 */
bool NinFileDeviceBase::doReadDirectory_(u32* pEntriesRead, DirectoryHandle* pHandle,
                                         DirectoryEntry* pEntries, u32 numEntries)
{
    const auto* inner = getDirHandleInner_(pHandle);

    for (u32 i = 0; i < numEntries; ++i)
    {
        nn::fs::DirectoryEntry entry;
        s64 count = 0;
        mLastError = nn::fs::ReadDirectory(&count, &entry, inner->mHandle, 1);

        if (mLastError.IsFailure())
        {
            SEAD_WARN("nn::fs::ReadDirectory failed. module = %d desc = %d inner_value = 0x%08x",
                      mLastError.GetModule(), mLastError.GetDescription(),
                      mLastError.GetInnerValueForDebug());
            return false;
        }

        // No more pEntries to read.
        if (count != 1)
        {
            if (pEntriesRead)
            {
                *pEntriesRead = i;
            }

            return true;
        }

        pEntries[i].name = entry.mName;
        pEntries[i].is_directory = entry.mTypeByte == nn::fs::DirectoryEntryType_Directory;
    }

    if (pEntriesRead)
    {
        *pEntriesRead = numEntries;
    }

    return true;
}

/**
 * Creates a directory.
 * @param rPath the directory path
 * @return true on success
 */
bool NinFileDeviceBase::doMakeDirectory_(const SafeString& rPath, u32)
{
    FixedSafeString<256> fs_path;

    if (!formatPathForFS_(&fs_path, rPath))
    {
        mLastError = nn::fs::ResultUnexpected();
        SEAD_WARN("invalid rPath. rPath = %s.", fs_path.cstr());
        return false;
    }

    const auto result = nn::fs::CreateDirectory(fs_path.cstr());
    mLastError = result;

    if (result.IsSuccess())
    {
        return true;
    }

    SEAD_WARN("nn::fs::CreateDirectory[%s] failed. module = %d desc = %d inner_value = 0x%08x",
              fs_path.cstr(), result.GetModule(), result.GetDescription(),
              result.GetInnerValueForDebug());
    return false;
}

/**
 * Gets the inner value of the last nn::fs result.
 * @return the raw error code
 */
s32 NinFileDeviceBase::doGetLastRawError_() const
{
    return mLastError.GetInnerValueForDebug();
}

/**
 * Resolves a path to its file system form.
 * @param pOut receives the resolved path
 * @param rPath the path to resolve
 */
void NinFileDeviceBase::doResolvePath_(BufferedSafeString* pOut, const SafeString& rPath) const
{
    formatPathForFS_(pOut, rPath);
}

/**
 * Formats a path as "mountpoint:/path" with '/' delimiters.
 * @param pOut receives the formatted path
 * @param rPath the path to format
 * @return true
 */
bool NinFileDeviceBase::formatPathForFS_(BufferedSafeString* pOut, const SafeString& rPath) const
{
    pOut->format("%s:/%s", mMountPoint.cstr(), rPath.cstr());
    Path::changeDelimiter(pOut, '/');
    return true;
}

/**
 * Gets the file handle data stored in a handle.
 * @param pHandle the handle
 * @return the inner file handle
 */
NinFileDeviceBase::FileHandleInner* NinFileDeviceBase::getFileHandleInner_(HandleBase* pHandle) const
{
    return reinterpret_cast<FileHandleInner*>(getHandleBaseHandleBuffer_(pHandle).getBufferPtr());
}

/**
 * Gets the directory handle data stored in a handle.
 * @param pHandle the handle
 * @return the inner directory handle
 */
NinFileDeviceBase::DirectoryHandleInner* NinFileDeviceBase::getDirHandleInner_(HandleBase* pHandle) const
{
    return reinterpret_cast<DirectoryHandleInner*>(getHandleBaseHandleBuffer_(pHandle).getBufferPtr());
}
}  // namespace sead
