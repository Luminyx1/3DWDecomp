#include "common/aglShader.h"

namespace agl {

/**
 * Constructs a shader without a binary.
 */
Shader::Shader() : mShaderBinary(nullptr), mCompileInfo(nullptr), mBinaryInfo() {}

/**
 * Sets the compiled shader binary.
 * @param pShaderBinary shader binary
 */
void Shader::setBinary(const void* pShaderBinary)
{
    mShaderBinary = pShaderBinary;
}

}  // namespace agl
