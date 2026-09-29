#include "g3d/aglTextureDataInitializerG3D.h"

#include "common/aglTextureData.h"
#include "detail/aglTextureDataUtil.h"
#include "g3d/aglG3DDecl.h"
#include "g3d/aglNW4FToNN.h"

namespace agl::g3d {

/**
 * Initializes a texture from a texture of a bfres file.
 * @param pTextureData texture to initialize
 * @param pFile bfres file
 * @param index index of the texture
 */
void TextureDataInitializerG3D::initialize(TextureData* pTextureData, void* pFile, s32 index)
{
    nn::g3d::ResFile* pResFile = nn::g3d::ResFile::ResCast(pFile);
    ResFile::Setup(pResFile);
    initialize(pTextureData, *ResFile::GetTexture(pResFile, index));
}

/**
 * Initializes a texture from a texture resource.
 * @param pTextureData texture to initialize
 * @param rResTexture texture resource
 */
void TextureDataInitializerG3D::initialize(TextureData* pTextureData,
                                           nn::gfx::ResTexture& rResTexture)
{
    nn::gfx::ResTextureData& rData = rResTexture.ToData();
    const nn::gfx::TextureInfoData& rInfo = rData.textureInfoData;
    u32 mipCount = rInfo.mipCount;
    u32 width = rInfo.width;
    u32 height = rInfo.height;
    u32 depth = rInfo.depth;
    u32 arrayLength = rInfo.arrayLength;
    TextureFormat format =
        detail::TextureDataUtil::convFormatNNGfxToAGL(nn::gfx::ImageFormat(rInfo.imageFormat));

    switch (rData.imageDimension)
    {
    case nn::gfx::ImageDimension_1d:
        pTextureData->initialize_(TextureType(0), format, width, 1, 1, mipCount, TextureAttribute(),
                                  MultiSampleType(), true);
        break;
    case nn::gfx::ImageDimension_2d:
        pTextureData->initialize_(TextureType(1), format, width, height, 1, mipCount,
                                  TextureAttribute(), MultiSampleType(), true);
        break;
    case nn::gfx::ImageDimension_3d:
        pTextureData->initialize_(TextureType(2), format, width, height, depth, mipCount,
                                  TextureAttribute(), MultiSampleType(), true);
        break;
    case nn::gfx::ImageDimension_CubeMap:
        pTextureData->initialize_(TextureType(8), format, width, height, 6, mipCount,
                                  TextureAttribute(), MultiSampleType(), true);
        break;
    case nn::gfx::ImageDimension_1dArray:
        pTextureData->initialize_(TextureType(3), format, width, arrayLength, 1, mipCount,
                                  TextureAttribute(), MultiSampleType(), true);
        break;
    case nn::gfx::ImageDimension_2dArray:
        pTextureData->initialize_(TextureType(4), format, width, height, arrayLength, mipCount,
                                  TextureAttribute(), MultiSampleType(), true);
        break;
    case nn::gfx::ImageDimension_CubeMapArray:
        pTextureData->initializeCubeMapArray(format, width, height, arrayLength / 6, mipCount,
                                             TextureAttribute());
        break;
    default:
        break;
    }

    nn::gfx::ResTextureContainerData* pContainer = rData.pResTextureContainerData.Get();
    s32 textureId = rData.userDescriptorSlot.value;
    GPUMemVoidAddr imagePtr(*static_cast<detail::MemoryPool**>(pContainer->pCurrentMemoryPool.Get()),
                            static_cast<u8*>(rData.pMipPtrArray.Get()->Get()) -
                                (static_cast<u8*>(pContainer->pTextureData.Get()) + 0x10));
    const NVNtexture* pTexture =
        *reinterpret_cast<NVNtexture* const*>(static_cast<u8*>(rData.pTextureView.Get()) + 8);
    pTextureData->setImagePtr(imagePtr, 1);
    pTextureData->getTexture().setDirect(*pTexture, textureId);
}

/**
 * Initializes a texture from a named texture of a bfres file.
 * @param pTextureData texture to initialize
 * @param pFile bfres file
 * @param rName name of the texture
 */
void TextureDataInitializerG3D::initialize(TextureData* pTextureData, void* pFile,
                                           const sead::SafeString& rName)
{
    nn::g3d::ResFile* pResFile = nn::g3d::ResFile::ResCast(pFile);
    ResFile::Setup(pResFile);
    initialize(pTextureData, *ResFile::GetTexture(pResFile, rName.cstr()));
}

}  // namespace agl::g3d
