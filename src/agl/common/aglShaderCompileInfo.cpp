#include "common/aglShaderCompileInfo.h"

namespace agl {

namespace {

const sead::SafeString cRegisterUniformBlockName = "RegisterUBO";

}  // namespace

/**
 * Constructs empty compile information.
 */
ShaderCompileInfo::ShaderCompileInfo() : mName("unititled"), mSource(nullptr), _20(nullptr) {}

/**
 * Destroys the compile information.
 */
ShaderCompileInfo::~ShaderCompileInfo()
{
    destroy();
}

/**
 * Frees the macro and variation arrays.
 */
void ShaderCompileInfo::destroy()
{
    if (mMacroName.isBufferReady()) {
        mMacroName.freeBuffer();
        mMacroValue.freeBuffer();
    }

    if (mVariationName.isBufferReady()) {
        mVariationName.freeBuffer();
        mVariationValue.freeBuffer();
    }
}

/**
 * Allocates the macro and variation arrays.
 * @param macroNum maximum number of macros
 * @param variationNum maximum number of variation macros
 * @param pHeap heap to allocate from
 */
void ShaderCompileInfo::create(s32 macroNum, s32 variationNum, sead::Heap* pHeap)
{
    if (macroNum > 0) {
        mMacroName.allocBuffer(macroNum, pHeap);
        mMacroValue.allocBuffer(macroNum, pHeap);
    }

    if (variationNum > 0) {
        mVariationName.allocBuffer(variationNum, pHeap);
        mVariationValue.allocBuffer(variationNum, pHeap);
    }
}

/**
 * Removes all variation macros.
 */
void ShaderCompileInfo::clearVariation()
{
    mVariationName.clear();
    mVariationValue.clear();
}

/**
 * Adds a variation macro.
 * @param pName macro name
 * @param pValue macro value
 */
void ShaderCompileInfo::pushBackVariation(const char* pName, const char* pValue)
{
    mVariationName.pushBack(pName);
    mVariationValue.pushBack(pValue);
}

/**
 * Gets the name of the uniform block that holds register uniforms.
 * @return "RegisterUBO"
 */
const sead::SafeString& ShaderCompileInfo::getRegitserUniformBlockName()
{
    return cRegisterUniformBlockName;
}

}  // namespace agl
