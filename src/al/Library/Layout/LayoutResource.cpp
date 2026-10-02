#include "Library/Layout/LayoutResource.hpp"

#include <eui/euiArcResourceMgr.h>
#include <eui/euiFontMgr.h>
#include <eui/euiScreenMgr.h>
#include <gfx/nin/seadGraphicsNvn.h>
#include <nn/gfx/gfx_MemoryPool.h>
#include <nn/gfx/gfx_MemoryPoolInfo.h>
#include <nn/gfx/gfx_ResTexture.h>
#include <nn/gfx/gfx_ResTextureData.h>
#include <nn/ui2d/ui2d_ArcExtractor.h>
#include <nn/ui2d/ui2d_Layout.h>
#include <nn/util/util_ResDic.h>
#include <prim/seadSafeString.h>

#include "Library/Layout/LayoutInitFunction.hpp"
#include "Library/Layout/LayoutSystem.hpp"
#include "Library/Message/LanguageUtil.hpp"
#include "Library/Resource/Resource.hpp"
#include "Library/Resource/ResourceFunction.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
namespace {
using MemoryPoolImpl = nn::gfx::detail::MemoryPoolImpl<nn::gfx::ApiVariationNvn8>;

/**
 * Returns the texture container of a texture file.
 * @param pFile texture file
 * @return the texture container
 */
nn::gfx::ResTextureContainerData& getContainer(nn::gfx::ResTextureFile* pFile) {
    return pFile->ToData().textureContainerData;
}

/**
 * Initializes the memory pool of a texture container on its texture data.
 * @param rContainer texture container
 */
void initializeTextureMemoryPool(nn::gfx::ResTextureContainerData& rContainer) {
    auto* device = sead::GraphicsNvn::instance()->getGfxDevice();
    nn::gfx::MemoryPoolInfo info;
    info.SetMemoryPoolProperty(0x21);
    auto* block = static_cast<nn::util::BinaryBlockHeader*>(rContainer.pTextureData.Get());
    info.SetPoolMemory(reinterpret_cast<u8*>(block) + sizeof(nn::util::BinaryBlockHeader),
                       block->GetBlockSize() - sizeof(nn::util::BinaryBlockHeader));
    static_cast<MemoryPoolImpl*>(rContainer.pTextureMemoryPool.Get())->Initialize(device, info);
}
}  // namespace

/**
 * Creates the resource accessor of a layout archive.
 * @param pResource layout archive
 * @param pLayoutSystem layout system holding the shared archives and fonts
 * @param isLocalized whether the layout archives are localized
 */
LayoutResource::LayoutResource(const Resource* pResource, const LayoutSystem* pLayoutSystem,
                               bool isLocalized)
    : eui::MultiArcResourceAccessor(pLayoutSystem->getScreenMgr()->mArcResourceMgr,
                                    pLayoutSystem->getScreenMgr()->getFontMgr()),
      mIsLocalized(isLocalized) {
    addResourceLink(pResource);
}

/**
 * Attaches the layout archive of a resource and sets up its combined texture file.
 * @param pResource resource holding the layout archive
 */
void LayoutResource::addResourceLink(const Resource* pResource) {
    void* archive = pResource->getOtherFile("layout.lyarc", nullptr);
    nn::ui2d::ArcExtractor extractor(archive);
    nn::ui2d::ArcFileInfo fileInfo;
    s32 entryId = extractor.ConvertPathToEntryId("timg/__Combined.bntx");
    nn::gfx::ResTextureFile* textureFile = nullptr;

    if (entryId >= 0) {
        void* file = extractor.GetFileFast(&fileInfo, entryId);

        if (file != nullptr && fileInfo.GetLength() != 0) {
            textureFile = nn::gfx::ResTextureFile::ResCast(file);
            nn::gfx::ResTextureContainerData& rContainer = getContainer(textureFile);

            if (static_cast<MemoryPoolImpl*>(rContainer.pTextureMemoryPool.Get())->ToData()->state ==
                nn::gfx::MemoryPoolImplData<nn::gfx::ApiVariationNvn8>::State_NotInitialized) {
                initializeTextureMemoryPool(rContainer);
                rContainer.pCurrentMemoryPool.Set(rContainer.pTextureMemoryPool.Get());
                rContainer.memoryPoolOffsetBase = 0;

                void* memory = nn::ui2d::Layout::AllocateMemory(sizeof(TextureFileLink));

                if (memory != nullptr) {
                    new (memory) TextureFileLink(textureFile);
                }

                mTextureFiles.LinkNext(&static_cast<TextureFileLink*>(memory)->mLink);
            }
        }
    }

    attachArchive(archive, textureFile);
}

/**
 * Reinitializes the shaders of every attached archive.
 * @param pDevice gfx device
 */
void LayoutResource::reinitializeShaders(nn::gfx::Device* pDevice) {
    ArchiveList& archives = getArchiveList();

    for (auto it = archives.begin(); it != archives.end(); ++it) {
        eui::ArcResourceMgr::reinitializeShaderResource(
            const_cast<nn::ui2d::ArcExtractor::ArchiveBlockHeader*>(
                it->mExtractor.m_pArchiveBlockHeader));
    }
}

/**
 * Acquires every texture of every attached archive's texture file.
 * @param pDevice gfx device
 */
void LayoutResource::loadAllTextures(nn::gfx::Device* pDevice) {
    ArchiveList& archives = getArchiveList();

    for (auto it = archives.begin(); it != archives.end(); ++it) {
        nn::gfx::ResTextureFile* textureFile = it->mTextureFile;

        if (textureFile == nullptr) {
            continue;
        }

        auto* dic = static_cast<const nn::util::ResDic*>(getContainer(textureFile).pTextureDic.Get());

        for (s32 i = 0; i < dic->GetCount(); i++) {
            AcquireTexture(pDevice, dic->GetKey(i).data());
        }
    }
}

/**
 * Finds a resource, loading the layout archive of a missing layout on demand.
 * @param pSize output size of the resource
 * @param type resource type
 * @param pName resource name
 * @return the resource, or nullptr
 */
void* LayoutResource::FindResourceByName(size_t* pSize, u32 type, const char* pName) {
    void* resource = MultiArcResourceAccessor::FindResourceByName(pSize, type, pName);

    if (resource != nullptr) {
        return resource;
    }

    if (type == 'blyt') {
        char layoutName[256];
        removeExtensionString(layoutName, sizeof(layoutName), pName);
        StringTmp<128> archivePath;
        makeLayoutArchivePath(&archivePath, layoutName, mIsLocalized);
        addResourceLink(findOrCreateResource(archivePath.cstr(), nullptr));
    }

    return MultiArcResourceAccessor::FindResourceByName(pSize, type, pName);
}

/**
 * Finalizes the texture memory pools and shaders of the attached archives.
 * @param pDevice gfx device
 */
void LayoutResource::Finalize(nn::gfx::Device* pDevice) {
    for (nn::util::IntrusiveListNode* node = mTextureFiles.GetNext(); &mTextureFiles != node;
         node = node->GetNext()) {
        auto* link = reinterpret_cast<TextureFileLink*>(node);
        nn::gfx::ResTextureContainerData& rContainer = getContainer(link->mTextureFile);

        if (rContainer.pCurrentMemoryPool.Get() == rContainer.pTextureMemoryPool.Get()) {
            static_cast<MemoryPoolImpl*>(rContainer.pCurrentMemoryPool.Get())->Finalize(pDevice);
        }

        rContainer.pCurrentMemoryPool.Set(nullptr);
    }

    ArchiveList& archives = getArchiveList();

    for (auto it = archives.begin(); it != archives.end(); ++it) {
        eui::ArcResourceMgr::finalizeInitializedShaderResource(
            const_cast<nn::ui2d::ArcExtractor::ArchiveBlockHeader*>(
                it->mExtractor.m_pArchiveBlockHeader));
    }

    MultiArcResourceAccessor::Finalize(pDevice);
}

/**
 * Acquires a font, preferring the language-specific variant of the game fonts.
 * @param pDevice gfx device
 * @param pName font name
 * @return the font, or nullptr
 */
nn::font::Font* LayoutResource::AcquireFont(nn::gfx::Device* pDevice, const char* pName) {
    const char* language = getLanguageString();
    StringTmp<128> fontName;
    removeExtensionString(fontName.getBuffer(), 128, pName);

    if (isEqualString("CNzh", language) || isEqualString("TWzh", language) ||
        isEqualString("KRko", language)) {
        if (isEqualString("MarioFont64", fontName.cstr()) ||
            isEqualString("MarioFont128", fontName.cstr()) ||
            isEqualString("MarioFont128Solid", fontName.cstr()) ||
            isEqualString("nintendo_NTLG_DB_002_Switch_50px", fontName.cstr())) {
            fontName.appendWithFormat("_%s.bffnt", language);
            nn::font::Font* font = mFonts->tryGetFont(fontName.cstr());

            if (font != nullptr) {
                return font;
            }
        }
    }

    nn::font::Font* font = mFonts->tryGetFont(pName);

    if (font != nullptr) {
        return font;
    }

    return mFonts->tryGetFont("nintendo_NTLG_DB_002_Switch_50px.bffnt");
}

/**
 * Destroys the layout resource.
 */
LayoutResource::~LayoutResource() = default;
}  // namespace al
