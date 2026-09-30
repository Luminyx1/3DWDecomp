#include <filedevice/nin/seadNinSaveFileDeviceNin.h>
#include <nn/fs.h>

namespace sead {
/**
 * Constructs the save device for a mounted save filesystem.
 * @param rMount Filesystem mount name used for operations and commits.
 */
NinSaveFileDevice::NinSaveFileDevice(const SafeString& rMount) : NinFileDeviceBase("save", rMount) {}

/**
 * Commits pending filesystem changes and records the result as the last error.
 * @return Whether the commit succeeded.
 */
bool NinSaveFileDevice::tryCommit()
{
    mLastError = nn::fs::Commit(mMountPoint.cstr());
    return mLastError.IsSuccess();
}
}  // namespace sead
