#include <nn/gfx/detail/gfx_Texture-api.nvn.8.h>

#include <nn/gfx/detail/gfx_CommonHelper.h>
#include <nn/gfx/detail/gfx_Device-api.nvn.8.h>
#include <nn/gfx/detail/gfx_MemoryPool-api.nvn.8.h>
#include <nn/gfx/detail/gfx_NvnHelper.h>
#include <nn/gfx/detail/gfx_TextureInfo-api.nvn.8.h>
#include <nn/gfx/gfx_TextureInfo.h>
#include <nn/util/util_BitUtil.h>
#include <nvn/nvn_FuncPtrInline.h>

namespace nn::gfx::detail {

typedef TextureImpl<ApiVariationNvn8> TextureImplNvn8;
typedef TextureViewImpl<ApiVariationNvn8> TextureViewImplNvn8;
typedef ColorTargetViewImpl<ApiVariationNvn8> ColorTargetViewImplNvn8;
typedef DepthStencilViewImpl<ApiVariationNvn8> DepthStencilViewImplNvn8;
typedef TextureInfoImpl<ApiVariationNvn8> TextureInfoImplNvn8;

namespace {

/**
 * Extracts the channel format part of an image format.
 *
 * @param format Image format.
 * @return Channel format of the image format.
 */
inline ChannelFormat GetChannelFormat(ImageFormat format) {
    return static_cast<ChannelFormat>(format >> TypeFormat_Bits);
}

/**
 * Computes the byte offset of every mip level of a texture by creating a temporary texture in
 * a virtual memory pool and querying the offsets of single-level views.
 *
 * @param pMipOffsets Receives one offset per mip level.
 * @param pDevice NVN device.
 * @param rBuilder Fully configured texture builder describing the texture.
 */
void CalculateMipDataOffsetsImpl(ptrdiff_t* pMipOffsets, NVNdevice* pDevice,
                                 const NVNtextureBuilder& rBuilder) {
    int pageSize;
    nvnDeviceGetInteger(pDevice, NVN_DEVICE_INFO_MEMORY_POOL_PAGE_SIZE, &pageSize);

    NVNtextureBuilder builder = rBuilder;
    nvnTextureBuilderSetDevice(&builder, pDevice);
    size_t size = nvnTextureBuilderGetStorageSize(&builder);

    NVNmemoryPoolBuilder memoryPoolBuilder;
    nvnMemoryPoolBuilderSetDefaults(&memoryPoolBuilder);
    nvnMemoryPoolBuilderSetDevice(&memoryPoolBuilder, pDevice);
    nvnMemoryPoolBuilderSetFlags(&memoryPoolBuilder,
                                 NVN_MEMORY_POOL_FLAGS_CPU_NO_ACCESS |
                                     NVN_MEMORY_POOL_FLAGS_GPU_CACHED |
                                     NVN_MEMORY_POOL_FLAGS_COMPRESSIBLE |
                                     NVN_MEMORY_POOL_FLAGS_VIRTUAL);
    nvnMemoryPoolBuilderSetStorage(&memoryPoolBuilder, nullptr,
                                   nn::util::align_up(size, pageSize));

    NVNmemoryPool memoryPool;
    nvnMemoryPoolInitialize(&memoryPool, &memoryPoolBuilder);
    nvnTextureBuilderSetStorage(&builder, &memoryPool, 0);

    NVNtexture texture;
    nvnTextureInitialize(&texture, &builder);

    int mipCount = nvnTextureBuilderGetLevels(&builder);

    NVNtextureView textureView;
    nvnTextureViewSetDefaults(&textureView);

    for (int mipLevel = 0; mipLevel < mipCount; ++mipLevel) {
        nvnTextureViewSetLevels(&textureView, mipLevel, 1);
        pMipOffsets[mipLevel] = nvnTextureGetViewOffset(&texture, &textureView);
    }

    nvnTextureFinalize(&texture);
    nvnMemoryPoolFinalize(&memoryPool);
}

}  // namespace

/**
 * Returns the alignment required for the mip data storage of a texture.
 *
 * @param pDevice Device the texture will be created on.
 * @param rInfo Texture description.
 * @return Storage alignment in bytes.
 */
size_t TextureImplNvn8::CalculateMipDataAlignment(DeviceImpl<ApiVariationNvn8>* pDevice,
                                                  const InfoType& rInfo) {
    NVNtextureBuilder builder;
    nvnTextureBuilderSetDevice(&builder, pDevice->ToData()->pNvnDevice);
    Nvn::ConvertToNvnTextureBuilder(&builder, rInfo);
    return nvnTextureBuilderGetStorageAlignment(&builder);
}

/**
 * Returns the alignment required for the mip data storage of a texture.
 *
 * @param pDevice Device the texture will be created on.
 * @param rInfo NVN-native texture description.
 * @return Storage alignment in bytes.
 */
size_t TextureImplNvn8::CalculateMipDataAlignment(DeviceImpl<ApiVariationNvn8>* pDevice,
                                                  const TextureInfoImplNvn8& rInfo) {
    NVNtextureBuilder* pBuilder = const_cast<NVNtextureBuilder*>(&rInfo.nvnTextureBuilder);
    nvnTextureBuilderSetDevice(pBuilder, pDevice->ToData()->pNvnDevice);
    return nvnTextureBuilderGetStorageAlignment(pBuilder);
}

/**
 * Returns the size of the mip data storage of a texture.
 *
 * @param pDevice Device the texture will be created on.
 * @param rInfo Texture description.
 * @return Storage size in bytes.
 */
size_t TextureImplNvn8::CalculateMipDataSize(DeviceImpl<ApiVariationNvn8>* pDevice,
                                             const InfoType& rInfo) {
    NVNtextureBuilder builder;
    nvnTextureBuilderSetDevice(&builder, pDevice->ToData()->pNvnDevice);
    Nvn::ConvertToNvnTextureBuilder(&builder, rInfo);
    return nvnTextureBuilderGetStorageSize(&builder);
}

/**
 * Returns the size of the mip data storage of a texture.
 *
 * @param pDevice Device the texture will be created on.
 * @param rInfo NVN-native texture description.
 * @return Storage size in bytes.
 */
size_t TextureImplNvn8::CalculateMipDataSize(DeviceImpl<ApiVariationNvn8>* pDevice,
                                             const TextureInfoImplNvn8& rInfo) {
    NVNtextureBuilder* pBuilder = const_cast<NVNtextureBuilder*>(&rInfo.nvnTextureBuilder);
    nvnTextureBuilderSetDevice(pBuilder, pDevice->ToData()->pNvnDevice);
    return nvnTextureBuilderGetStorageSize(pBuilder);
}

/**
 * Returns both the size and the alignment of the mip data storage of a texture.
 *
 * @param pOutSize Receives the storage size in bytes.
 * @param pOutAlignment Receives the storage alignment in bytes.
 * @param pDevice Device the texture will be created on.
 * @param rInfo Texture description.
 */
void TextureImplNvn8::CalculateMipDataSizeAndAlignment(size_t* pOutSize, size_t* pOutAlignment,
                                                       DeviceImpl<ApiVariationNvn8>* pDevice,
                                                       const InfoType& rInfo) {
    NVNtextureBuilder builder;
    nvnTextureBuilderSetDevice(&builder, pDevice->ToData()->pNvnDevice);
    Nvn::ConvertToNvnTextureBuilder(&builder, rInfo);
    *pOutSize = nvnTextureBuilderGetStorageSize(&builder);
    *pOutAlignment = nvnTextureBuilderGetStorageAlignment(&builder);
}

/**
 * Returns both the size and the alignment of the mip data storage of a texture.
 *
 * @param pOutSize Receives the storage size in bytes.
 * @param pOutAlignment Receives the storage alignment in bytes.
 * @param pDevice Device the texture will be created on.
 * @param rInfo NVN-native texture description.
 */
void TextureImplNvn8::CalculateMipDataSizeAndAlignment(size_t* pOutSize, size_t* pOutAlignment,
                                                       DeviceImpl<ApiVariationNvn8>* pDevice,
                                                       const TextureInfoImplNvn8& rInfo) {
    NVNtextureBuilder* pBuilder = const_cast<NVNtextureBuilder*>(&rInfo.nvnTextureBuilder);
    nvnTextureBuilderSetDevice(pBuilder, pDevice->ToData()->pNvnDevice);
    *pOutSize = nvnTextureBuilderGetStorageSize(pBuilder);
    *pOutAlignment = nvnTextureBuilderGetStorageAlignment(pBuilder);
}

/**
 * Computes the byte offset of every mip level within the texture storage.
 *
 * @param pMipOffsets Receives one offset per mip level.
 * @param pDevice Device the texture will be created on.
 * @param rInfo Texture description.
 */
void TextureImplNvn8::CalculateMipDataOffsets(ptrdiff_t* pMipOffsets,
                                              DeviceImpl<ApiVariationNvn8>* pDevice,
                                              const InfoType& rInfo) {
    NVNdevice* pNvnDevice = pDevice->ToData()->pNvnDevice;

    NVNtextureBuilder builder;
    nvnTextureBuilderSetDevice(&builder, pNvnDevice);
    Nvn::ConvertToNvnTextureBuilder(&builder, rInfo);
    CalculateMipDataOffsetsImpl(pMipOffsets, pNvnDevice, builder);
}

/**
 * Computes the byte offset of every mip level within the texture storage.
 *
 * @param pMipOffsets Receives one offset per mip level.
 * @param pDevice Device the texture will be created on.
 * @param rInfo NVN-native texture description.
 */
void TextureImplNvn8::CalculateMipDataOffsets(ptrdiff_t* pMipOffsets,
                                              DeviceImpl<ApiVariationNvn8>* pDevice,
                                              const TextureInfoImplNvn8& rInfo) {
    CalculateMipDataOffsetsImpl(pMipOffsets, pDevice->ToData()->pNvnDevice,
                                rInfo.nvnTextureBuilder);
}

/**
 * Returns the row pitch of a linear texture.
 *
 * @param pDevice Device the texture will be created on.
 * @param rInfo Texture description.
 * @return Row pitch in bytes, aligned to the device's linear stride alignment.
 */
size_t TextureImplNvn8::GetRowPitch(DeviceImpl<ApiVariationNvn8>* pDevice,
                                    const InfoType& rInfo) {
    int strideAlignment;
    nvnDeviceGetInteger(
        pDevice->ToData()->pNvnDevice,
        (rInfo.GetGpuAccessFlags() & (GpuAccess_DepthStencil | GpuAccess_ColorBuffer)) != 0 ?
            NVN_DEVICE_INFO_LINEAR_RENDER_TARGET_STRIDE_ALIGNMENT :
            NVN_DEVICE_INFO_LINEAR_TEXTURE_STRIDE_ALIGNMENT,
        &strideAlignment);

    size_t rowSize =
        CalculateRowSize(rInfo.GetWidth(), GetChannelFormat(rInfo.GetImageFormat()));
    return nn::util::align_up(rowSize, strideAlignment);
}

/**
 * Returns the row pitch of a linear texture.
 *
 * @param pDevice Device the texture will be created on (unused).
 * @param rInfo NVN-native texture description.
 * @return Stride configured in the texture builder.
 */
size_t TextureImplNvn8::GetRowPitch(DeviceImpl<ApiVariationNvn8>* pDevice,
                                    const TextureInfoImplNvn8& rInfo) {
    return nvnTextureBuilderGetStride(&rInfo.nvnTextureBuilder);
}

/**
 * Constructs an uninitialized texture, clearing all of its data.
 */
TextureImplNvn8::TextureImpl() {
    state = State_NotInitialized;
}

/**
 * Destroys the texture object. Finalize must have been called beforehand.
 */
TextureImplNvn8::~TextureImpl() {}

/**
 * Initializes the texture from a gfx texture description.
 *
 * @param pDevice Device to create the texture on.
 * @param rInfo Texture description.
 * @param pMemoryPool Memory pool holding the texture storage.
 * @param memoryPoolOffset Offset of the storage within the memory pool.
 * @param memoryPoolSize Size of the storage in bytes.
 */
void TextureImplNvn8::Initialize(DeviceImpl<ApiVariationNvn8>* pDevice, const InfoType& rInfo,
                                 MemoryPoolImpl<ApiVariationNvn8>* pMemoryPool,
                                 ptrdiff_t memoryPoolOffset, size_t memoryPoolSize) {
    NVNtextureBuilder builder;
    nvnTextureBuilderSetDevice(&builder, pDevice->ToData()->pNvnDevice);
    Nvn::ConvertToNvnTextureBuilder(&builder, rInfo);
    nvnTextureBuilderSetStorage(&builder, pMemoryPool->ToData()->pNvnMemoryPool,
                                memoryPoolOffset);

    if ((rInfo.GetGpuAccessFlags() & GpuAccess_Texture) != 0) {
        Nvn::SetPackagedTextureDataImpl(&builder, pMemoryPool, memoryPoolOffset, memoryPoolSize);
    }

    pNvnTexture = nvnTexture;
    nvnTextureInitialize(pNvnTexture, &builder);

    flags.SetBit(Flag_Shared, false);
    state = State_Initialized;
}

/**
 * Initializes the texture from an already configured NVN texture builder.
 *
 * @param pDevice Device to create the texture on.
 * @param rInfo NVN-native texture description.
 * @param pMemoryPool Memory pool holding the texture storage.
 * @param memoryPoolOffset Offset of the storage within the memory pool.
 * @param memoryPoolSize Size of the storage in bytes.
 */
void TextureImplNvn8::Initialize(DeviceImpl<ApiVariationNvn8>* pDevice,
                                 const TextureInfoImplNvn8& rInfo,
                                 MemoryPoolImpl<ApiVariationNvn8>* pMemoryPool,
                                 ptrdiff_t memoryPoolOffset, size_t memoryPoolSize) {
    NVNtextureBuilder builder = rInfo.nvnTextureBuilder;
    nvnTextureBuilderSetDevice(&builder, pDevice->ToData()->pNvnDevice);
    nvnTextureBuilderSetStorage(&builder, pMemoryPool->ToData()->pNvnMemoryPool,
                                memoryPoolOffset);
    Nvn::SetPackagedTextureDataImpl(&builder, pMemoryPool, memoryPoolOffset, memoryPoolSize);

    pNvnTexture = nvnTexture;
    nvnTextureInitialize(pNvnTexture, &builder);

    flags.SetBit(Flag_Shared, false);
    state = State_Initialized;
}

/**
 * Finalizes the texture, releasing the underlying NVN texture.
 *
 * @param pDevice Device the texture was created on (unused).
 */
void TextureImplNvn8::Finalize(DeviceImpl<ApiVariationNvn8>* pDevice) {
    nvnTextureFinalize(pNvnTexture);
    pNvnTexture = nullptr;
    state = State_NotInitialized;
}

/**
 * Sets a debug label on the underlying NVN texture.
 *
 * @param pDevice Device the texture was created on (unused).
 * @param pLabel Label string.
 */
void TextureImplNvn8::SetDebugLabel(DeviceImpl<ApiVariationNvn8>* pDevice, const char* pLabel) {
    nvnTextureSetDebugLabel(pNvnTexture, pLabel);
}

/**
 * Constructs an uninitialized texture view, clearing all of its data.
 */
TextureViewImplNvn8::TextureViewImpl() {
    state = State_NotInitialized;
}

/**
 * Destroys the texture view object. Finalize must have been called beforehand.
 */
TextureViewImplNvn8::~TextureViewImpl() {}

/**
 * Initializes the texture view.
 *
 * @param pDevice Device the source texture was created on (unused).
 * @param rInfo Texture view description.
 */
void TextureViewImplNvn8::Initialize(DeviceImpl<ApiVariationNvn8>* pDevice,
                                     const InfoType& rInfo) {
    const TextureImplNvn8* pSourceTexture = rInfo.GetTexturePtr();

    pNvnTexture = pSourceTexture->ToData()->pNvnTexture;
    pNvnTextureView = nvnTextureView;

    static const NVNtextureSwizzle s_ChannelMappingTable[] = {
        NVN_TEXTURE_SWIZZLE_ZERO, NVN_TEXTURE_SWIZZLE_ONE, NVN_TEXTURE_SWIZZLE_R,
        NVN_TEXTURE_SWIZZLE_G,    NVN_TEXTURE_SWIZZLE_B,   NVN_TEXTURE_SWIZZLE_A,
    };

    nvnTextureViewSetDefaults(pNvnTextureView);
    nvnTextureViewSetLevels(pNvnTextureView,
                            rInfo.GetSubresourceRange().GetMipRange().GetMinMipLevel(),
                            rInfo.GetSubresourceRange().GetMipRange().GetMipCount());

    if (rInfo.GetImageDimension() != ImageDimension_3d) {
        nvnTextureViewSetLayers(pNvnTextureView,
                                rInfo.GetSubresourceRange().GetArrayRange().GetBaseArrayIndex(),
                                rInfo.GetSubresourceRange().GetArrayRange().GetArrayLength());
    }

    Nvn::SetTextureViewFormat(pNvnTextureView, Nvn::GetImageFormat(rInfo.GetImageFormat()),
                              pNvnTexture);
    nvnTextureViewSetSwizzle(pNvnTextureView,
                             s_ChannelMappingTable[rInfo.GetChannelMapping(ColorChannel_Red)],
                             s_ChannelMappingTable[rInfo.GetChannelMapping(ColorChannel_Green)],
                             s_ChannelMappingTable[rInfo.GetChannelMapping(ColorChannel_Blue)],
                             s_ChannelMappingTable[rInfo.GetChannelMapping(ColorChannel_Alpha)]);
    nvnTextureViewSetDepthStencilMode(
        pNvnTextureView,
        rInfo.GetDepthStencilTextureMode() != DepthStencilFetchMode_DepthComponent ?
            NVN_TEXTURE_DEPTH_STENCIL_MODE_STENCIL :
            NVN_TEXTURE_DEPTH_STENCIL_MODE_DEPTH);
    nvnTextureViewSetTarget(pNvnTextureView, Nvn::GetImageTarget(rInfo.GetImageDimension()));

    flags.SetBit(Flag_Shared, false);
    state = State_Initialized;
}

/**
 * Finalizes the texture view.
 *
 * @param pDevice Device the source texture was created on (unused).
 */
void TextureViewImplNvn8::Finalize(DeviceImpl<ApiVariationNvn8>* pDevice) {
    state = State_NotInitialized;
    pNvnTexture = nullptr;
    pNvnTextureView = nullptr;
}

/**
 * Constructs an uninitialized color target view, clearing all of its data.
 */
ColorTargetViewImplNvn8::ColorTargetViewImpl() {
    state = State_NotInitialized;
}

/**
 * Destroys the color target view object. Finalize must have been called beforehand.
 */
ColorTargetViewImplNvn8::~ColorTargetViewImpl() {}

/**
 * Initializes the color target view.
 *
 * @param pDevice Device the source texture was created on (unused).
 * @param rInfo Color target view description.
 */
void ColorTargetViewImplNvn8::Initialize(DeviceImpl<ApiVariationNvn8>* pDevice,
                                         const InfoType& rInfo) {
    const TextureImplNvn8* pSourceTexture = rInfo.GetTexturePtr();

    pNvnTexture = pSourceTexture->ToData()->pNvnTexture;
    pNvnTextureView = nvnTextureView;

    nvnTextureViewSetDefaults(pNvnTextureView);
    nvnTextureViewSetLevels(pNvnTextureView, rInfo.GetMipLevel(), 1);
    nvnTextureViewSetLayers(pNvnTextureView, rInfo.GetArrayRange().GetBaseArrayIndex(),
                            rInfo.GetArrayRange().GetArrayLength());
    nvnTextureViewSetFormat(pNvnTextureView, Nvn::GetImageFormat(rInfo.GetImageFormat()));
    nvnTextureViewSetTarget(pNvnTextureView, Nvn::GetImageTarget(rInfo.GetImageDimension()));

    flags.SetBit(Flag_Shared, false);
    state = State_Initialized;
}

/**
 * Finalizes the color target view.
 *
 * @param pDevice Device the source texture was created on (unused).
 */
void ColorTargetViewImplNvn8::Finalize(DeviceImpl<ApiVariationNvn8>* pDevice) {
    state = State_NotInitialized;
    pNvnTexture = nullptr;
    pNvnTextureView = nullptr;
}

/**
 * Constructs an uninitialized depth stencil view, clearing all of its data.
 */
DepthStencilViewImplNvn8::DepthStencilViewImpl() {
    state = State_NotInitialized;
}

/**
 * Destroys the depth stencil view object. Finalize must have been called beforehand.
 */
DepthStencilViewImplNvn8::~DepthStencilViewImpl() {}

/**
 * Initializes the depth stencil view.
 *
 * @param pDevice Device the source texture was created on (unused).
 * @param rInfo Depth stencil view description.
 */
void DepthStencilViewImplNvn8::Initialize(DeviceImpl<ApiVariationNvn8>* pDevice,
                                          const InfoType& rInfo) {
    const TextureImplNvn8* pSourceTexture = rInfo.GetTexturePtr();

    pNvnTexture = pSourceTexture->ToData()->pNvnTexture;
    pNvnTextureView = nvnTextureView;

    nvnTextureViewSetDefaults(pNvnTextureView);
    nvnTextureViewSetLevels(pNvnTextureView, rInfo.GetMipLevel(), 1);
    nvnTextureViewSetLayers(pNvnTextureView, rInfo.GetArrayRange().GetBaseArrayIndex(),
                            rInfo.GetArrayRange().GetArrayLength());
    nvnTextureViewSetTarget(pNvnTextureView, Nvn::GetImageTarget(rInfo.GetImageDimension()));

    flags.SetBit(Flag_Shared, false);
    state = State_Initialized;
}

/**
 * Finalizes the depth stencil view.
 *
 * @param pDevice Device the source texture was created on (unused).
 */
void DepthStencilViewImplNvn8::Finalize(DeviceImpl<ApiVariationNvn8>* pDevice) {
    state = State_NotInitialized;
    pNvnTexture = nullptr;
    pNvnTextureView = nullptr;
}

/**
 * Queries the capabilities of an image format.
 *
 * @param pOutImageFormatProperty Receives the format capabilities.
 * @param pDevice Device to query (unused).
 * @param imageFormat Image format to query.
 */
template <>
void GetImageFormatProperty<ApiVariationNvn8>(ImageFormatProperty* pOutImageFormatProperty,
                                              DeviceImpl<ApiVariationNvn8>* pDevice,
                                              ImageFormat imageFormat) {
    NVNformat nvnFormat = Nvn::GetImageFormat(imageFormat);
    Nvn::GetImageFormatProperty(pOutImageFormatProperty, nvnFormat);
}

}  // namespace nn::gfx::detail
