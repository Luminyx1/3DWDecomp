#include <nn/gfx/gfx_ResShader.h>

namespace nn::gfx {

/**
 * Returns the largest alignment a ResShader binary may require.
 *
 * @return Alignment in bytes.
 */
size_t ResShaderFile::GetMaxFileAlignment() {
    return 4096;
}

/**
 * Checks the file header's signature and version.
 *
 * @param ptr Start of the file.
 * @return true if the file can be used.
 */
bool ResShaderFile::IsValid(const void* ptr) {
    const nn::util::BinaryFileHeader* pFileHeader =
        static_cast<const nn::util::BinaryFileHeader*>(ptr);

    return pFileHeader->IsValid(Signature, MajorVersion, MinorVersion, MicroVersion);
}

/**
 * Casts loaded file data to the resource, relocating it on first use.
 *
 * @param ptr Start of the file.
 * @return The resource file.
 */
ResShaderFile* ResShaderFile::ResCast(void* ptr) {
    value_type* pData = static_cast<value_type*>(ptr);
    ResShaderFile* pRet = ToAccessor(pData);

    if (!pRet->fileHeader.IsRelocated()) {
        pRet->fileHeader.GetRelocationTable()->Relocate();
    }

    return pRet;
}

}  // namespace nn::gfx
