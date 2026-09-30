#include <nn/font/font_ResFont.h>

#include <nn/nn_SdkAssert.h>
#include <nn/util.h>
#include <nn/util/util_BinaryFormat.h>

namespace nn {
namespace font {

namespace detail {

inline bool IsValidBinaryFile(const detail::BinaryFileHeader* pHeader, uint32_t signature,
                              uint32_t version, uint16_t minBlocks) {
    if (pHeader->signature != signature) {
        char buffer[256];
        nn::util::SNPrintf(
            buffer, sizeof(buffer), "Signature check failed ('%c%c%c%c' must be '%c%c%c%c').",
            static_cast<char>(pHeader->signature >> 24),
            static_cast<char>(pHeader->signature >> 16), static_cast<char>(pHeader->signature >> 8),
            static_cast<char>(pHeader->signature), static_cast<char>(signature >> 24),
            static_cast<char>(signature >> 16), static_cast<char>(signature >> 8),
            static_cast<char>(signature));
        return false;
    }

    if (pHeader->byteOrder != 0xfeff) {
        return false;
    }

    if ((pHeader->version >> 24) != (version >> 24)) {
        return false;
    }

    if (((pHeader->version >> 16) & 0xff) > ((version >> 16) & 0xff)) {
        return false;
    }

    if (pHeader->fileSize <
        sizeof(detail::BinaryFileHeader) + sizeof(detail::BinaryBlockHeader) * minBlocks) {
        return false;
    }

    if (pHeader->dataBlocks < minBlocks) {
        return false;
    }

    return true;
}

}  // namespace detail

namespace {

const uint32_t FontFileVersion = 0x04010000;

bool IsKnownBlock(uint32_t kind) {
    switch (kind) {
    case BinBlockSignatureFinf:
    case BinBlockSignatureKrng:
    case BinBlockSignatureCwdh:
    case BinBlockSignatureCmap:
    case BinBlockSignatureGlgr:
    case BinBlockSignatureTglp:
        return true;
    default:
        return false;
    }
}

}  // namespace

/**
 * Marks an unrelocated font binary as relocated again.
 * @param pBfnt font binary
 */
void ResFont::RevertResource(void* pBfnt) {
    detail::BinaryFileHeader* pHeader = static_cast<detail::BinaryFileHeader*>(pBfnt);

    if (pHeader->signature != BinFileSignatureFontUnrelocated) {
        return;
    }

    detail::BinaryBlockHeader* pBlock = reinterpret_cast<detail::BinaryBlockHeader*>(
        reinterpret_cast<uint8_t*>(pHeader) + pHeader->headerSize);
    for (int i = 0; i < pHeader->dataBlocks; i++) {
        if (!IsKnownBlock(pBlock->kind)) {
            return;
        }

        pBlock = reinterpret_cast<detail::BinaryBlockHeader*>(reinterpret_cast<uint8_t*>(pBlock) +
                                                              pBlock->size);
    }

    pHeader->signature = BinFileSignatureFont;
}

/**
 * Constructs a resource font without a resource.
 */
ResFont::ResFont() = default;

/**
 * Destroys the resource font.
 */
ResFont::~ResFont() {
    NN_SDK_ASSERT(m_pResource == nullptr);
}

/**
 * Finalizes the font and removes its resource.
 * @param pDevice gfx device
 */
void ResFont::Finalize(nn::gfx::Device* pDevice) {
    RemoveResource(pDevice);
}

/**
 * Removes the font resource and unloads its texture.
 * @param pDevice gfx device
 * @return the removed resource, or nullptr if there was none
 */
void* ResFont::RemoveResource(nn::gfx::Device* pDevice) {
    if (m_pResource == nullptr) {
        return nullptr;
    }

    DeleteTextureNames(pDevice);
    return RemoveResourceBuffer();
}

/**
 * Sets the font resource and loads its texture.
 * @param pDevice gfx device
 * @param pBfnt font binary
 * @param pMemoryPool memory pool holding the binary
 * @param memoryPoolOffset offset of the binary in the memory pool
 * @param memoryPoolSize size of the binary in the memory pool
 * @return whether the resource was set
 */
bool ResFont::SetResource(nn::gfx::Device* pDevice, void* pBfnt, nn::gfx::MemoryPool* pMemoryPool,
                          ptrdiff_t memoryPoolOffset, size_t memoryPoolSize) {
    if (m_pResource != nullptr) {
        return false;
    }

    detail::BinaryFileHeader* pHeader = static_cast<detail::BinaryFileHeader*>(pBfnt);

    if (!detail::IsValidBinaryFile(pHeader, BinFileSignatureFont, FontFileVersion, 2)) {
        return false;
    }

    FontInformation* pFontInfo = Rebuild(pHeader);
    m_pResourceBase = pHeader;

    if (pFontInfo == nullptr) {
        return false;
    }

    FontKerningTable* pKerningTable =
        static_cast<FontKerningTable*>(FindBlock(pHeader, BinBlockSignatureKrng));
    SetResourceBuffer(pBfnt, pFontInfo, pKerningTable, pMemoryPool, memoryPoolOffset,
                      memoryPoolSize);
    GenTextureNames(pDevice);
    return true;
}

/**
 * Finds the font information block of a font binary.
 * @param pHeader font binary
 * @return the font information, or nullptr if the binary has an unknown block
 */
FontInformation* ResFont::Rebuild(detail::BinaryFileHeader* pHeader) {
    FontInformation* pFontInfo = nullptr;
    detail::BinaryBlockHeader* pBlock = reinterpret_cast<detail::BinaryBlockHeader*>(
        reinterpret_cast<uint8_t*>(pHeader) + pHeader->headerSize);
    for (int i = 0; i < pHeader->dataBlocks; i++) {
        switch (pBlock->kind) {
        case BinBlockSignatureFinf:
            pFontInfo = reinterpret_cast<FontInformation*>(pBlock + 1);
            break;
        case BinBlockSignatureKrng:
        case BinBlockSignatureCwdh:
        case BinBlockSignatureCmap:
        case BinBlockSignatureGlgr:
        case BinBlockSignatureTglp:
            break;
        default:
            return nullptr;
        }

        pBlock = reinterpret_cast<detail::BinaryBlockHeader*>(reinterpret_cast<uint8_t*>(pBlock) +
                                                              pBlock->size);
    }

    return pFontInfo;
}

/**
 * Unrelocates the sheet texture file of a font binary.
 * @param pBfnt font binary
 */
void ResFont::Unrelocate(void* pBfnt) {
    detail::BinaryFileHeader* pHeader = static_cast<detail::BinaryFileHeader*>(pBfnt);
    detail::BinaryBlockHeader* pBlock = reinterpret_cast<detail::BinaryBlockHeader*>(
        reinterpret_cast<uint8_t*>(pHeader) + pHeader->headerSize);
    while (pBlock->kind != BinBlockSignatureFinf) {
    }

    FontInformation* pFontInfo = reinterpret_cast<FontInformation*>(pBlock + 1);
    const FontTextureGlyph* pTexGlyph = reinterpret_cast<const FontTextureGlyph*>(
        reinterpret_cast<uint8_t*>(pHeader) + pFontInfo->pGlyph);
    nn::util::BinaryFileHeader* pTextureFile = reinterpret_cast<nn::util::BinaryFileHeader*>(
        reinterpret_cast<uint8_t*>(pHeader) + pTexGlyph->sheetImage);
    if (pTextureFile->IsRelocated()) {
        pTextureFile->GetRelocationTable()->Unrelocate();
        pTextureFile->SetRelocated(false);
    }
}

}  // namespace font
}  // namespace nn
