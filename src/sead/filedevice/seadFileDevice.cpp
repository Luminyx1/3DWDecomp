#include <basis/seadNew.h>
#include <basis/seadRawPrint.h>
#include <filedevice/seadFileDevice.h>
#include <filedevice/seadFileDeviceMgr.h>
#include <filedevice/seadPath.h>
#include <heap/seadHeapMgr.h>
#include <math/seadMathCalcCommon.h>

namespace sead
{
/**
 * Closes the file through the device that opened it.
 * @return true on success
 */
bool FileHandle::close()
{
    if (!mOriginalDevice)
    {
        SEAD_ASSERT_MSG(false, "handle not opened");
        return false;
    }

    return mOriginalDevice->close(this);
}

/**
 * Tries to close the file through the device that opened it.
 * @return true on success
 */
bool FileHandle::tryClose()
{
    if (!mOriginalDevice)
    {
        SEAD_ASSERT_MSG(false, "handle not opened");
        return false;
    }

    return mOriginalDevice->tryClose(this);
}

/**
 * Flushes the file through the device that opened it.
 * @return true on success
 */
bool FileHandle::flush()
{
    if (!mOriginalDevice)
    {
        SEAD_ASSERT_MSG(false, "handle not opened");
        return false;
    }

    return mOriginalDevice->flush(this);
}

/**
 * Tries to flush the file through the device that opened it.
 * @return true on success
 */
bool FileHandle::tryFlush()
{
    if (!mOriginalDevice)
    {
        SEAD_ASSERT_MSG(false, "handle not opened");
        return false;
    }

    return mOriginalDevice->tryFlush(this);
}

/**
 * Reads data from the file.
 * @param pOutBuffer the buffer that receives the data
 * @param bytesToRead the number of bytes to read
 * @return the number of bytes read
 */
u32 FileHandle::read(u8* pOutBuffer, u32 bytesToRead)
{
    if (!mDevice)
    {
        SEAD_ASSERT_MSG(false, "handle not opened");
        return 0;
    }

    return mDevice->read(this, pOutBuffer, bytesToRead);
}

/**
 * Tries to read data from the file.
 * @param pActualSize receives the number of bytes read
 * @param pData the buffer that receives the data
 * @param size the number of bytes to read
 * @return true on success
 */
bool FileHandle::tryRead(u32* pActualSize, u8* pData, u32 size)
{
    if (!mDevice)
    {
        SEAD_ASSERT_MSG(false, "handle not opened");
        return false;
    }

    return mDevice->tryRead(pActualSize, this, pData, size);
}

/**
 * Writes data to the file.
 * @param pData the data to write
 * @param size the number of bytes to write
 * @return the number of bytes written
 */
u32 FileHandle::write(const u8* pData, u32 size)
{
    if (!mDevice)
    {
        SEAD_ASSERT_MSG(false, "handle not opened");
        return 0;
    }

    return mDevice->write(this, pData, size);
}

/**
 * Tries to write data to the file.
 * @param pActualSize receives the number of bytes written
 * @param pData the data to write
 * @param size the number of bytes to write
 * @return true on success
 */
bool FileHandle::tryWrite(u32* pActualSize, const u8* pData, u32 size)
{
    if (!mDevice)
    {
        SEAD_ASSERT_MSG(false, "handle not opened");
        return false;
    }

    return mDevice->tryWrite(pActualSize, this, pData, size);
}

/**
 * Moves the file's seek position.
 * @param offset the offset relative to the origin
 * @param origin the seek origin
 * @return true on success
 */
bool FileHandle::seek(s32 offset, FileDevice::SeekOrigin origin)
{
    if (!mDevice)
    {
        SEAD_ASSERT_MSG(false, "handle not opened");
        return false;
    }

    return mDevice->seek(this, offset, origin);
}

/**
 * Tries to move the file's seek position.
 * @param offset the offset relative to the origin
 * @param origin the seek origin
 * @return true on success
 */
bool FileHandle::trySeek(s32 offset, FileDevice::SeekOrigin origin)
{
    if (!mDevice)
    {
        SEAD_ASSERT_MSG(false, "handle not opened");
        return false;
    }

    return mDevice->trySeek(this, offset, origin);
}

/**
 * Gets the file's current seek position.
 * @return the current seek position
 */
u32 FileHandle::getCurrentSeekPos()
{
    if (!mDevice)
    {
        SEAD_ASSERT_MSG(false, "handle not opened");
        return 0;
    }

    return mDevice->getCurrentSeekPos(this);
}

/**
 * Tries to get the file's current seek position.
 * @param pos receives the current seek position
 * @return true on success
 */
bool FileHandle::tryGetCurrentSeekPos(u32* pos)
{
    if (!mDevice)
    {
        SEAD_ASSERT_MSG(false, "handle not opened");
        return false;
    }

    return mDevice->tryGetCurrentSeekPos(pos, this);
}

/**
 * Gets the size of the file.
 * @return the file size in bytes
 */
u32 FileHandle::getFileSize()
{
    if (!mDevice)
    {
        SEAD_ASSERT_MSG(false, "handle not opened");
        return 0;
    }

    return mDevice->getFileSize(this);
}

/**
 * Tries to get the size of the file.
 * @param pSize receives the file size in bytes
 * @return true on success
 */
bool FileHandle::tryGetFileSize(u32* pSize)
{
    if (!mDevice)
    {
        SEAD_ASSERT_MSG(false, "handle not opened");
        return false;
    }

    return mDevice->tryGetFileSize(pSize, this);
}

/**
 * Closes the directory through the device that opened it.
 * @return true on success
 */
bool DirectoryHandle::close()
{
    if (!mOriginalDevice)
    {
        SEAD_ASSERT_MSG(false, "handle not opened");
        return false;
    }

    return mOriginalDevice->closeDirectory(this);
}

/**
 * Tries to close the directory through the device that opened it.
 * @return true on success
 */
bool DirectoryHandle::tryClose()
{
    if (!mOriginalDevice)
    {
        SEAD_ASSERT_MSG(false, "handle not opened");
        return false;
    }

    return mOriginalDevice->tryCloseDirectory(this);
}

/**
 * Reads entries from the directory.
 * @param pEntries the array that receives the entries
 * @param count the maximum number of entries to read
 * @return the number of entries read
 */
u32 DirectoryHandle::read(DirectoryEntry* pEntries, u32 count)
{
    if (!mDevice)
    {
        SEAD_ASSERT_MSG(false, "handle not opened");
        return false;
    }

    return mDevice->readDirectory(this, pEntries, count);
}

/**
 * Tries to read entries from the directory.
 * @param pActualCount receives the number of entries read
 * @param pEntries the array that receives the entries
 * @param count the maximum number of entries to read
 * @return true on success
 */
bool DirectoryHandle::tryRead(u32* pActualCount, DirectoryEntry* pEntries, u32 count)
{
    if (!mDevice)
    {
        SEAD_ASSERT_MSG(false, "handle not opened");
        return false;
    }

    return mDevice->tryReadDirectory(pActualCount, this, pEntries, count);
}

/**
 * Destroys the device, unmounting it from the file device manager if one exists.
 */
FileDevice::~FileDevice()
{
    if (FileDeviceMgr::instance() != NULL)
    {
        FileDeviceMgr::instance()->unmount(this);
    }
}

/**
 * Checks whether a handle was opened by this device.
 * @param pHandle the handle to check
 * @return true if the handle belongs to this device
 */
bool FileDevice::isMatchDevice_(const HandleBase* pHandle) const
{
    return pHandle->mDevice == this;
}

/**
 * Loads a whole file into a buffer, allocating one from the heap if none is given.
 * @param rArg the load settings, updated with the read and rounded-up sizes
 * @return the loaded buffer, or null on failure
 */
u8* FileDevice::doLoad_(LoadArg& rArg)
{
    if (rArg.buffer && rArg.buffer_size == 0)
    {
        SEAD_WARN("rArg.buffer is specified, but rArg.buffer_size is zero");
        return nullptr;
    }

    if (rArg.buffer_size_alignment % cBufferMinAlignment != 0)
    {
        SEAD_WARN(
            "rArg.buffer_size_alignment[%u] is not multipe of FileDevice::cBufferMinAlignment[%u]",
            rArg.buffer_size_alignment, cBufferMinAlignment);
        return nullptr;
    }

    FileHandle handle;
    if (!tryOpen(&handle, rArg.path, FileDevice::cFileOpenFlag_ReadOnly, rArg.div_size))
    {
        return nullptr;
    }

    u32 bytesToRead = rArg.buffer_size;
    if (!rArg.buffer || rArg.check_read_entire_file)
    {
        u32 fileSize = 0;
        if (!tryGetFileSize(&fileSize, &handle))
        {
            return nullptr;
        }

        if (fileSize == 0)
        {
            SEAD_WARN("file_size is zero.[%s]", rArg.path.cstr());
            return nullptr;
        }

        if (bytesToRead != 0)
        {
            if (bytesToRead < fileSize)
            {
                SEAD_WARN("rArg.buffer_size[%u] is smaller than file size[%u]", bytesToRead,
                          fileSize);
                return nullptr;
            }

            if (rArg.buffer_size_alignment && bytesToRead % rArg.buffer_size_alignment != 0)
            {
                SEAD_WARN("rArg.buffer_size[%u] is not multipe of rArg.buffer_size_alignment[%u]",
                          bytesToRead, rArg.buffer_size_alignment);
                return nullptr;
            }
        }
        else
        {
            if (rArg.buffer_size_alignment)
            {
                bytesToRead = Mathu::roundUp(fileSize, rArg.buffer_size_alignment);
            }
            else
            {
                bytesToRead = Mathi::roundUpPow2(fileSize, FileDevice::cBufferMinAlignment);
            }
        }
    }

    u8* buf = rArg.buffer;
    bool allocated = false;

    if (buf == nullptr)
    {
        const s32 sign = Mathi::sign(rArg.alignment);
        s32 alignment = Mathi::abs(rArg.alignment);
        alignment = sign * ((alignment < cBufferMinAlignment) ? cBufferMinAlignment : alignment);

        Heap* heap = rArg.heap;
        if (!heap)
        {
            heap = HeapMgr::instance()->getCurrentHeap();
        }

        void* raw_buf = heap->tryAlloc(bytesToRead, alignment);
        if (!raw_buf)
        {
            if (rArg.assert_on_alloc_fail)
            {
                SEAD_ASSERT_MSG(false, "alloc size[%u] failed in heap[%s] for file[%s]",
                                bytesToRead, heap->getName().cstr(), rArg.path.cstr());
            }

            return nullptr;
        }

        buf = new (raw_buf) u8[bytesToRead];
        allocated = true;
    }

    u32 bytesRead = 0;
    if (!tryRead(&bytesRead, &handle, buf, bytesToRead))
    {
        if (allocated)
        {
            delete[] buf;
        }

        return nullptr;
    }

    if (!tryClose(&handle))
    {
        if (allocated)
        {
            delete[] buf;
        }

        return nullptr;
    }

    rArg.read_size = bytesRead;
    rArg.roundup_size = bytesToRead;
    rArg.need_unload = allocated;

    return buf;
}

/**
 * Saves a buffer to a file.
 * @param rArg the save settings, updated with the written size
 * @return true on success
 */
bool FileDevice::doSave_(FileDevice::SaveArg& rArg)
{
    if (!rArg.buffer)
    {
        SEAD_ASSERT_MSG(false, "rArg.buffer must be set for save file[%s]", rArg.path.cstr());
        return false;
    }

    FileHandle handle;
    if (!tryOpen(&handle, rArg.path, cFileOpenFlag_WriteOnly))
    {
        return false;
    }

    const bool ret =
        rArg.buffer_size == 0 || tryWrite(&rArg.write_size, &handle, rArg.buffer, rArg.buffer_size);

    if (!tryClose(&handle))
    {
        return false;
    }

    return ret;
}

/**
 * Prints a path and its resolved form for debugging.
 * @param rPath the path to trace
 */
void FileDevice::doTracePath_(const SafeString& rPath) const
{
    SEAD_DEBUG_PRINT("[%s] %s\n", mDriveName.cstr(), rPath.cstr());
    FixedSafeString<512> out;
    doResolvePath_(&out, rPath);
    SEAD_DEBUG_PRINT("  -> %s\n", out.cstr());
}

/**
 * Resolves a path by copying it unchanged.
 * @param pOut receives the resolved path
 * @param rPath the path to resolve
 */
void FileDevice::doResolvePath_(BufferedSafeString* pOut, const SafeString& rPath) const
{
    pOut->copy(rPath);
}

/**
 * Checks whether the device has permission and is available.
 * @return true if the device is available
 */
bool FileDevice::isAvailable() const
{
    SEAD_ASSERT_MSG(mPermission, "Device permission error.");
    if (!mPermission)
    {
        return false;
    }

    return doIsAvailable_();
}

/**
 * Loads a file if the device has permission.
 * @param rArg the load settings
 * @return the loaded buffer, or null on failure
 */
u8* FileDevice::tryLoad(LoadArg& rArg)
{
    SEAD_ASSERT_MSG(mPermission, "Device permission error.");
    if (!mPermission)
    {
        return NULL;
    }

    return doLoad_(rArg);
}

/**
 * Saves a file if the device has permission.
 * @param rArg the save settings
 * @return true on success
 */
bool FileDevice::trySave(FileDevice::SaveArg& rArg)
{
    SEAD_ASSERT_MSG(mPermission, "Device permission error.");
    if (!mPermission)
    {
        return false;
    }

    return doSave_(rArg);
}

/**
 * Opens a file and binds the handle to the device that opened it.
 * @param pHandle the handle to open
 * @param rPath the file path
 * @param flag the open mode
 * @param divSize the read division size
 * @return the device that opened the file, or null on failure
 */
FileDevice* FileDevice::tryOpen(FileHandle* pHandle, const SafeString& rPath, FileOpenFlag flag,
                                u32 divSize)
{
    SEAD_ASSERT_MSG(mPermission, "Device permission error.");
    if (!mPermission)
    {
        return nullptr;
    }

    if (pHandle == nullptr)
    {
        SEAD_ASSERT_MSG(false, "pHandle is null");
        return nullptr;
    }

    setFileHandleDivSize_(pHandle, divSize);
    FileDevice* device = doOpen_(pHandle, rPath, flag);
    setHandleBaseFileDevice_(pHandle, device);
    if (device)
    {
        setHandleBaseOriginalFileDevice_(pHandle, this);
    }

    return device;
}

/**
 * Closes a file opened by this device and unbinds the handle.
 * @param pHandle the handle to close
 * @return true on success
 */
bool FileDevice::tryClose(FileHandle* pHandle)
{
    SEAD_ASSERT_MSG(mPermission, "Device permission error.");
    if (!mPermission)
    {
        return false;
    }

    if (pHandle == nullptr)
    {
        SEAD_ASSERT_MSG(false, "pHandle is null");
        return false;
    }

    if (!isMatchDevice_(pHandle))
    {
        SEAD_ASSERT_MSG(false, "pHandle device miss match");
        return false;
    }

    bool closed = doClose_(pHandle);
    if (closed)
    {
        setHandleBaseFileDevice_(pHandle, nullptr);
        setHandleBaseOriginalFileDevice_(pHandle, nullptr);
    }

    return closed;
}

/**
 * Flushes a file opened by this device.
 * @param pHandle the file handle
 * @return true on success
 */
bool FileDevice::tryFlush(FileHandle* pHandle)
{
    SEAD_ASSERT_MSG(mPermission, "Device permission error.");
    if (!mPermission)
    {
        return false;
    }

    if (!pHandle)
    {
        SEAD_ASSERT_MSG(false, "pHandle is null");
        return false;
    }

    if (!isMatchDevice_(pHandle))
    {
        SEAD_ASSERT_MSG(false, "pHandle device miss match");
        return false;
    }

    return doFlush_(pHandle);
}

/**
 * Removes a file if the device has permission.
 * @param rStr the path of the file to remove
 * @return true on success
 */
bool FileDevice::tryRemove(const SafeString& rStr)
{
    SEAD_ASSERT_MSG(mPermission, "Device permission error.");
    if (!mPermission)
    {
        return false;
    }

    return doRemove_(rStr);
}

/**
 * Reads from a file, splitting the read into chunks when the handle has a division size.
 * @param pBytesRead receives the number of bytes read
 * @param pHandle the file handle
 * @param pOutBuffer the buffer that receives the data
 * @param bytesToRead the number of bytes to read
 * @return true on success
 */
bool FileDevice::tryRead(u32* pBytesRead, FileHandle* pHandle, u8* pOutBuffer, u32 bytesToRead)
{
    SEAD_ASSERT_MSG(mPermission, "Device permission error.");
    if (!mPermission)
    {
        return false;
    }

    if (pHandle == nullptr)
    {
        SEAD_ASSERT_MSG(false, "pHandle is null");
        return false;
    }

    if (!isMatchDevice_(pHandle))
    {
        SEAD_ASSERT_MSG(false, "pHandle device miss match");
        return false;
    }

    if (pOutBuffer == nullptr)
    {
        SEAD_ASSERT_MSG(false, "buf is null");
        return false;
    }

    if (pHandle->mDivSize == 0)
    {
        const bool ret = doRead_(pBytesRead, pHandle, pOutBuffer, bytesToRead);
        SEAD_ASSERT_MSG(!pBytesRead || *pBytesRead <= bytesToRead, "buffer overflow");
        return ret;
    }

    u32 totalReadSize = 0;

    do
    {
        u32 size =
            (static_cast<s32>(bytesToRead) < pHandle->mDivSize) ? bytesToRead : pHandle->mDivSize;
        u32 readSize = 0;

        if (!doRead_(&readSize, pHandle, pOutBuffer, size))
        {
            if (pBytesRead != NULL)
            {
                *pBytesRead = totalReadSize;
            }

            return false;
        }

        totalReadSize += readSize;
        if (readSize < size)
        {
            break;
        }

        pOutBuffer += readSize;
        bytesToRead -= size;
    } while (bytesToRead != 0);

    if (pBytesRead != NULL)
    {
        *pBytesRead = totalReadSize;
    }

    return true;
}

/**
 * Writes to a file opened by this device.
 * @param pBytesWritten receives the number of bytes written
 * @param pHandle the file handle
 * @param pInBuffer the data to write
 * @param bytesToWrite the number of bytes to write
 * @return true on success
 */
bool FileDevice::tryWrite(u32* pBytesWritten, FileHandle* pHandle, const u8* pInBuffer,
                          u32 bytesToWrite)
{
    SEAD_ASSERT_MSG(mPermission, "Device permission error.");
    if (!mPermission)
    {
        return false;
    }

    if (pHandle == nullptr)
    {
        SEAD_ASSERT_MSG(false, "pHandle is null");
        return false;
    }

    if (pInBuffer == nullptr)
    {
        SEAD_ASSERT_MSG(false, "buf is null");
        return false;
    }

    if (!isMatchDevice_(pHandle))
    {
        SEAD_ASSERT_MSG(false, "pHandle device miss match");
        return false;
    }

    return doWrite_(pBytesWritten, pHandle, pInBuffer, bytesToWrite);
}

/**
 * Moves the seek position of a file opened by this device.
 * @param pHandle the file handle
 * @param offset the offset relative to the origin
 * @param origin the seek origin
 * @return true on success
 */
bool FileDevice::trySeek(FileHandle* pHandle, s32 offset, FileDevice::SeekOrigin origin)
{
    SEAD_ASSERT_MSG(mPermission, "Device permission error.");
    if (!mPermission)
    {
        return false;
    }

    if (pHandle == nullptr)
    {
        SEAD_ASSERT_MSG(false, "pHandle is null");
        return false;
    }

    if (!isMatchDevice_(pHandle))
    {
        SEAD_ASSERT_MSG(false, "pHandle device miss match");
        return false;
    }

    return doSeek_(pHandle, offset, origin);
}

/**
 * Gets the seek position of a file opened by this device.
 * @param pSeekPos receives the current seek position
 * @param pHandle the file handle
 * @return true on success
 */
bool FileDevice::tryGetCurrentSeekPos(u32* pSeekPos, FileHandle* pHandle)
{
    SEAD_ASSERT_MSG(mPermission, "Device permission error.");
    if (!mPermission)
    {
        return false;
    }

    if (pHandle == NULL)
    {
        SEAD_ASSERT_MSG(false, "pHandle is null");
        return false;
    }

    if (!isMatchDevice_(pHandle))
    {
        SEAD_ASSERT_MSG(false, "pHandle device miss match");
        return false;
    }

    if (pSeekPos == NULL)
    {
        SEAD_ASSERT_MSG(false, "pos is null");
        return false;
    }

    return doGetCurrentSeekPos_(pSeekPos, pHandle);
}

/**
 * Gets the size of the file at a path.
 * @param pFileSize receives the file size in bytes
 * @param rPath the file path
 * @return true on success
 */
bool FileDevice::tryGetFileSize(u32* pFileSize, const SafeString& rPath)
{
    SEAD_ASSERT_MSG(mPermission, "Device permission error.");
    if (!mPermission)
    {
        return false;
    }

    if (pFileSize == NULL)
    {
        SEAD_ASSERT_MSG(false, "size is null");
        return false;
    }

    return doGetFileSize_(pFileSize, rPath);
}

/**
 * Gets the size of an open file.
 * @param pSize receives the file size in bytes
 * @param pHandle the file handle
 * @return true on success
 */
bool FileDevice::tryGetFileSize(u32* pSize, FileHandle* pHandle)
{
    SEAD_ASSERT_MSG(mPermission, "Device permission error.");
    if (!mPermission)
    {
        return false;
    }

    if (pHandle == nullptr)
    {
        SEAD_ASSERT_MSG(false, "pHandle is null");
        return false;
    }

    if (pSize == nullptr)
    {
        SEAD_ASSERT_MSG(false, "pSize is null");
        return false;
    }

    return doGetFileSize_(pSize, pHandle);
}

/**
 * Checks whether a file exists.
 * @param pExists receives whether the file exists
 * @param rPath the file path
 * @return true on success
 */
bool FileDevice::tryIsExistFile(bool* pExists, const SafeString& rPath)
{
    SEAD_ASSERT_MSG(mPermission, "Device permission error.");
    if (!mPermission)
    {
        return false;
    }

    if (pExists == NULL)
    {
        SEAD_ASSERT_MSG(false, "is_exist is null");
        return false;
    }

    return doIsExistFile_(pExists, rPath);
}

/**
 * Checks whether a directory exists.
 * @param pExists receives whether the directory exists
 * @param rPath the directory path
 * @return true on success
 */
bool FileDevice::tryIsExistDirectory(bool* pExists, const SafeString& rPath)
{
    SEAD_ASSERT_MSG(mPermission, "Device permission error.");
    if (!mPermission)
    {
        return false;
    }

    if (pExists == NULL)
    {
        SEAD_ASSERT_MSG(false, "is_exist is null");
        return false;
    }

    return doIsExistDirectory_(pExists, rPath);
}

/**
 * Opens a directory and binds the handle to the device that opened it.
 * @param pHandle the handle to open
 * @param rPath the directory path
 * @return the device that opened the directory, or null on failure
 */
FileDevice* FileDevice::tryOpenDirectory(DirectoryHandle* pHandle, const SafeString& rPath)
{
    SEAD_ASSERT_MSG(mPermission, "Device permission error.");
    if (!mPermission)
    {
        return NULL;
    }

    if (pHandle == NULL)
    {
        SEAD_ASSERT_MSG(false, "pHandle is null");
        return NULL;
    }

    FileDevice* device = doOpenDirectory_(pHandle, rPath);
    setHandleBaseFileDevice_(pHandle, device);
    if (device != NULL)
    {
        setHandleBaseOriginalFileDevice_(pHandle, this);
    }

    return device;
}

/**
 * Closes a directory opened by this device and unbinds the handle.
 * @param pHandle the directory handle
 * @return true on success
 */
bool FileDevice::tryCloseDirectory(DirectoryHandle* pHandle)
{
    SEAD_ASSERT_MSG(mPermission, "Device permission error.");
    if (!mPermission)
    {
        return false;
    }

    if (pHandle == NULL)
    {
        SEAD_ASSERT_MSG(false, "pHandle is null");
        return false;
    }

    if (!isMatchDevice_(pHandle))
    {
        SEAD_ASSERT_MSG(false, "pHandle device miss match");
        return false;
    }

    bool closed = doCloseDirectory_(pHandle);
    if (closed)
    {
        setHandleBaseFileDevice_(pHandle, NULL);
        setHandleBaseOriginalFileDevice_(pHandle, NULL);
    }

    return closed;
}

/**
 * Reads entries from a directory opened by this device.
 * @param pEntriesRead receives the number of entries read
 * @param pHandle the directory handle
 * @param pEntries the array that receives the entries
 * @param entriesToRead the maximum number of entries to read
 * @return true on success
 */
bool FileDevice::tryReadDirectory(u32* pEntriesRead, DirectoryHandle* pHandle,
                                  DirectoryEntry* pEntries, u32 entriesToRead)
{
    SEAD_ASSERT_MSG(mPermission, "Device permission error.");
    if (!mPermission)
    {
        return false;
    }

    if (pHandle == NULL)
    {
        SEAD_ASSERT_MSG(false, "pHandle is null");
        return false;
    }

    if (!isMatchDevice_(pHandle))
    {
        SEAD_ASSERT_MSG(false, "pHandle device miss match");
        return false;
    }

    u32 readCount = 0;
    bool success = doReadDirectory_(&readCount, pHandle, pEntries, entriesToRead);

    if (pEntriesRead != NULL)
    {
        *pEntriesRead = readCount;
    }

    if (readCount > entriesToRead)
    {
        SEAD_ASSERT_MSG(false, "buffer overflow");
        return false;
    }

    return success;
}

/**
 * Creates a directory if the device has permission.
 * @param rPath the directory path
 * @param permission the permission flags
 * @return true on success
 */
bool FileDevice::tryMakeDirectory(const SafeString& rPath, u32 permission)
{
    SEAD_ASSERT_MSG(mPermission, "Device permission error.");
    if (!mPermission)
    {
        return false;
    }

    return doMakeDirectory_(rPath, permission);
}

/**
 * Creates a directory along with any missing parent directories.
 * @param rPath the directory path
 * @param x the permission flags passed to each directory creation
 * @return true on success
 */
bool FileDevice::tryMakeDirectoryWithParent(const SafeString& rPath, u32 x)
{
    SEAD_ASSERT_MSG(mPermission, "Device permission error.");
    if (!mPermission)
    {
        return false;
    }

    bool exists = false;
    if (!doIsExistDirectory_(&exists, rPath))
    {
        return false;
    }

    if (exists)
    {
        return true;
    }

    FixedSafeString<512> dir_name;
    int num_existing_parents = 1;
    bool should_trim = true;
    bool reached_end = !Path::getDirectoryName(&dir_name, rPath);
    while (!reached_end)
    {
        exists = false;
        if (!tryIsExistDirectory(&exists, dir_name))
        {
            return false;
        }

        if (exists)
        {
            should_trim = false;
            break;
        }

        reached_end = !Path::getDirectoryName(&dir_name, dir_name);
        ++num_existing_parents;
    }

    if (should_trim)
    {
        dir_name.trim(0);
    }

    int num_path_components = 0;
    auto counting_iterator = rPath.tokenBegin("/");
    const auto end = rPath.tokenEnd("/");
    for (; end != counting_iterator; ++counting_iterator)
    {
        ++num_path_components;
    }

    auto it = rPath.tokenBegin("/");
    int num_levels_to_create = num_path_components - num_existing_parents;
    for (; end != it; ++it)
    {
        if (num_levels_to_create >= 1)
        {
            --num_levels_to_create;
            continue;
        }

        FixedSafeString<128> component;
        it.get(&component);

        if (dir_name != "")
        {
            dir_name.append("/");
        }

        dir_name.append(component);

        if (!tryMakeDirectory(dir_name, x))
        {
            return false;
        }
    }

    return true;
}

/**
 * Gets the last raw error code from the device.
 * @return the raw error code
 */
s32 FileDevice::getLastRawError() const
{
    return doGetLastRawError_();
}

/**
 * Gets the implementation buffer of a handle.
 * @param pHandle the handle
 * @return the handle's buffer
 */
HandleBuffer& FileDevice::getHandleBaseHandleBuffer_(HandleBase* pHandle) const
{
    return pHandle->mHandleBuffer;
}

/**
 * Sets the read division size of a file handle.
 * @param pHandle the file handle
 * @param divSize the division size
 */
void FileDevice::setFileHandleDivSize_(FileHandle* pHandle, u32 divSize) const
{
    pHandle->mDivSize = divSize;
}

/**
 * Sets the device of a handle.
 * @param pHandle the handle
 * @param pDevice the device
 */
void FileDevice::setHandleBaseFileDevice_(HandleBase* pHandle, FileDevice* pDevice) const
{
    pHandle->mDevice = pDevice;
}

/**
 * Sets the original device of a handle.
 * @param pHandle the handle
 * @param pDevice the original device
 */
void FileDevice::setHandleBaseOriginalFileDevice_(HandleBase* pHandle, FileDevice* pDevice) const
{
    pHandle->mOriginalDevice = pDevice;
}

}  // namespace sead
