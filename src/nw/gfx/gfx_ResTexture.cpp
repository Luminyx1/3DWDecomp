#include <nn/gfx/gfx_ResTexture.h>

namespace nn::gfx {

/**
 * Returns the largest alignment a ResTexture binary may require.
 *
 * @return Alignment in bytes.
 */
size_t ResTextureFile::GetMaxFileAlignment() {
    return 0x20000;
}

/**
 * Checks the file header's signature and version.
 *
 * @param ptr Start of the file.
 * @return true if the file can be used.
 */
bool ResTextureFile::IsValid(const void* ptr) {
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
ResTextureFile* ResTextureFile::ResCast(void* ptr) {
    value_type* pData = static_cast<value_type*>(ptr);
    ResTextureFile* pRet = ToAccessor(pData);

    if (!pRet->fileHeader.IsRelocated()) {
        pRet->fileHeader.GetRelocationTable()->Relocate();
    }

    return pRet;
}

}  // namespace nn::gfx
