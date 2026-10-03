#pragma once

#include <nn/gfx/gfx_Enum.h>
#include <nn/gfx/gfx_GpuAddress.h>
#include <nn/types.h>
#include <nn/util/util_BitPack.h>
#include <nvn/nvn.h>
#include <nvnTool/nvnTool_GlslcInterface.h>

namespace nn::gfx {

struct ImageFormatProperty;
class TextureInfo;
class SwapChainInfo;
class QueueInfo;
class MemoryPoolInfo;
class BufferInfo;
class SamplerInfo;

namespace detail {

/// Bit positions of the features reported by Nvn::GetDeviceFeature.
enum NvnDeviceFeature {
    NvnDeviceFeature_SupportMinMaxFiltering,
    NvnDeviceFeature_SupportStencil8Format,
    NvnDeviceFeature_SupportAstcFormat,
    NvnDeviceFeature_SupportConservativeRaster,
    NvnDeviceFeature_SupportZeroFromUnmappedVirtualPoolPage,
    NvnDeviceFeature_SupportPassthroughGeometryShader,
    NvnDeviceFeature_SupportViewportSwizzle,
    NvnDeviceFeature_SupportSparseTiledPackagedTexture,
    NvnDeviceFeature_AdvancedBlendModes,
    NvnDeviceFeature_DrawTexture,
    NvnDeviceFeature_TargetIndependentRasterization,
    NvnDeviceFeature_FragmentCoverageToColor,
    NvnDeviceFeature_PostDepthCoverage,
    NvnDeviceFeature_ImagesUsingTextureHandles,
    NvnDeviceFeature_SampleLocations,
    NvnDeviceFeature_SupportFragmentShaderInterlock,
    NvnDeviceFeature_SupportsDebugLayer
};

/// Conversion helpers between nn::gfx and NVN types.
class Nvn {
public:
    static void CheckRequiredVersion(int majorVersion, int minorVersion);
    static NVNformat GetImageFormat(ImageFormat format);
    static NVNformat GetAttributeFormat(AttributeFormat format);
    static NVNtextureTarget GetImageTarget(ImageDimension dimension);
    static NVNdepthFunc GetDepthFunction(ComparisonFunction compare);
    static NVNstencilOp GetStencilOperation(StencilOperation operation);
    static NVNstencilFunc GetStencilFunction(ComparisonFunction compare);
    static NVNblendEquation GetBlendEquation(BlendFunction function);
    static NVNblendFunc GetBlendFunction(BlendFactor factor);
    static NVNlogicOp GetLogicOperation(LogicOperation operation);
    static NVNfrontFace GetFrontFace(FrontFace frontFace);
    static NVNpolygonMode GetFillMode(FillMode fillMode);
    static NVNdrawPrimitive GetDrawPrimitive(PrimitiveTopology topology);
    static NVNminFilter GetMinFilter(FilterMode filterMode);
    static NVNmagFilter GetMagFilter(FilterMode filterMode);
    static NVNwrapMode GetWrapMode(TextureAddressMode addressMode);
    static NVNshaderStage GetShaderStage(ShaderStage stage);
    static int GetShaderStageBits(int shaderStageBits);
    static NVNcompareFunc GetRComparisonFunction(ComparisonFunction compare);
    static NVNindexType GetIndexFormat(IndexFormat format);
    static NVNface GetCullMode(CullMode cullMode);
    static int GetMemoryPoolFlags(int memoryPoolProperty);
    static NVNbufferAddress GetBufferAddress(GpuAddress gpuAddress);
    static void SetupScanBufferTextureInfo(TextureInfo* pOutInfo, const SwapChainInfo& rInfo);
    static NVNcounterType GetCounterType(QueryTarget target);
    static NVNsamplerReduction GetSamplerReduction(FilterMode filterMode);
    static const float* GetBorderColor(TextureBorderColorType borderColorType);
    static nn::util::BitPack32 GetDeviceFeature(const NVNdevice* pDevice);
    static void GetImageFormatProperty(ImageFormatProperty* pOutProperty, NVNformat nvnFormat);
    static ImageFormat GetGfxImageFormat(NVNformat nvnFormat);
    static void ConvertToNvnQueueBuilder(NVNqueueBuilder* pBuilder, const QueueInfo& rInfo);
    static void ConvertToNvnMemoryPoolBuilder(NVNmemoryPoolBuilder* pBuilder,
                                              const MemoryPoolInfo& rInfo);
    static void ConvertToNvnBufferBuilder(NVNbufferBuilder* pBuilder, const BufferInfo& rInfo);
    static void ConvertToNvnTextureBuilder(NVNtextureBuilder* pBuilder, const TextureInfo& rInfo);
    static void ConvertToNvnSamplerBuilder(NVNsamplerBuilder* pBuilder, const SamplerInfo& rInfo);
    static void DebugCallback(NVNdebugCallbackSource source, NVNdebugCallbackType type, int id,
                              NVNdebugCallbackSeverity severity, const char* pMessage,
                              void* pUserParam);
};

/// Entry points of the GLSLC shader compiler library.
class GlslcDll {
public:
    typedef bool (*GlslcCompilePreSpecializedType)(GLSLCcompileObject*);
    typedef const GLSLCoutput* const* (*GlslcCompileSpecializedType)(
        GLSLCcompileObject*, const GLSLCspecializationBatch*);
    typedef uint8_t (*GlslcInitializeType)(GLSLCcompileObject*);
    typedef void (*GlslcFinalizeType)(GLSLCcompileObject*);
    typedef uint8_t (*GlslcCompileType)(GLSLCcompileObject*);
    typedef GLSLCversion (*GlslcGetVersionType)();
    typedef void (*GlslcSetAllocatorType)(GLSLCallocateFunction, GLSLCfreeFunction,
                                          GLSLCreallocateFunction, void*);
    typedef GLSLCoptions (*GlslcGetDefaultOptionsType)();

    static GlslcDll& GetInstance();

    GlslcDll();
    bool Initialize();
    void Finalize();
    bool IsInitialized() const;

    GlslcCompilePreSpecializedType GlslcCompilePreSpecialized;
    GlslcCompileSpecializedType GlslcCompileSpecialized;
    GlslcInitializeType GlslcInitialize;
    GlslcFinalizeType GlslcFinalize;
    GlslcCompileType GlslcCompile;
    GlslcGetVersionType GlslcGetVersion;
    GlslcSetAllocatorType GlslcSetAllocator;
    GlslcGetDefaultOptionsType GlslcGetDefaultOptions;

private:
    GlslcDll(const GlslcDll&) = delete;
    GlslcDll& operator=(const GlslcDll&) = delete;

    void* m_hDll;
};

}  // namespace detail

}  // namespace nn::gfx
