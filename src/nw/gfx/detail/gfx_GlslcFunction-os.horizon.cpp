#include <nn/gfx/detail/gfx_NvnHelper.h>

// GLSLC library entry points, statically linked into the program on this platform.
extern "C" {

bool glslcCompilePreSpecialized(GLSLCcompileObject* pCompileObject);
const GLSLCoutput* const* glslcCompileSpecialized(GLSLCcompileObject* pCompileObject,
                                                  const GLSLCspecializationBatch* pBatch);
uint8_t glslcInitialize(GLSLCcompileObject* pCompileObject);
void glslcFinalize(GLSLCcompileObject* pCompileObject);
uint8_t glslcCompile(GLSLCcompileObject* pCompileObject);
GLSLCversion glslcGetVersion();
void glslcSetAllocator(GLSLCallocateFunction pAllocate, GLSLCfreeFunction pFree,
                       GLSLCreallocateFunction pReallocate, void* pUserData);
GLSLCoptions glslcGetDefaultOptions();
}

namespace nn::gfx::detail {

/**
 * Returns the glslcCompilePreSpecialized entry point.
 *
 * @return The entry point.
 */
GlslcDll::GlslcCompilePreSpecializedType GetGlslcCompilePreSpecializedFunction() {
    return glslcCompilePreSpecialized;
}

/**
 * Returns the glslcCompileSpecialized entry point.
 *
 * @return The entry point.
 */
GlslcDll::GlslcCompileSpecializedType GetGlslcCompileSpecializedFunction() {
    return glslcCompileSpecialized;
}

/**
 * Returns the glslcInitialize entry point.
 *
 * @return The entry point.
 */
GlslcDll::GlslcInitializeType GetGlslcInitializeFunction() {
    return glslcInitialize;
}

/**
 * Returns the glslcFinalize entry point.
 *
 * @return The entry point.
 */
GlslcDll::GlslcFinalizeType GetGlslcFinalizeFunction() {
    return glslcFinalize;
}

/**
 * Returns the glslcCompile entry point.
 *
 * @return The entry point.
 */
GlslcDll::GlslcCompileType GetGlslcCompileFunction() {
    return glslcCompile;
}

/**
 * Returns the glslcGetVersion entry point.
 *
 * @return The entry point.
 */
GlslcDll::GlslcGetVersionType GetGlslcGetVersionFunction() {
    return glslcGetVersion;
}

/**
 * Returns the glslcSetAllocator entry point.
 *
 * @return The entry point.
 */
GlslcDll::GlslcSetAllocatorType GetGlslcSetAllocatorFunction() {
    return glslcSetAllocator;
}

/**
 * Returns the glslcGetDefaultOptions entry point.
 *
 * @return The entry point.
 */
GlslcDll::GlslcGetDefaultOptionsType GetGlslcGetDefaultOptionsFunction() {
    return glslcGetDefaultOptions;
}

}  // namespace nn::gfx::detail
