#include <nn/gfx/detail/gfx_Device-api.nvn.8.h>

#include <nn/gfx/detail/gfx_CommonHelper.h>
#include <nn/gfx/detail/gfx_NvnHelper.h>
#include <nn/gfx/gfx_DeviceInfo.h>
#include <nvn/nvn_FuncPtrInline.h>

namespace nn::gfx::detail {

typedef DeviceImpl<ApiVariationNvn8> DeviceImplNvn8;

/**
 * Constructs an uninitialized device, clearing all of its data.
 */
DeviceImplNvn8::DeviceImpl() {
    state = State_NotInitialized;
}

/**
 * Destroys the device object. Finalize must have been called beforehand.
 */
DeviceImplNvn8::~DeviceImpl() {}

/**
 * Initializes the NVN device.
 *
 * Bootstraps the NVN C entry points, checks that the driver provides the required API version,
 * creates the NVN device and then reloads the entry points for that device.
 *
 * @param rInfo Device description (unused on NVN).
 */
void DeviceImplNvn8::Initialize(const InfoType& rInfo) {
    UseMiddleWare();

    PFNNVNDEVICEGETPROCADDRESSPROC getProcAddress =
        reinterpret_cast<PFNNVNDEVICEGETPROCADDRESSPROC>(
            nvnBootstrapLoader("nvnDeviceGetProcAddress"));

    nvnLoadCProcs(nullptr, getProcAddress);
    Nvn::CheckRequiredVersion(53, 313);

    int deviceFlags = 0;

    NVNdeviceBuilder builder;
    nvnDeviceBuilderSetDefaults(&builder);
    nvnDeviceBuilderSetFlags(&builder, deviceFlags);

    pNvnDevice = nvnDevice;
    nvnDeviceInitialize(static_cast<NVNdevice*>(pNvnDevice.ptr), &builder);
    nvnLoadCProcs(static_cast<NVNdevice*>(pNvnDevice.ptr), getProcAddress);

    supportedFeatures = Nvn::GetDeviceFeature(static_cast<NVNdevice*>(pNvnDevice.ptr));
    flags.SetBit(Flag_Shared, false);
    state = State_Initialized;
}

/**
 * Finalizes the NVN device.
 */
void DeviceImplNvn8::Finalize() {
    nvnDeviceFinalize(static_cast<NVNdevice*>(pNvnDevice.ptr));
    pNvnDevice = nullptr;
    state = State_NotInitialized;
}

}  // namespace nn::gfx::detail
