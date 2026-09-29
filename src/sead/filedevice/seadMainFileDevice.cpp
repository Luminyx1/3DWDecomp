#include <filedevice/seadMainFileDevice.h>
#include <prim/seadSafeString.h>

#ifdef cafe
#include <filedevice/cafe/seadCafeFSAFileDeviceCafe.h>
#elif defined(NNSDK)
#include <filedevice/nin/seadNinContentFileDeviceNin.h>
#endif

namespace sead
{
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

MainFileDevice::~MainFileDevice()
{
    if (mFileDevice == NULL)
    {
        return;
    }

    delete mFileDevice;
    mFileDevice = NULL;
}

void MainFileDevice::traceFilePath(const SafeString& rPath) const
{
    mFileDevice->traceFilePath(rPath);
}

void MainFileDevice::traceDirectoryPath(const SafeString& rPath) const
{
    mFileDevice->traceDirectoryPath(rPath);
}

void MainFileDevice::resolveFilePath(BufferedSafeString* pOut, const SafeString& rPath) const
{
    mFileDevice->resolveFilePath(pOut, rPath);
}

void MainFileDevice::resolveDirectoryPath(BufferedSafeString* pOut, const SafeString& rPath) const
{
    mFileDevice->resolveDirectoryPath(pOut, rPath);
}
}  // namespace sead
