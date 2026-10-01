#ifdef cafe
#include <cafe.h>
#include <nn/save.h>
#endif  // cafe

#ifdef NNSDK
#include <nn/fs/fs_mount.h>
#include <nn/fs/fs_rom.h>
#include <nn/fs/fs_save.h>
#endif

#include <basis/seadNew.h>
#include <basis/seadRawPrint.h>
#include <devenv/seadEnvUtil.h>
#include <filedevice/seadFileDeviceMgr.h>
#include <filedevice/seadPath.h>
#include <heap/seadHeapMgr.h>

namespace sead
{
SEAD_SINGLETON_DISPOSER_IMPL(FileDeviceMgr)

FileDeviceMgr::FileDeviceMgr()
{
    if (HeapMgr::sInstancePtr == NULL)
    {
        SEAD_ASSERT_MSG(false, "FileDeviceMgr need HeapMgr");
        return;
    }

    Heap* const heap = HeapMgr::instance()->findContainHeap(this);
    mount_(heap);

    mMainFileDevice = new (heap) MainFileDevice(heap);
    mount(mMainFileDevice);

    mDefaultFileDevice = mMainFileDevice;
}

FileDeviceMgr::~FileDeviceMgr()
{
    if (mMainFileDevice != NULL)
    {
        delete mMainFileDevice;
        mMainFileDevice = NULL;
    }

    unmount_();
}

void FileDeviceMgr::mount_([[maybe_unused]] Heap* pHeap)
{
#ifdef cafe
    FSInit();
    FSAddClient(&client, FS_RET_NO_ERROR);

    FSStateChangeParams changeParams = {
        .userCallback = stateChangeCallback_, .userContext = NULL, .ioMsgQueue = NULL};

    FSSetStateChangeNotification(&client, &changeParams);
    SAVEInit();
    _17A4[0] = 0;
    _1824 = 0;
#elif defined(NNSDK)
    // For release builds, only content is mounted using the regular nn::fs::MountRom.
    // For debug builds, content is mounted using nn::fs::MountRom or by mounting
    // SEAD_NIN_CONTENT_DIR on the host computer and the host root and SD are also mounted.
#ifdef SEAD_DEBUG
    const auto mount_host_result = nn::fs::MountHostRoot();

    if (mount_host_result.IsFailure())
    {
        SEAD_WARN("nn::fs::MountHostRoot() failed. module = %d desc = %d innervalue = 0x%08x",
                  mount_host_result.GetModule(), mount_host_result.GetDescription(),
                  mount_host_result.GetInnerValueForDebug());
        mMountedHost = false;
    }
    else
    {
        mMountedHost = true;
    }
#endif  // SEAD_DEBUG

#ifdef SEAD_DEBUG
    if (nn::fs::CanMountRomForDebug())
#endif
    {
        u64 cache_size = 0;
        const auto query_result = nn::fs::QueryMountRomCacheSize(&cache_size);
        SEAD_ASSERT_MSG(query_result.IsSuccess(),
                        "nn::fs::QueryMountRomCacheSize() failed. module = %d desc = %d "
                        "innervalue = 0x%08x",
                        query_result.GetModule(), query_result.GetDescription(),
                        query_result.GetInnerValueForDebug());

        SEAD_DEBUG_PRINT("FileDeviceMgr: MountRom cache size => %zd\n", cache_size);
        mRomCache = new (pHeap) u8[cache_size];

        const auto result = nn::fs::MountRom("content", mRomCache, cache_size);
        SEAD_ASSERT_MSG(result.IsSuccess(),
                        "nn::fs::MountRom() failed. module = %d desc = %d innervalue = 0x%08x",
                        result.GetModule(), result.GetDescription(),
                        result.GetInnerValueForDebug());
    }
#ifdef SEAD_DEBUG
    else
    {
        FixedSafeString<256> content_dir;

        if (EnvUtil::getEnvironmentVariable(&content_dir, "SEAD_NIN_CONTENT_DIR") == -1)
        {
            SEAD_WARN("SEAD_NIN_CONTENT_DIR is not set.");
        }
        else
        {
            const auto result = nn::fs::MountHost("content", content_dir.cstr());
            SEAD_ASSERT_MSG(result.IsSuccess(),
                            "nn::fs::MountHost() failed. module = %d desc = %d innervalue = 0x%08x",
                            result.GetModule(), result.GetDescription(),
                            result.GetInnerValueForDebug());
            system::Print("FileDeviceMgr: MountHost => %s\n", content_dir.cstr());
        }
    }

    const auto sd_result = nn::fs::MountSdCardForDebug("sd");
    mMountedSd = sd_result.IsSuccess();

    if (sd_result.IsSuccess())
    {
        system::Print("FileDeviceMgr: mount SD card\n");
    }
    else if (nn::fs::ResultMountNameAlreadyExists().Includes(sd_result))
    {
        system::Print("FileDeviceMgr: SD card already mounted\n");
    }
    else if (nn::fs::ResultSdCardAccessFailed().Includes(sd_result))
    {
        system::Print("FileDeviceMgr: SD card is not ready\n");
    }
#endif  // SEAD_DEBUG
#else
#error "Unknown platform"
#endif
}

void FileDeviceMgr::unmount_()
{
#ifdef cafe
    FSDelClient(&client, FS_RET_NO_ERROR);
    SAVEShutdown();
    FSShutdown();
#elif defined(NNSDK)
#ifdef SEAD_DEBUG
    if (mMountedHost)
    {
        nn::fs::UnmountHostRoot();
    }
#endif

    nn::fs::Unmount("content");

    if (mRomCache)
    {
        delete[] mRomCache;
    }

#ifdef SEAD_DEBUG
    if (mMountedSd)
    {
        nn::fs::Unmount("sd");
    }
#endif
#else
#error "Unknown platform"
#endif
}

void FileDeviceMgr::traceFilePath(const SafeString& rPath) const
{
    SEAD_DEBUG_PRINT("[FileDeviceMgr] %s\n", rPath.cstr());
    FixedSafeString<256> pathNoDrive;
    FileDevice* device = findDeviceFromPath(rPath, &pathNoDrive);

    if (device != NULL)
    {
        device->traceFilePath(pathNoDrive);
    }
    else
    {
        SEAD_WARN("FileDevice not found: %s", rPath.cstr());
    }
}

void FileDeviceMgr::traceDirectoryPath(const SafeString& rPath) const
{
    SEAD_DEBUG_PRINT("[FileDeviceMgr] %s\n", rPath.cstr());
    FixedSafeString<256> pathNoDrive;
    FileDevice* device = findDeviceFromPath(rPath, &pathNoDrive);

    if (device != NULL)
    {
        device->traceDirectoryPath(pathNoDrive);
    }
    else
    {
        SEAD_WARN("FileDevice not found: %s", rPath.cstr());
    }
}

void FileDeviceMgr::resolveFilePath(BufferedSafeString* pOut, const SafeString& rPath) const
{
    FixedSafeString<256> pathNoDrive;
    FileDevice* device = findDeviceFromPath(rPath, &pathNoDrive);

    if (device != NULL)
    {
        device->resolveFilePath(pOut, pathNoDrive);
    }
    else
    {
        SEAD_WARN("FileDevice not found: %s", rPath.cstr());
    }
}

void FileDeviceMgr::resolveDirectoryPath(BufferedSafeString* pOut, const SafeString& rPath) const
{
    FixedSafeString<256> pathNoDrive;
    FileDevice* device = findDeviceFromPath(rPath, &pathNoDrive);

    if (device != NULL)
    {
        device->resolveDirectoryPath(pOut, pathNoDrive);
    }
    else
    {
        SEAD_WARN("FileDevice not found: %s", rPath.cstr());
    }
}

void FileDeviceMgr::mount(FileDevice* pDevice, const SafeString& rName)
{
    if (!rName.isEqual(SafeString::cEmptyString))
    {
        pDevice->setDriveName(rName);
    }

    mDeviceList.pushBack(pDevice);
}

void FileDeviceMgr::unmount(FileDevice* pDevice)
{
    mDeviceList.erase(pDevice);

    if (pDevice == mDefaultFileDevice)
    {
        mDefaultFileDevice = NULL;
    }
}

void FileDeviceMgr::unmount(const SafeString& rName)
{
    auto* device = findDevice(rName);

    if (device == nullptr)
    {
        SEAD_ASSERT_MSG(false, "drive not found: %s\n", rName.cstr());
        return;
    }

    unmount(device);
}

FileDevice* FileDeviceMgr::findDeviceFromPath(const SafeString& rPath,
                                              BufferedSafeString* pathNoDrive) const
{
    FixedSafeString<32> driveName;
    FileDevice* device;

    if (!Path::getDriveName(&driveName, rPath))
    {
        device = mDefaultFileDevice;

        if (device == nullptr)
        {
            SEAD_ASSERT_MSG(false, "drive name not found and default file device is null");
            return nullptr;
        }
    }
    else
    {
        device = findDevice(driveName);
    }

    if (device == nullptr)
    {
        return nullptr;
    }

    if (pathNoDrive != NULL)
    {
        Path::getPathExceptDrive(pathNoDrive, rPath);
    }

    return device;
}

FileDevice* FileDeviceMgr::findDevice(const SafeString& rName) const
{
    for (auto it = mDeviceList.begin(); it != mDeviceList.end(); ++it)
    {
        if ((*it)->getDriveName() == rName)
        {
            return *it;
        }
    }

    return nullptr;
}

FileDevice* FileDeviceMgr::tryOpen(FileHandle* pHandle, const SafeString& rPath,
                                   FileDevice::FileOpenFlag flag, u32 divSize)
{
    FixedSafeString<256> pathNoDrive;
    FileDevice* device = findDeviceFromPath(rPath, &pathNoDrive);

    if (device == NULL)
    {
        return NULL;
    }

    return device->tryOpen(pHandle, pathNoDrive, flag, divSize);
}

FileDevice* FileDeviceMgr::tryOpenDirectory(DirectoryHandle* pHandle, const SafeString& rPath)
{
    FixedSafeString<256> pathNoDrive;
    FileDevice* device = findDeviceFromPath(rPath, &pathNoDrive);

    if (device == nullptr)
    {
        return nullptr;
    }

    if (!device->isExistDirectory(pathNoDrive))
    {
        return nullptr;
    }

    return device->tryOpenDirectory(pHandle, pathNoDrive);
}

u8* FileDeviceMgr::tryLoad(FileDevice::LoadArg& rArg)
{
    SEAD_ASSERT_MSG(rArg.path != SafeString::cEmptyString, "path is null");

    FixedSafeString<256> pathNoDrive;
    FileDevice* device = findDeviceFromPath(rArg.path, &pathNoDrive);

    if (device == NULL)
    {
        return NULL;
    }

    FileDevice::LoadArg arg2(rArg);
    arg2.path = pathNoDrive.cstr();

    u8* data = device->tryLoad(arg2);

    rArg.read_size = arg2.read_size;
    rArg.roundup_size = arg2.roundup_size;
    rArg.need_unload = arg2.need_unload;

    return data;
}

void FileDeviceMgr::unload(u8* pData)
{
    SEAD_ASSERT(pData);

    if (pData)
    {
        delete pData;
    }
}

bool FileDeviceMgr::trySave(FileDevice::SaveArg& rArg)
{
    SEAD_ASSERT_MSG(rArg.path != SafeString::cEmptyString, "path is null");

    FixedSafeString<256> pathNoDrive;
    FileDevice* device = findDeviceFromPath(rArg.path, &pathNoDrive);

    if (device == nullptr)
    {
        return false;
    }

    FileDevice::SaveArg arg2(rArg);
    arg2.path = pathNoDrive.cstr();

    const bool ret = device->trySave(arg2);
    rArg.write_size = arg2.write_size;
    return ret;
}

#ifdef NNSDK
void FileDeviceMgr::mountSaveDataForDebug(Heap*)
{
#ifdef SEAD_DEBUG
    const auto result = nn::fs::MountSaveDataForDebug("save");
    SEAD_ASSERT_MSG(
        result.IsSuccess(),
        "nn::fs::MountSaveDataForDebug() failed. module = %d desc = %d innervalue = 0x%08x",
        result.GetModule(), result.GetDescription(), result.GetInnerValueForDebug());
#endif
}

void FileDeviceMgr::unmountSaveDataForDebug()
{
#ifdef SEAD_DEBUG
    nn::fs::Unmount("save");
#endif
}
#endif

#ifdef cafe
void FileDeviceMgr::stateChangeCallback_(FSClient* pClient, FSVolumeState state, void* pContext)
{
    FSGetLastError(pClient);
}
#endif  // cafe

}  // namespace sead
