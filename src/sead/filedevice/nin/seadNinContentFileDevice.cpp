#include <filedevice/nin/seadNinContentFileDeviceNin.h>

namespace sead {
/** Constructs the content device using its standard mount name. */
NinContentFileDevice::NinContentFileDevice() : NinFileDeviceBase("content", "content") {}

}  // namespace sead
