#include <nn/gfx/detail/gfx_NvnHelper.h>

#include <nn/time.h>
#include <nvn/nvn_FuncPtrInline.h>

namespace nn::gfx::detail {

/**
 * Returns the index of the first scan buffer of a swap chain.
 *
 * @return Always 0.
 */
int Nvn::GetFirstScanBufferIndex() {
    return 0;
}

/**
 * Sets the packaged texture data of a texture builder. Does nothing on this platform.
 *
 * @param pBuilder The texture builder.
 * @param pMemoryPool The memory pool holding the texture data.
 * @param memoryPoolOffset The offset of the texture data in the memory pool.
 * @param memoryPoolSize The size of the texture data.
 */
void Nvn::SetPackagedTextureDataImpl(NVNtextureBuilder* pBuilder,
                                     MemoryPoolImpl<ApiVariationNvn8>* pMemoryPool,
                                     ptrdiff_t memoryPoolOffset, size_t memoryPoolSize) {}

/**
 * Sets the format of a texture view.
 *
 * @param pTextureView The texture view to modify.
 * @param format The new format.
 * @param pTexture The texture the view refers to (unused on this platform).
 */
void Nvn::SetTextureViewFormat(NVNtextureView* pTextureView, NVNformat format,
                               const NVNtexture* pTexture) {
    nvnTextureViewSetFormat(pTextureView, format);
}

/**
 * Converts a GPU timestamp to a time span.
 *
 * @param timestamp The GPU timestamp, in GPU ticks (614.4 MHz).
 * @return The corresponding time span.
 */
TimeSpan Nvn::ToTimeSpan(int64_t timestamp) {
    return TimeSpan::FromNanoSeconds(timestamp * 625 / 384);
}

/**
 * Resolves the GLSLC entry points.
 *
 * @return Whether every entry point was resolved.
 */
bool GlslcDll::Initialize() {
    GlslcCompilePreSpecialized = GetGlslcCompilePreSpecializedFunction();
    GlslcCompileSpecialized = GetGlslcCompileSpecializedFunction();
    GlslcInitialize = GetGlslcInitializeFunction();
    GlslcFinalize = GetGlslcFinalizeFunction();
    GlslcCompile = GetGlslcCompileFunction();
    GlslcGetVersion = GetGlslcGetVersionFunction();
    GlslcSetAllocator = GetGlslcSetAllocatorFunction();
    GlslcGetDefaultOptions = GetGlslcGetDefaultOptionsFunction();

    return GlslcCompilePreSpecialized != nullptr && GlslcCompileSpecialized != nullptr &&
           GlslcInitialize != nullptr && GlslcFinalize != nullptr && GlslcCompile != nullptr &&
           GlslcGetVersion != nullptr && GlslcSetAllocator != nullptr &&
           GlslcGetDefaultOptions != nullptr;
}

/**
 * Releases the GLSLC library. Does nothing on this platform.
 */
void GlslcDll::Finalize() {}

/**
 * Returns whether the GLSLC entry points have been resolved.
 *
 * @return Whether Initialize has resolved the entry points.
 */
bool GlslcDll::IsInitialized() const {
    return GlslcCompilePreSpecialized != nullptr;
}

/**
 * Returns whether the GLSLC compiler can emit thin GPU binaries.
 *
 * @return Always true.
 */
bool IsThinBinaryAvailable() {
    return true;
}

}  // namespace nn::gfx::detail
