#include <nn/gfx/detail/gfx_Sampler-api.nvn.8.h>

#include <nn/gfx/detail/gfx_Device-api.nvn.8.h>
#include <nn/gfx/detail/gfx_NvnHelper.h>
#include <nn/gfx/detail/gfx_SamplerInfo-api.nvn.8.h>
#include <nn/gfx/gfx_SamplerInfo.h>
#include <nvn/nvn_FuncPtrInline.h>

namespace nn::gfx::detail {

typedef SamplerImpl<ApiVariationNvn8> SamplerImplNvn8;

/**
 * Constructs an uninitialized sampler, clearing all of its data.
 */
SamplerImplNvn8::SamplerImpl() {
    state = State_NotInitialized;
}

/**
 * Destroys the sampler object. Finalize must have been called beforehand.
 */
SamplerImplNvn8::~SamplerImpl() {}

/**
 * Initializes the sampler from a gfx sampler description.
 *
 * Min/max reduction filtering is only kept when the device supports it; otherwise the
 * reduction mode is forced back to averaging.
 *
 * @param pDevice Device to create the sampler on.
 * @param rInfo Sampler description.
 */
void SamplerImplNvn8::Initialize(DeviceImpl<ApiVariationNvn8>* pDevice, const InfoType& rInfo) {
    NVNsamplerBuilder builder;
    nvnSamplerBuilderSetDevice(&builder, pDevice->ToData()->pNvnDevice);
    Nvn::ConvertToNvnSamplerBuilder(&builder, rInfo);

    if (!pDevice->ToData()->supportedFeatures.GetBit(NvnDeviceFeature_SupportMinMaxFiltering)) {
        nvnSamplerBuilderSetReductionFilter(&builder, NVN_SAMPLER_REDUCTION_AVERAGE);
    }

    pNvnSampler = nvnSampler;
    nvnSamplerInitialize(pNvnSampler, &builder);

    flags.SetBit(Flag_Shared, false);
    state = State_Initialized;
}

/**
 * Initializes the sampler from an already configured NVN sampler builder.
 *
 * @param pDevice Device to create the sampler on.
 * @param rInfo NVN-native sampler description.
 */
void SamplerImplNvn8::Initialize(DeviceImpl<ApiVariationNvn8>* pDevice,
                                 const SamplerInfoImpl<ApiVariationNvn8>& rInfo) {
    NVNsamplerBuilder builder = rInfo.nvnSamplerBuilder;
    nvnSamplerBuilderSetDevice(&builder, pDevice->ToData()->pNvnDevice);

    if (!pDevice->ToData()->supportedFeatures.GetBit(NvnDeviceFeature_SupportMinMaxFiltering)) {
        nvnSamplerBuilderSetReductionFilter(&builder, NVN_SAMPLER_REDUCTION_AVERAGE);
    }

    pNvnSampler = nvnSampler;
    nvnSamplerInitialize(pNvnSampler, &builder);

    flags.SetBit(Flag_Shared, false);
    state = State_Initialized;
}

/**
 * Finalizes the sampler, releasing the underlying NVN sampler.
 *
 * @param pDevice Device the sampler was created on (unused).
 */
void SamplerImplNvn8::Finalize(DeviceImpl<ApiVariationNvn8>* pDevice) {
    nvnSamplerFinalize(pNvnSampler);
    pNvnSampler = nullptr;
    state = State_NotInitialized;
}

/**
 * Sets a debug label on the underlying NVN sampler.
 *
 * @param pDevice Device the sampler was created on (unused).
 * @param pLabel Label string.
 */
void SamplerImplNvn8::SetDebugLabel(DeviceImpl<ApiVariationNvn8>* pDevice, const char* pLabel) {
    nvnSamplerSetDebugLabel(pNvnSampler, pLabel);
}

}  // namespace nn::gfx::detail
