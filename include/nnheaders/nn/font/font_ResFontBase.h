#pragma once

#include <nn/font/detail/font_ResourceFormat.h>
#include <nn/font/font_Font.h>
#include <cstring>
#include <nn/gfx/detail/gfx_MemoryPool-api.nvn.8.h>
#include <nn/gfx/detail/gfx_Texture-api.nvn.8.h>
#include <nn/gfx/gfx_MemoryPoolInfo.h>
#include <nn/gfx/gfx_ResTexture.h>
#include <nn/gfx/gfx_TextureInfo.h>

namespace nn {
namespace font {
namespace detail {

using GfxDeviceImpl = nn::gfx::detail::DeviceImpl<nn::gfx::ApiVariationNvn8>;
using GfxMemoryPoolImpl = nn::gfx::detail::MemoryPoolImpl<nn::gfx::ApiVariationNvn8>;
using GfxTextureImpl = nn::gfx::detail::TextureImpl<nn::gfx::ApiVariationNvn8>;
using GfxTextureViewImpl = nn::gfx::detail::TextureViewImpl<nn::gfx::ApiVariationNvn8>;

}  // namespace detail

class ResourceTextureObject : public TextureObject {
public:
    ResourceTextureObject() : m_pResTexture(nullptr) { Reset(); }

    void Initialize(nn::gfx::Device* pDevice, nn::gfx::MemoryPool* pMemoryPool,
                    ptrdiff_t memoryPoolOffset, size_t memoryPoolSize);

    const nn::gfx::TextureView* GetTextureView() const override {
        return static_cast<const nn::gfx::TextureView*>(m_pResTexture->ToData().pTextureView.Get());
    }
    nn::gfx::TextureView* GetTextureView() override {
        return static_cast<nn::gfx::TextureView*>(m_pResTexture->ToData().pTextureView.Get());
    }

    nn::gfx::ResTexture* GetResTexture() const { return m_pResTexture; }

private:
    friend class ResFontBase;

    nn::gfx::ResTexture* m_pResTexture;
};

class ResFontBase : public Font {
public:
    NN_RUNTIME_TYPEINFO(Font);

    static const int CharCodeRangeCountMax = 16;

    ResFontBase();
    ~ResFontBase() override;

    int GetWidth() const override;
    int GetHeight() const override;
    int GetAscent() const override;
    int GetDescent() const override;
    int GetMaxCharWidth() const override;
    FontType GetType() const override;
    TexFmt GetTextureFormat() const override;
    int GetLineFeed() const override;
    const CharWidths GetDefaultCharWidths() const override;
    void SetLineFeed(int linefeed) override;
    void SetDefaultCharWidths(const CharWidths& rWidths) override;
    bool SetAlternateChar(uint32_t c) override;
    int GetCharWidth(uint32_t c) const override;
    const CharWidths GetCharWidths(uint32_t c) const override;
    int GetGlyph(Glyph* pGlyph, uint32_t c) const override;
    bool HasGlyph(uint32_t c) const override;
    int GetKerning(uint32_t c0, uint32_t c1) const override;
    CharacterCode GetCharacterCode() const override;
    int GetBaselinePos() const override;
    int GetCellHeight() const override;
    int GetCellWidth() const override;
    void SetLinearFilterEnabled(bool atSmall, bool atLarge) override;
    bool IsLinearFilterEnabledAtSmall() const override;
    bool IsLinearFilterEnabledAtLarge() const override;
    uint32_t GetTextureWrapFilterValue() const override;
    bool IsColorBlackWhiteInterpolationEnabled() const override;
    void SetColorBlackWhiteInterpolationEnabled(bool isEnabled) override;

    /**
     * Checks whether the font has border glyphs.
     * @return whether the font type is the border type
     */
    bool IsBorderAvailable() const override { return m_pFontInfo->fontType == 2; }

    bool IsBorderEffectEnabled() const override;
    void GetAlternateCharGlyph(Glyph* pGlyph, uint32_t c) const override;
    virtual int GetActiveSheetCount() const;

    void RegisterTextureViewToDescriptorPool(RegisterTextureViewSlot pRegisterFunction,
                                             void* pUserData);
    void UnregisterTextureViewFromDescriptorPool(UnregisterTextureViewSlot pUnregisterFunction,
                                                 void* pUserData);

    uint16_t FindGlyphIndex(uint32_t c) const;
    uint16_t GetGlyphIndex(bool* pIsFound, uint32_t c) const;
    const CharWidths* GetCharWidthsFromIndex(uint16_t index) const;
    void GetGlyphFromIndex(Glyph* pGlyph, uint16_t index) const;
    bool CheckCharCodeRange(uint32_t c) const;
    void SetCharCodeRange(int count, uint32_t* pFirst, uint32_t* pLast);

    bool IsManaging(const void* ptr) const { return m_pResource == ptr; }

protected:
    void SetResourceBuffer(void* pUserBuffer, FontInformation* pFontInfo,
                           FontKerningTable* pKerningTable, nn::gfx::MemoryPool* pMemoryPool,
                           ptrdiff_t memoryPoolOffset, size_t memoryPoolSize);
    void* RemoveResourceBuffer();

    uint16_t FindGlyphIndex(const FontCodeMap* pMap, uint32_t c) const;
    const CharWidths* GetCharWidthsFromIndex(const FontWidth* pWidth, uint16_t index) const;
    static void SetGlyphMember(Glyph* pGlyph, uint16_t index, const FontTextureGlyph& rTexGlyph);
    static void* FindBlock(detail::BinaryFileHeader* pHeader, uint32_t signature);

    void GenTextureNames(nn::gfx::Device* pDevice);
    bool LoadTexture(nn::gfx::Device* pDevice, ResourceTextureObject* pTexObj) const;
    void DeleteTextureNames(nn::gfx::Device* pDevice);
    void UnloadTexture(nn::gfx::Device* pDevice, ResourceTextureObject* pTexObj) const;

    template <typename T>
    const T* GetResourcePtr(uint32_t offset) const {
        return reinterpret_cast<const T*>(reinterpret_cast<uintptr_t>(m_pResourceBase) + offset);
    }

    template <typename T>
    const T* GetResourcePtrOrNull(uint32_t offset) const {
        if (offset == 0) {
            return nullptr;
        }
        return GetResourcePtr<T>(offset);
    }

    const FontTextureGlyph* GetTextureGlyph() const {
        return GetResourcePtr<FontTextureGlyph>(m_pFontInfo->pGlyph);
    }

    void* m_pResource;
    void* m_pResourceBase;
    FontInformation* m_pFontInfo;
    ResourceTextureObject m_TextureObject;
    uint32_t m_TextureWrapFilter;
    FontKerningTable* m_pKerningTable;
    nn::gfx::MemoryPool* m_pMemoryPool;
    ptrdiff_t m_MemoryPoolOffset;
    size_t m_MemoryPoolSize;
    int m_CharCodeRangeCount;
    uint32_t m_CharCodeRangeFirst[CharCodeRangeCountMax];
    uint32_t m_CharCodeRangeLast[CharCodeRangeCountMax];
};

/**
 * Initializes the texture file of the image and its first texture.
 * @param pDevice gfx device
 * @param pMemoryPool memory pool holding the image, or nullptr to create one
 * @param memoryPoolOffset offset of the image in the memory pool
 * @param memoryPoolSize size of the image in the memory pool
 */
inline void ResourceTextureObject::Initialize(nn::gfx::Device* pDevice,
                                              nn::gfx::MemoryPool* pMemoryPool,
                                              ptrdiff_t memoryPoolOffset, size_t memoryPoolSize) {
    nn::gfx::ResTextureFile* pFile = nn::gfx::ResTextureFile::ResCast(const_cast<void*>(m_pImage));
    nn::gfx::ResTextureContainerData& rContainer = pFile->ToData().textureContainerData;

    if (rContainer.pCurrentMemoryPool.Get() != nullptr) {
        m_pResTexture = rContainer.pTexturePtrArray.Get()[0].Get();
        return;
    }

    auto* pDeviceImpl = reinterpret_cast<detail::GfxDeviceImpl*>(pDevice);

    if (pMemoryPool == nullptr) {
        nn::gfx::MemoryPoolInfo info;
        info.SetMemoryPoolProperty(0x21);
        auto* pBlock = static_cast<nn::util::BinaryBlockHeader*>(rContainer.pTextureData.Get());
        info.SetPoolMemory(reinterpret_cast<u8*>(pBlock) + sizeof(nn::util::BinaryBlockHeader),
                           pBlock->GetBlockSize() - sizeof(nn::util::BinaryBlockHeader));
        static_cast<detail::GfxMemoryPoolImpl*>(rContainer.pTextureMemoryPool.Get())
            ->Initialize(pDeviceImpl, info);
        rContainer.pCurrentMemoryPool.Set(rContainer.pTextureMemoryPool.Get());
        rContainer.memoryPoolOffsetBase = 0;
    } else {
        rContainer.pCurrentMemoryPool.Set(pMemoryPool);
        rContainer.memoryPoolOffsetBase =
            memoryPoolOffset +
            nn::util::BytePtr(pFile).Distance(rContainer.pTextureData.Get()) +
            sizeof(nn::util::BinaryBlockHeader);
    }

    nn::gfx::ResTexture* pResTexture = rContainer.pTexturePtrArray.Get()[0].Get();
    m_pResTexture = pResTexture;

    nn::gfx::ResTextureData& rData = pResTexture->ToData();
    const nn::gfx::TextureInfo& rInfo =
        *reinterpret_cast<const nn::gfx::TextureInfo*>(&rData.textureInfoData);
    nn::gfx::ResTextureContainerData* pContainer = rData.pResTextureContainerData.Get();
    ptrdiff_t offset =
        pContainer->memoryPoolOffsetBase -
        reinterpret_cast<uintptr_t>(static_cast<u8*>(pContainer->pTextureData.Get()) +
                                    sizeof(nn::util::BinaryBlockHeader)) +
        reinterpret_cast<uintptr_t>(rData.pMipPtrArray.Get()->Get());
    static_cast<detail::GfxTextureImpl*>(rData.pTexture.Get())
        ->Initialize(pDeviceImpl, rInfo,
                     static_cast<detail::GfxMemoryPoolImpl*>(pContainer->pCurrentMemoryPool.Get()),
                     offset, rData.textureDataSize);

    nn::gfx::TextureViewInfo info;
    info.SetDefault();
    info.SetImageDimension(static_cast<nn::gfx::ImageDimension>(rData.imageDimension));
    std::memcpy(info.ToData()->channelMapping, rData.channelMapping, sizeof(rData.channelMapping));
    info.SetImageFormat(static_cast<nn::gfx::ImageFormat>(rData.textureInfoData.imageFormat));
    info.SetTexturePtr(rData.pTexture.Get());
    info.EditSubresourceRange().EditArrayRange().SetArrayLength(rData.textureInfoData.arrayLength);
    info.EditSubresourceRange().EditMipRange().SetMipCount(rData.textureInfoData.mipCount);
    static_cast<detail::GfxTextureViewImpl*>(rData.pTextureView.Get())
        ->Initialize(pDeviceImpl, info);
}

}  // namespace font
}  // namespace nn
