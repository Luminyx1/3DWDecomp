#include <nn/gfx/detail/gfx_ResShaderImpl.h>

#include <algorithm>
#include <attributes.h>

#include <nn/gfx/detail/gfx_Device-api.nvn.8.h>
#include <nn/gfx/detail/gfx_MemoryPool-api.nvn.8.h>
#include <nn/gfx/detail/gfx_NvnHelper.h>
#include <nn/gfx/gfx_Device.h>
#include <nn/gfx/gfx_MemoryPoolInfo.h>
#include <nn/gfx/gfx_ResShader.h>
#include <nn/gfx/gfx_ResShaderData-api.nvn.h>
#include <nn/util/util_BytePtr.h>
#include <nvn/nvn_FuncPtrInline.h>

namespace nn::gfx {
namespace detail {

typedef MemoryPoolImpl<ApiVariationNvn8> MemoryPoolImplNvn8;
typedef DeviceImpl<ApiVariationNvn8> DeviceImplNvn8;

/**
 * Rounds a shader scratch memory requirement up to the device's scratch memory granularity.
 *
 * When no stage reports a recommended size, the per-warp size is scaled by the device's
 * recommended scale factor instead.
 *
 * @param pDevice Device to query.
 * @param maxSizeRecommended Largest recommended scratch size over all stages.
 * @param maxSizePerWarp Largest per-warp scratch size over all stages.
 * @return Scratch memory size in bytes, or 0 when no scratch memory is needed.
 */
static ALWAYS_INLINE size_t CalculateScratchMemorySize(DeviceImplNvn8* pDevice,
                                                       uint32_t maxSizeRecommended,
                                                       uint32_t maxSizePerWarp) {
    uint32_t maxSize = 0;

    if (maxSizeRecommended != 0) {
        maxSize = maxSizeRecommended;
    } else if (maxSizePerWarp != 0) {
        int scaleFactor;
        nvnDeviceGetInteger(pDevice->ToData()->pNvnDevice,
                            NVN_DEVICE_INFO_SHADER_SCRATCH_MEMORY_SCALE_FACTOR_RECOMMENDED,
                            &scaleFactor);
        maxSize = scaleFactor * maxSizePerWarp;
    } else {
        return maxSize;
    }

    int granularity;
    nvnDeviceGetInteger(pDevice->ToData()->pNvnDevice,
                        NVN_DEVICE_INFO_SHADER_SCRATCH_MEMORY_GRANULARITY, &granularity);
    return (maxSize + granularity - 1) / granularity * granularity;
}

/**
 * Computes the recommended scratch memory size for a single shader program.
 *
 * @param pThis Shader program resource.
 * @param pDevice Device the program will run on.
 * @return Recommended scratch memory size in bytes, rounded up to the device granularity.
 */
template <>
size_t ResShaderProgramImpl::NvnGetRecommendedScrachMemorySize<ApiVariationNvn8>(
    const ResShaderProgram* pThis, DeviceImplNvn8* pDevice) {
    static Ptr<const void> ShaderInfoData::*s_pStageCodes[] = {
        &ShaderInfoData::pVertexShaderCode,   &ShaderInfoData::pHullShaderCode,
        &ShaderInfoData::pDomainShaderCode,   &ShaderInfoData::pGeometryShaderCode,
        &ShaderInfoData::pPixelShaderCode,    &ShaderInfoData::pComputeShaderCode,
    };

    const ResShaderProgram::value_type& data = pThis->ToData();
    uint32_t maxSizePerWarp = 0;
    uint32_t maxSizeRecommended = 0;

    for (int idxStage = 0; idxStage < ShaderStage_End; ++idxStage) {
        const NvnShaderCode* pNvnShaderData =
            static_cast<const NvnShaderCode*>(data.info.*s_pStageCodes[idxStage]);
        if (pNvnShaderData != nullptr) {
            maxSizeRecommended =
                std::max(maxSizeRecommended, pNvnShaderData->scratchMemoryRecommended);
            maxSizePerWarp = std::max(maxSizePerWarp, pNvnShaderData->scratchMemoryPerWarp);
        }
    }

    return CalculateScratchMemorySize(pDevice, maxSizeRecommended, maxSizePerWarp);
}

/**
 * Initializes a shader container, binding its shader binary pool to GPU memory and
 * resolving the GPU address of every binary shader stage.
 *
 * @param pThis Shader container resource.
 * @param pDevice Device to initialize on.
 * @param pMemoryPool External memory pool holding the container, or nullptr to create the
 *        container's internal memory pool.
 * @param memoryPoolOffset Offset of the container within pMemoryPool.
 * @param memoryPoolSize Size of the container's region in pMemoryPool (unused).
 */
template <>
void ResShaderContainerImpl::Initialize<ApiVariationNvn8>(ResShaderContainer* pThis,
                                                          DeviceImplNvn8* pDevice,
                                                          MemoryPoolImplNvn8* pMemoryPool,
                                                          ptrdiff_t memoryPoolOffset,
                                                          size_t memoryPoolSize) {
    ResShaderContainer::value_type& data = pThis->ToData();
    NvnShaderPool* pShaderPool = static_cast<NvnShaderPool*>(data.pShaderBinaryPool.Get());
    MemoryPoolInfo& memoryPoolInfo = DataToAccessor(pShaderPool->memoryPoolInfo);

    NVNbufferAddress headAddress;

    if (pMemoryPool != nullptr) {
        pShaderPool->pCurrentMemoryPool.Set(pMemoryPool);
        ptrdiff_t offset = memoryPoolOffset +
                           nn::util::BytePtr(pThis).Distance(pShaderPool->memoryPoolInfo.pMemory);
        headAddress = nvnMemoryPoolGetBufferAddress(pMemoryPool->ToData()->pNvnMemoryPool);
        headAddress += offset;
    } else {
        MemoryPoolImplNvn8* pInternalMemoryPool =
            static_cast<MemoryPoolImplNvn8*>(pShaderPool->pMemoryPool.Get());
        pInternalMemoryPool->Initialize(pDevice, memoryPoolInfo);
        pShaderPool->pCurrentMemoryPool.Set(pInternalMemoryPool);
        headAddress = nvnMemoryPoolGetBufferAddress(pInternalMemoryPool->ToData()->pNvnMemoryPool);
    }

    for (int idxVariation = 0, variationCount = pThis->GetShaderVariationCount();
         idxVariation < variationCount; ++idxVariation) {
        ResShaderVariation* pResShaderVariation = pThis->GetResShaderVariation(idxVariation);
        ResShaderProgram* pResShaderProgram =
            pResShaderVariation->GetResShaderProgram(ShaderCodeType_Binary);
        if (pResShaderProgram == nullptr) {
            continue;
        }

        ShaderInfo* pShaderInfo = pResShaderProgram->GetShaderInfo();
        for (int idxStage = 0; idxStage < 6; ++idxStage) {
            NvnShaderCode* pNvnShaderCode = const_cast<NvnShaderCode*>(
                static_cast<const NvnShaderCode*>(
                    pShaderInfo->GetShaderCodePtr(static_cast<ShaderStage>(idxStage))));
            if (pNvnShaderCode != nullptr) {
                pNvnShaderCode->dataAddress =
                    headAddress + nn::util::BytePtr(memoryPoolInfo.GetPoolMemory())
                                      .Distance(pNvnShaderCode->pData.Get());
            }
        }
    }

    pThis->ToData().targetCodeType = ShaderCodeType_Binary;
}

/**
 * Finalizes a shader container, releasing its internal memory pool if it was used.
 *
 * @param pThis Shader container resource.
 * @param pDevice Device the container was initialized on.
 */
template <>
void ResShaderContainerImpl::Finalize<ApiVariationNvn8>(ResShaderContainer* pThis,
                                                        DeviceImplNvn8* pDevice) {
    ResShaderContainer::value_type& data = pThis->ToData();
    NvnShaderPool* pShaderPool = static_cast<NvnShaderPool*>(data.pShaderBinaryPool.Get());

    MemoryPoolImplNvn8* pCurrentMemoryPool =
        static_cast<MemoryPoolImplNvn8*>(pShaderPool->pCurrentMemoryPool.Get());

    if (pCurrentMemoryPool == pShaderPool->pMemoryPool.Get()) {
        pCurrentMemoryPool->Finalize(pDevice);
    }

    pShaderPool->pCurrentMemoryPool.Set(nullptr);
}

}  // namespace detail

/**
 * Computes the largest recommended scratch memory size over every binary shader program in a
 * set of shader files.
 *
 * @param pDevice Device the shaders will run on.
 * @param ppResShaderFileArray Array of shader file resources.
 * @param shaderFileCount Number of entries in ppResShaderFileArray.
 * @return Recommended scratch memory size in bytes, rounded up to the device granularity.
 */
template <>
size_t NvnGetMaxRecommendedScratchMemorySize<ApiVariationNvn8>(
    TDevice<ApiVariationNvn8>* pDevice, const ResShaderFile* const* ppResShaderFileArray,
    int shaderFileCount) {
    static detail::Ptr<const void> ShaderInfoData::*s_pStageCodes[] = {
        &ShaderInfoData::pVertexShaderCode,   &ShaderInfoData::pHullShaderCode,
        &ShaderInfoData::pDomainShaderCode,   &ShaderInfoData::pGeometryShaderCode,
        &ShaderInfoData::pPixelShaderCode,    &ShaderInfoData::pComputeShaderCode,
    };

    uint32_t maxSizePerWarp = 0;
    uint32_t maxSizeRecommended = 0;

    for (int idxResShaderFile = 0; idxResShaderFile < shaderFileCount; ++idxResShaderFile) {
        const ResShaderContainer* pResShaderContainer =
            ppResShaderFileArray[idxResShaderFile]->GetShaderContainer();
        if (pResShaderContainer == nullptr) {
            continue;
        }

        for (int idxVariation = 0, variationCount = pResShaderContainer->GetShaderVariationCount();
             idxVariation < variationCount; ++idxVariation) {
            const ResShaderVariation* pResShaderVariation =
                pResShaderContainer->GetResShaderVariation(idxVariation);
            const ResShaderProgram* pResShaderProgram =
                pResShaderVariation->GetResShaderProgram(ShaderCodeType_Binary);
            if (pResShaderProgram == nullptr) {
                continue;
            }

            for (int idxStage = 0; idxStage < 6; ++idxStage) {
                const NvnShaderCode* pNvnShaderData = static_cast<const NvnShaderCode*>(
                    pResShaderProgram->ToData().info.*s_pStageCodes[idxStage]);
                if (pNvnShaderData != nullptr) {
                    maxSizeRecommended =
                        std::max(maxSizeRecommended, pNvnShaderData->scratchMemoryRecommended);
                    maxSizePerWarp =
                        std::max(maxSizePerWarp, pNvnShaderData->scratchMemoryPerWarp);
                }
            }
        }
    }

    return detail::CalculateScratchMemorySize(pDevice, maxSizeRecommended, maxSizePerWarp);
}

}  // namespace nn::gfx
