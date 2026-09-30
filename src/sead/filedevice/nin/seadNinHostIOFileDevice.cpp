#include <filedevice/nin/seadNinHostIOFileDevice.h>

namespace sead {
/** Constructs the hostio device using its standard mount name. */
NinHostIOFileDevice::NinHostIOFileDevice() : NinFileDeviceBase("hostio", "hostio") {}

/** @return False: host I/O is unavailable in this release build. */
bool NinHostIOFileDevice::doIsAvailable_() const { return false; }

/**
 * Rejects host I/O path conversion in this release build.
 * @param pOut Unused destination buffer.
 * @param rPath Unused input path.
 * @return Always false.
 */
bool NinHostIOFileDevice::formatPathForFS_(BufferedSafeString* pOut, const SafeString& rPath) const
{
    return false;
}

}  // namespace sead
