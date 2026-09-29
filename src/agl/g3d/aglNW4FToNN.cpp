#include "g3d/aglNW4FToNN.h"

#include <cstring>
#include <nn/gfx/gfx_MemoryPool.h>
#include <nn/gfx/gfx_MemoryPoolInfo.h>
#include <nn/gfx/gfx_TextureInfo.h>

#include "driver/aglNVNMgr.h"
#include "g3d/aglG3DDecl.h"

namespace agl::g3d {

namespace {

using DeviceImpl = nn::gfx::detail::DeviceImpl<nn::gfx::ApiVariationNvn8>;
using MemoryPoolImpl = nn::gfx::detail::MemoryPoolImpl<nn::gfx::ApiVariationNvn8>;
using TextureImpl = nn::gfx::detail::TextureImpl<nn::gfx::ApiVariationNvn8>;
using TextureViewImpl = nn::gfx::detail::TextureViewImpl<nn::gfx::ApiVariationNvn8>;

/**
 * Gets the gfx device of the graphics driver.
 * @return the gfx device
 */
DeviceImpl* getDevice()
{
    return static_cast<DeviceImpl*>(driver::NVNMgr::instance()->getGfxDevice());
}

/**
 * Gets the texture container of a texture file.
 * @param pFile texture file
 * @return the texture container
 */
nn::gfx::ResTextureContainerData& getContainer(nn::gfx::ResTextureFile* pFile)
{
    return pFile->ToData().textureContainerData;
}

/**
 * Gets the texture container of a texture file.
 * @param pFile texture file
 * @return the texture container
 */
const nn::gfx::ResTextureContainerData& getContainer(const nn::gfx::ResTextureFile* pFile)
{
    return pFile->ToData().textureContainerData;
}

/**
 * Gets a texture of a texture container.
 * @param rContainer texture container
 * @param index index of the texture
 * @return the texture
 */
nn::gfx::ResTexture* getTexture(const nn::gfx::ResTextureContainerData& rContainer, int index)
{
    return const_cast<nn::gfx::ResTexture*>(rContainer.pTexturePtrArray.Get()[index].Get());
}

/**
 * Makes a texture reference to a texture resource.
 * @param pTexture texture resource
 * @return the texture reference
 */
nn::g3d::TextureRef makeTextureRef(const nn::gfx::ResTexture* pTexture)
{
    const nn::gfx::ResTextureData& rData = pTexture->ToData();
    return nn::g3d::TextureRef(static_cast<const nn::gfx::TextureView*>(rData.pTextureView.Get()),
                               rData.userDescriptorSlot.value);
}

/**
 * Gets the texture view of a texture resource.
 * @param rData texture resource data
 * @return the texture view
 */
TextureViewImpl* getTextureView(const nn::gfx::ResTextureData& rData)
{
    return static_cast<TextureViewImpl*>(const_cast<void*>(rData.pTextureView.Get()));
}

struct SharedTextureFiles {
    nn::gfx::ResTextureFile* mpFile;
    const nn::gfx::ResTextureFile* mpSharedFile;
};

}  // namespace

/**
 * Sets up a bfres file, its embedded textures and its samplers.
 * @param pResFile bfres file
 */
void ResFile::Setup(nn::g3d::ResFile* pResFile)
{
    pResFile->Setup(reinterpret_cast<nn::gfx::Device*>(getDevice()));

    nn::gfx::ResTextureFile* pTextureFile = getResTextureFile(pResFile);
    if (pTextureFile)
    {
        nn::gfx::ResTextureContainerData& rContainer = getContainer(pTextureFile);
        if (static_cast<MemoryPoolImpl*>(rContainer.pTextureMemoryPool.Get())->ToData()->state ==
            nn::gfx::MemoryPoolImplData<nn::gfx::ApiVariationNvn8>::State_Initialized)
        {
            return;
        }

        DeviceImpl* pDevice = getDevice();
        {
            nn::gfx::MemoryPoolInfo info;
            info.SetMemoryPoolProperty(0x21);
            auto* pBlock =
                static_cast<nn::util::BinaryBlockHeader*>(rContainer.pTextureData.Get());
            info.SetPoolMemory(reinterpret_cast<u8*>(pBlock) + sizeof(nn::util::BinaryBlockHeader),
                               pBlock->GetBlockSize() - sizeof(nn::util::BinaryBlockHeader));
            static_cast<MemoryPoolImpl*>(rContainer.pTextureMemoryPool.Get())
                ->Initialize(pDevice, info);
        }

        rContainer.pCurrentMemoryPool.Set(rContainer.pTextureMemoryPool.Get());
        rContainer.memoryPoolOffsetBase = 0;
        for (s32 i = 0, n = rContainer.pTextureDic.Get()->GetCount(); i < n; i++)
        {
            ResTexture::Initialize(getTexture(rContainer, i));
        }
    }

    for (s32 i = 0; i < pResFile->GetModelCount(); i++)
    {
        nn::g3d::ResModel* pModel = pResFile->GetModel(i);
        for (s32 j = 0; j < pModel->GetMaterialCount(); j++)
        {
            nn::g3d::ResMaterialData& rMaterial = pModel->GetMaterial(j)->ToData();
            for (s32 k = 0; k < rMaterial.samplerCount; k++)
            {
                const auto* pSampler =
                    static_cast<const NVNsampler*>(rMaterial.pSamplerArray.Get()[k].pGfxSampler.Get());
                rMaterial.pSamplerSlotArray.Get()[k] =
                    driver::NVNMgr::instance()->registerSampler(pSampler, nullptr);
            }
        }
    }
}

/**
 * Gets the texture file embedded in a bfres file.
 * @param pResFile bfres file
 * @return the texture file, or nullptr if there is none
 */
nn::gfx::ResTextureFile* ResFile::getResTextureFile(nn::g3d::ResFile* pResFile)
{
    const nn::g3d::ResExternalFileData* pFile = pResFile->FindExternalFile("textures.bntx");
    if (!pFile)
    {
        return nullptr;
    }
    return nn::gfx::ResTextureFile::ResCast(const_cast<void*>(pFile->pData.Get()));
}

/**
 * Initializes the texture and texture view of a texture resource and registers it.
 * @param pResTexture texture resource
 */
void ResTexture::Initialize(nn::gfx::ResTexture* pResTexture)
{
    nn::gfx::ResTextureData& rData = pResTexture->ToData();
    auto* pTextureView = static_cast<TextureViewImpl*>(rData.pTextureView.Get());
    pTextureView->ToData()->userPtr = pResTexture;

    DeviceImpl* pDevice = getDevice();
    nn::gfx::ResTextureContainerData* pContainer = rData.pResTextureContainerData.Get();
    ptrdiff_t offset =
        pContainer->memoryPoolOffsetBase -
        reinterpret_cast<uintptr_t>(static_cast<u8*>(pContainer->pTextureData.Get()) + 0x10) +
        reinterpret_cast<uintptr_t>(rData.pMipPtrArray.Get()->Get());
    static_cast<TextureImpl*>(rData.pTexture.Get())
        ->Initialize(pDevice, *reinterpret_cast<const nn::gfx::TextureInfo*>(&rData.textureInfoData),
                     static_cast<MemoryPoolImpl*>(pContainer->pCurrentMemoryPool.Get()), offset,
                     rData.textureDataSize);

    nn::gfx::TextureViewInfo info;
    info.SetDefault();
    info.SetImageDimension(static_cast<nn::gfx::ImageDimension>(rData.imageDimension));
    std::memcpy(info.ToData()->channelMapping, rData.channelMapping, sizeof(rData.channelMapping));
    info.SetImageFormat(static_cast<nn::gfx::ImageFormat>(rData.textureInfoData.imageFormat));
    info.SetTexturePtr(rData.pTexture.Get());
    info.EditSubresourceRange().EditArrayRange().SetArrayLength(rData.textureInfoData.arrayLength);
    info.EditSubresourceRange().EditMipRange().SetMipCount(rData.textureInfoData.mipCount);
    getTextureView(rData)->Initialize(pDevice, info);

    const char* pName = rData.pName.Get()->GetData();
    const auto* pNvnTexture = static_cast<const NVNtexture*>(pTextureView->ToData()->pNvnTexture.ptr);
    const auto* pNvnTextureView =
        static_cast<const NVNtextureView*>(pTextureView->ToData()->pNvnTextureView.ptr);
    rData.userDescriptorSlot.value = static_cast<u32>(
        driver::NVNMgr::instance()->registerTexture(pNvnTexture, pNvnTextureView, pName));
}

/**
 * Cleans up a bfres file, its embedded textures and its samplers.
 * @param pResFile bfres file
 */
void ResFile::Cleanup(nn::g3d::ResFile* pResFile)
{
    pResFile->Cleanup(reinterpret_cast<nn::gfx::Device*>(getDevice()));

    nn::gfx::ResTextureFile* pTextureFile = getResTextureFile(pResFile);
    if (pTextureFile)
    {
        nn::gfx::ResTextureContainerData& rContainer = getContainer(pTextureFile);
        auto* pMemoryPool = static_cast<MemoryPoolImpl*>(rContainer.pTextureMemoryPool.Get());
        if (pMemoryPool->ToData()->state == nn::gfx::MemoryPoolImplData<
                                                nn::gfx::ApiVariationNvn8>::State_NotInitialized)
        {
            return;
        }

        for (s32 i = 0, n = rContainer.pTextureDic.Get()->GetCount(); i < n; i++)
        {
            ResTexture::Finalize(getTexture(rContainer, i));
        }

        if (rContainer.pCurrentMemoryPool.Get() == rContainer.pTextureMemoryPool.Get())
        {
            static_cast<MemoryPoolImpl*>(rContainer.pTextureMemoryPool.Get())->Finalize(getDevice());
        }
        rContainer.pCurrentMemoryPool.Set(nullptr);
    }

    for (s32 i = 0; i < pResFile->GetModelCount(); i++)
    {
        nn::g3d::ResModel* pModel = pResFile->GetModel(i);
        for (s32 j = 0; j < pModel->GetMaterialCount(); j++)
        {
            nn::g3d::ResMaterialData& rMaterial = pModel->GetMaterial(j)->ToData();
            for (s32 k = 0; k < rMaterial.samplerCount; k++)
            {
                u64 slot = rMaterial.pSamplerSlotArray.Get()[k];
                if (slot != nn::g3d::TextureRef::InvalidDescriptorSlot)
                {
                    driver::NVNMgr::instance()->releaseSampler(slot);
                }
            }
        }
    }
}

/**
 * Releases and finalizes the texture and texture view of a texture resource.
 * @param pResTexture texture resource
 */
void ResTexture::Finalize(nn::gfx::ResTexture* pResTexture)
{
    nn::gfx::ResTextureData& rData = pResTexture->ToData();
    getTextureView(rData)->ToData()->userPtr = nullptr;
    if (static_cast<s32>(rData.userDescriptorSlot.value) != -1)
    {
        driver::NVNMgr::instance()->releaseTexture(rData.userDescriptorSlot.value);
        DeviceImpl* pDevice = getDevice();
        static_cast<TextureImpl*>(rData.pTexture.Get())->Finalize(pDevice);
        getTextureView(rData)->Finalize(pDevice);
    }
    rData.userDescriptorSlot.value = 0;
}

/**
 * Gets the name of an embedded texture.
 * @param pResFile bfres file
 * @param index index of the texture
 * @return the name, or nullptr if there is no texture file
 */
const char* ResFile::GetTextureName(const nn::g3d::ResFile* pResFile, s32 index)
{
    const nn::gfx::ResTextureFile* pTextureFile = getResTextureFile(pResFile);
    if (!pTextureFile)
    {
        return nullptr;
    }
    return getTexture(getContainer(pTextureFile), index)->ToData().pName.Get()->GetData();
}

/**
 * Gets the texture file embedded in a bfres file.
 * @param pResFile bfres file
 * @return the texture file, or nullptr if there is none
 */
const nn::gfx::ResTextureFile* ResFile::getResTextureFile(const nn::g3d::ResFile* pResFile)
{
    const nn::g3d::ResExternalFileData* pFile = pResFile->FindExternalFile("textures.bntx");
    if (!pFile)
    {
        return nullptr;
    }
    return nn::gfx::ResTextureFile::ResCast(const_cast<void*>(pFile->pData.Get()));
}

/**
 * Gets the number of embedded textures.
 * @param pResFile bfres file
 * @return the number of textures
 */
s32 ResFile::GetTextureCount(const nn::g3d::ResFile* pResFile)
{
    const nn::gfx::ResTextureFile* pTextureFile = getResTextureFile(pResFile);
    if (!pTextureFile)
    {
        return 0;
    }
    return getContainer(pTextureFile).pTextureDic.Get()->GetCount();
}

/**
 * Gets the index of an embedded texture.
 * @param pResFile bfres file
 * @param pName name of the texture
 * @return the index, or -1 if not found
 */
s32 ResFile::GetTextureIndex(const nn::g3d::ResFile* pResFile, const char* pName)
{
    const nn::gfx::ResTextureFile* pTextureFile = getResTextureFile(pResFile);
    if (!pTextureFile)
    {
        return -1;
    }
    return getContainer(pTextureFile).pTextureDic.Get()->FindIndex(pName);
}

/**
 * Gets an embedded texture by name.
 * @param pResFile bfres file
 * @param pName name of the texture
 * @return the texture, or nullptr if not found
 */
nn::gfx::ResTexture* ResFile::GetTexture(nn::g3d::ResFile* pResFile, const char* pName)
{
    nn::gfx::ResTextureFile* pTextureFile = getResTextureFile(pResFile);
    if (!pTextureFile)
    {
        return nullptr;
    }
    const nn::gfx::ResTextureContainerData& rContainer = getContainer(pTextureFile);
    s32 index = rContainer.pTextureDic.Get()->FindIndex(pName);
    if (index == -1)
    {
        return nullptr;
    }
    return getTexture(rContainer, index);
}

/**
 * Gets an embedded texture by name.
 * @param pResFile bfres file
 * @param pName name of the texture
 * @return the texture, or nullptr if not found
 */
const nn::gfx::ResTexture* ResFile::GetTexture(const nn::g3d::ResFile* pResFile, const char* pName)
{
    const nn::gfx::ResTextureFile* pTextureFile = getResTextureFile(pResFile);
    if (!pTextureFile)
    {
        return nullptr;
    }
    const nn::gfx::ResTextureContainerData& rContainer = getContainer(pTextureFile);
    s32 index = rContainer.pTextureDic.Get()->FindIndex(pName);
    if (index == -1)
    {
        return nullptr;
    }
    return getTexture(rContainer, index);
}

/**
 * Gets an embedded texture by index.
 * @param pResFile bfres file
 * @param index index of the texture
 * @return the texture, or nullptr if there is no texture file
 */
nn::gfx::ResTexture* ResFile::GetTexture(nn::g3d::ResFile* pResFile, s32 index)
{
    nn::gfx::ResTextureFile* pTextureFile = getResTextureFile(pResFile);
    if (!pTextureFile)
    {
        return nullptr;
    }
    return getTexture(getContainer(pTextureFile), index);
}

/**
 * Gets an embedded texture by index.
 * @param pResFile bfres file
 * @param index index of the texture
 * @return the texture, or nullptr if there is no texture file
 */
const nn::gfx::ResTexture* ResFile::GetTexture(const nn::g3d::ResFile* pResFile, s32 index)
{
    const nn::gfx::ResTextureFile* pTextureFile = getResTextureFile(pResFile);
    if (!pTextureFile)
    {
        return nullptr;
    }
    return getTexture(getContainer(pTextureFile), index);
}

/**
 * Binds the textures of a bfres file to the textures embedded in another one.
 * @param pResFile bfres file to bind
 * @param pTextureResFile bfres file containing the textures
 * @return whether every texture was bound
 */
bool ResFile::BindTexture(nn::g3d::ResFile* pResFile, const nn::g3d::ResFile* pTextureResFile)
{
    const nn::gfx::ResTextureFile* pTextureFile = getResTextureFile(pTextureResFile);
    if (!pTextureFile)
    {
        return false;
    }
    return pResFile
        ->BindTexture(TextureBindCallback, const_cast<nn::gfx::ResTextureFile*>(pTextureFile))
        .IsComplete();
}

/**
 * Looks up a texture in a texture file.
 * @param pName name of the texture
 * @param pUserData texture file
 * @return the texture reference, invalid if not found
 */
nn::g3d::TextureRef ResFile::TextureBindCallback(const char* pName, void* pUserData)
{
    const nn::gfx::ResTextureContainerData& rContainer =
        getContainer(static_cast<const nn::gfx::ResTextureFile*>(pUserData));
    s32 index = rContainer.pTextureDic.Get()->FindIndex(pName);
    if (index == -1)
    {
        return nn::g3d::TextureRef();
    }
    return makeTextureRef(getTexture(rContainer, index));
}

/**
 * Binds the textures of a bfres file to its own textures or those of a shared file.
 * @param pResFile bfres file to bind
 * @param pSharedResFile bfres file containing shared textures
 * @return whether every texture was bound
 */
bool ResFile::BindSharedTexture(nn::g3d::ResFile* pResFile,
                                const nn::g3d::ResFile* pSharedResFile)
{
    SharedTextureFiles files;
    files.mpFile = getResTextureFile(pResFile);
    files.mpSharedFile = getResTextureFile(pSharedResFile);
    if (!files.mpFile && !files.mpSharedFile)
    {
        return false;
    }
    return pResFile->BindTexture(SharedTextureBindCallback, &files).IsComplete();
}

/**
 * Looks up a texture in a texture file, then in a shared texture file.
 * @param pName name of the texture
 * @param pUserData pair of texture files
 * @return the texture reference, invalid if not found
 */
nn::g3d::TextureRef ResFile::SharedTextureBindCallback(const char* pName, void* pUserData)
{
    const auto* pFiles = static_cast<const SharedTextureFiles*>(pUserData);
    if (pFiles->mpFile)
    {
        const nn::gfx::ResTextureContainerData& rContainer = getContainer(pFiles->mpFile);
        s32 index = rContainer.pTextureDic.Get()->FindIndex(pName);
        if (index != -1)
        {
            return makeTextureRef(getTexture(rContainer, index));
        }
    }
    if (pFiles->mpSharedFile)
    {
        const nn::gfx::ResTextureContainerData& rContainer = getContainer(pFiles->mpSharedFile);
        s32 index = rContainer.pTextureDic.Get()->FindIndex(pName);
        if (index != -1)
        {
            return makeTextureRef(getTexture(rContainer, index));
        }
    }
    return nn::g3d::TextureRef();
}

/**
 * Checks whether a texture resource was initialized.
 * @param pResTexture texture resource
 * @return whether the texture view refers back to the resource
 */
bool ResTexture::IsInitialized(const nn::gfx::ResTexture* pResTexture)
{
    return nn::g3d::GetTextureViewUserPtr(static_cast<const nn::gfx::TextureView*>(
               pResTexture->ToData().pTextureView.Get())) != nullptr;
}

namespace {

/**
 * Gets the texture resource of a texture view.
 * @param pTextureView texture view, may be nullptr
 * @return the texture resource, or nullptr
 */
nn::gfx::ResTexture* toResTexture(const nn::gfx::TextureView* pTextureView)
{
    return pTextureView ?
               static_cast<nn::gfx::ResTexture*>(nn::g3d::GetTextureViewUserPtr(pTextureView)) :
               nullptr;
}

}  // namespace

/**
 * Gets a texture bound to a material resource.
 * @param pMaterial material resource
 * @param index index of the texture
 * @return the texture, or nullptr if unbound
 */
nn::gfx::ResTexture* ResMaterial::GetTexture(nn::g3d::ResMaterial* pMaterial, s32 index)
{
    return toResTexture(pMaterial->GetTextureView(index));
}

/**
 * Gets a texture bound to a material resource.
 * @param pMaterial material resource
 * @param index index of the texture
 * @return the texture, or nullptr if unbound
 */
const nn::gfx::ResTexture* ResMaterial::GetTexture(const nn::g3d::ResMaterial* pMaterial,
                                                   s32 index)
{
    return toResTexture(pMaterial->GetTextureView(index));
}

/**
 * Binds a texture to a material resource.
 * @param pMaterial material resource
 * @param index index of the texture
 * @param pTexture texture
 */
void ResMaterial::ForceBindTexture(nn::g3d::ResMaterial* pMaterial, s32 index,
                                   const nn::gfx::ResTexture* pTexture)
{
    pMaterial->ForceBindTexture(index, makeTextureRef(pTexture));
}

/**
 * Gets the name of a texture of a material resource.
 * @param pMaterial material resource
 * @param index index of the texture
 * @return the name
 */
const char* ResMaterial::GetTextureName(const nn::g3d::ResMaterial* pMaterial, s32 index)
{
    return pMaterial->GetTextureName(index);
}

/**
 * Unbinds a texture from a material resource.
 * @param pMaterial material resource
 * @param index index of the texture
 */
void ResMaterial::ReleaseTexture(nn::g3d::ResMaterial* pMaterial, s32 index)
{
    pMaterial->ReleaseTexture(index);
}

/**
 * Gets a texture bound to a material animation.
 * @param pAnim material animation
 * @param index index of the texture
 * @return the texture, or nullptr if unbound
 */
nn::gfx::ResTexture* ResMaterialAnim::GetTexture(nn::g3d::ResMaterialAnim* pAnim, s32 index)
{
    return toResTexture(pAnim->GetTextureView(index));
}

/**
 * Gets a texture bound to a material animation.
 * @param pAnim material animation
 * @param index index of the texture
 * @return the texture, or nullptr if unbound
 */
const nn::gfx::ResTexture* ResMaterialAnim::GetTexture(const nn::g3d::ResMaterialAnim* pAnim,
                                                       s32 index)
{
    return toResTexture(pAnim->GetTextureView(index));
}

/**
 * Gets the name of a texture of a material animation.
 * @param pAnim material animation
 * @param index index of the texture
 * @return the name
 */
const char* ResMaterialAnim::GetTextureName(const nn::g3d::ResMaterialAnim* pAnim, s32 index)
{
    return pAnim->GetTextureName(index);
}

/**
 * Gets the number of textures of a material animation.
 * @param pAnim material animation
 * @return the number of textures
 */
s32 ResMaterialAnim::GetTextureCount(const nn::g3d::ResMaterialAnim* pAnim)
{
    return pAnim->GetTextureCount();
}

/**
 * Binds a texture to a material animation.
 * @param pAnim material animation
 * @param index index of the texture
 * @param pTexture texture
 */
void ResMaterialAnim::ForceBindTexture(nn::g3d::ResMaterialAnim* pAnim, s32 index,
                                       const nn::gfx::ResTexture* pTexture)
{
    pAnim->ForceBindTexture(index, makeTextureRef(pTexture));
}

/**
 * Unbinds a texture from a material animation.
 * @param pAnim material animation
 * @param index index of the texture
 */
void ResMaterialAnim::ReleaseTexture(nn::g3d::ResMaterialAnim* pAnim, s32 index)
{
    pAnim->ReleaseTexture(index);
}

/**
 * Gets a texture set on a material.
 * @param pMaterial material
 * @param index index of the texture
 * @return the texture, or nullptr if unset
 */
nn::gfx::ResTexture* MaterialObj::GetResTexture(nn::g3d::MaterialObj* pMaterial, s32 index)
{
    return toResTexture(pMaterial->GetTextureView(index));
}

/**
 * Gets a texture set on a material.
 * @param pMaterial material
 * @param index index of the texture
 * @return the texture, or nullptr if unset
 */
const nn::gfx::ResTexture* MaterialObj::GetResTexture(const nn::g3d::MaterialObj* pMaterial,
                                                      s32 index)
{
    return toResTexture(pMaterial->GetTextureView(index));
}

/**
 * Sets a texture of a material animation instance.
 * @param pAnim material animation instance
 * @param index index of the texture
 * @param pTexture texture
 */
void MaterialAnimObj::SetResTexture(nn::g3d::MaterialAnimObj* pAnim, s32 index,
                                    nn::gfx::ResTexture* pTexture)
{
    pAnim->SetTexture(index, makeTextureRef(pTexture));
}

/**
 * Gets a texture of a material animation instance.
 * @param pAnim material animation instance
 * @param index index of the texture
 * @return the texture, or nullptr if unset
 */
nn::gfx::ResTexture* MaterialAnimObj::GetResTexture(nn::g3d::MaterialAnimObj* pAnim, s32 index)
{
    return toResTexture(pAnim->GetTextureView(index));
}

/**
 * Gets a texture of a material animation instance.
 * @param pAnim material animation instance
 * @param index index of the texture
 * @return the texture, or nullptr if unset
 */
const nn::gfx::ResTexture* MaterialAnimObj::GetResTexture(const nn::g3d::MaterialAnimObj* pAnim,
                                                          s32 index)
{
    return toResTexture(pAnim->GetTextureView(index));
}

/**
 * Gets the name of a texture of a material.
 * @param pMaterial material
 * @param index index of the texture
 * @return the name
 */
const char* MaterialObj::GetResTextureName(const nn::g3d::MaterialObj* pMaterial, s32 index)
{
    return pMaterial->GetResource()->GetTextureName(index);
}

/**
 * Sets a texture on a material.
 * @param pMaterial material
 * @param index index of the texture
 * @param pTexture texture
 */
void MaterialObj::SetResTexture(nn::g3d::MaterialObj* pMaterial, s32 index,
                                nn::gfx::ResTexture* pTexture)
{
    pMaterial->SetTexture(index, makeTextureRef(pTexture));
}

/**
 * Resets every texture of a material to the one bound to its resource.
 * @param pMaterial material
 */
void MaterialObj::ClearTexture(nn::g3d::MaterialObj* pMaterial)
{
    const nn::g3d::ResMaterial* pRes = pMaterial->GetResource();
    for (s32 i = 0; i < pRes->GetTextureCount(); i++)
    {
        pMaterial->SetTexture(
            i, nn::g3d::TextureRef(pRes->GetTextureView(i), pRes->GetTextureDescriptorSlot(i)));
    }
}

/**
 * Maps a material uniform block buffer.
 * @param pMaterial material
 * @param bufferIndex buffer index
 * @return the mapped memory
 */
void* MaterialObj::Map(nn::g3d::MaterialObj* pMaterial, s32 bufferIndex)
{
    return pMaterial->GetMaterialBlock(bufferIndex)->Map();
}

/**
 * Flushes and unmaps a material uniform block buffer.
 * @param pMaterial material
 * @param bufferIndex buffer index
 */
void MaterialObj::FlushAndUnmap(nn::g3d::MaterialObj* pMaterial, s32 bufferIndex)
{
    pMaterial->GetMaterialBlock(bufferIndex)->FlushMappedRange(0, pMaterial->GetMaterialBlockSize());
    pMaterial->GetMaterialBlock(bufferIndex)->Unmap();
}

/**
 * Maps a shape uniform block buffer.
 * @param pShape shape
 * @param viewIndex view index
 * @param bufferIndex buffer index
 * @return the mapped memory
 */
void* ShapeObj::Map(nn::g3d::ShapeObj* pShape, s32 viewIndex, s32 bufferIndex)
{
    return pShape->GetShapeBlock(viewIndex, bufferIndex)->Map();
}

/**
 * Flushes and unmaps a shape uniform block buffer.
 * @param pShape shape
 * @param viewIndex view index
 * @param bufferIndex buffer index
 */
void ShapeObj::FlushAndUnmap(nn::g3d::ShapeObj* pShape, s32 viewIndex, s32 bufferIndex)
{
    pShape->GetShapeBlock(viewIndex, bufferIndex)->FlushMappedRange(0, 0x100);
    pShape->GetShapeBlock(viewIndex, bufferIndex)->Unmap();
}

}  // namespace agl::g3d
