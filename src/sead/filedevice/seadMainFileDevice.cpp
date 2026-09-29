#include <filedevice/seadMainFileDevice.h>
#include <prim/seadSafeString.h>

#ifdef cafe
#include <filedevice/cafe/seadCafeFSAFileDeviceCafe.h>
#elif defined(NNSDK)
#include <filedevice/nin/seadNinContentFileDeviceNin.h>
#endif

namespace sead
{
/**
 * Constructs the device and the platform content device it forwards to.
 * @param pHeap Heap the platform device is allocated from.
 */
MainFileDevice::MainFileDevice(Heap* pHeap) : FileDevice("main"), mFileDevice(nullptr)
{
#ifdef cafe
    mFileDevice = new (pHeap, 4) CafeContentFileDevice();
#elif defined(NNSDK)
    mFileDevice = new (pHeap, 8) NinContentFileDevice();
#else
#error "Unknown platform"
#endif
    SEAD_ASSERT(mFileDevice);
}

/**
 * Destroys the platform content device.
 */
MainFileDevice::~MainFileDevice()
{
    if (mFileDevice == nullptr)
    {
        return;
    }

    delete mFileDevice;
    mFileDevice = nullptr;
}

/**
 * Prints a file path through the platform device.
 * @param rPath Path to trace.
 */
void MainFileDevice::traceFilePath(const SafeString& rPath) const
{
    mFileDevice->traceFilePath(rPath);
}

/**
 * Prints a directory path through the platform device.
 * @param rPath Path to trace.
 */
void MainFileDevice::traceDirectoryPath(const SafeString& rPath) const
{
    mFileDevice->traceDirectoryPath(rPath);
}

/**
 * Resolves a file path through the platform device.
 * @param pOut Receives the resolved path.
 * @param rPath Path to resolve.
 */
void MainFileDevice::resolveFilePath(BufferedSafeString* pOut, const SafeString& rPath) const
{
    mFileDevice->resolveFilePath(pOut, rPath);
}

/**
 * Resolves a directory path through the platform device.
 * @param pOut Receives the resolved path.
 * @param rPath Path to resolve.
 */
void MainFileDevice::resolveDirectoryPath(BufferedSafeString* pOut, const SafeString& rPath) const
{
    mFileDevice->resolveDirectoryPath(pOut, rPath);
}
}  // namespace sead
