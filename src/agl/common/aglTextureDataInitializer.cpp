#include "common/aglTextureDataInitializer.h"
#include <cstring>
#include <heap/seadHeap.h>
#include "common/aglTextureData.h"
#include "common/aglTextureFormatInfo.h"
#include "detail/aglGPUMemBlockMgr.h"
#include "detail/aglPrivateResource.h"
#include "driver/aglNVNMgr.h"

namespace agl {

/**
 * Checks the file header of a GTX file (always valid on this platform).
 * @param pData file data
 * @return true
 */
bool TextureDataInitializerGTX::checkFileHeader(const void* pData)
{
    return true;
}

/**
 * Checks a block header of a GTX file (always valid on this platform).
 * @param pData block data
 * @return true
 */
bool TextureDataInitializerGTX::checkBlockHeader(const void* pData)
{
    return true;
}

/**
 * Checks whether a TGA image can be loaded.
 * @param pData TGA file data
 * @return 0 if supported, 8 otherwise
 */
u32 TextureDataInitializerTGA::checkTGAHeader(const void* pData)
{
    const Header* header = static_cast<const Header*>(pData);

    switch (header->mBitsPerPixel >> 3) {
    case 1:
    case 3:
    case 4:
        break;
    default:
        return 8;
    }

    if (header->mColorMapType != 0) {
        return 8;
    }

    return header->mImageType > 15 ? 8 : 0;
}

/**
 * Checks whether a TGA image can be loaded and has the expected properties.
 * @param pData TGA file data
 * @param pixelByteSize expected bytes per pixel
 * @param width expected width
 * @param height expected height
 * @return a combination of error bits, or 0 if the image matches
 */
u32 TextureDataInitializerTGA::checkTGAHeader(const void* pData, s32 pixelByteSize, s32 width,
                                              s32 height)
{
    const Header* header = static_cast<const Header*>(pData);

    u32 result = checkTGAHeader(pData);

    if ((header->mBitsPerPixel >> 3) != pixelByteSize) {
        result |= 1;
    }

    if (header->mWidth != width) {
        result |= 2;
    }

    if (header->mHeight != height) {
        result |= 4;
    }

    return result;
}

/**
 * Reads the properties of a TGA image.
 * @param pData TGA file data
 */
TextureDataInitializerTGA::TGAData::TGAData(const void* pData, bool)
{
    const Header* header = static_cast<const Header*>(pData);

    mImage = reinterpret_cast<const u8*>(header + 1) + header->mIDLength;
    mWidth = header->mWidth;
    mHeight = header->mHeight;

    const s32 pixel_byte_size = header->mBitsPerPixel >> 3;
    mFormat = TextureFormat::cTextureFormat_R8_G8_B8_A8_uNorm;
    mSrcPixelByteSize = pixel_byte_size;
    mDstPixelByteSize = pixel_byte_size;
    mSrcImageSize = mHeight * mWidth * pixel_byte_size;

    switch (pixel_byte_size) {
    case 1:
        mFormat = TextureFormat::cTextureFormat_R8_uNorm;
        break;
    case 3:
        mFormat = TextureFormat::cTextureFormat_R8_G8_B8_A8_uNorm;
        mDstPixelByteSize = 4;
        break;
    case 4:
        mFormat = TextureFormat::cTextureFormat_R8_G8_B8_A8_uNorm;
        break;
    }

    TextureData texture;
    texture.initialize_(TextureType(NVN_TEXTURE_TARGET_2D), mFormat, mWidth, mHeight, 1, 1,
                        TextureAttribute(0), MultiSampleType(0), true);
    mDstImageSize = texture.getSurface().mStorageSize;
    mIsRLE = (header->mImageType >> 3) & 1;
}

/**
 * Loads a TGA image (not supported for this overload).
 * @param pTextureData texture to initialize
 * @param pData TGA file data
 * @param size size of the file data
 * @param pHeap heap to allocate from
 */
void TextureDataInitializerTGA::initialize(TextureData* pTextureData, void* pData, u64 size,
                                           sead::Heap* pHeap)
{
}

/**
 * Loads a TGA image into a tiled texture.
 * @param pTextureData texture to initialize
 * @param pHeap heap to allocate the image from
 * @param pData TGA file data
 * @param pWorkHeap heap for temporary memory
 */
void TextureDataInitializerTGA::initialize(TextureData* pTextureData, sead::Heap* pHeap,
                                           const void* pData, sead::Heap* pWorkHeap)
{
    TGAData data(pData, false);

    const u64 size = detail::GPUMemBlockMgr::calcGPUMemorySize(data.mDstImageSize);
    const s32 alignment = detail::GPUMemBlockMgr::calcGPUMemoryAlignment(0x2000);
    auto* block = new (pHeap, 8) GPUMemBlockU8;
    block->allocBuffer_(size, pHeap, alignment, MemoryAttribute::CpuCached);
    GPUMemVoidAddr image_ptr(*block, 0);
    initializeTiling_(pTextureData, image_ptr, data, pWorkHeap);
}

/**
 * Initializes a tiled texture from TGA data.
 * @param pTextureData texture to initialize
 * @param imagePtr image memory
 * @param rData decoded TGA data
 * @param pWorkHeap heap for temporary buffers
 */
void TextureDataInitializerTGA::initializeTiling_(TextureData* pTextureData,
                                                  GPUMemVoidAddr imagePtr, const TGAData& rData,
                                                  sead::Heap* pWorkHeap)
{
    if (pWorkHeap == nullptr) {
        pWorkHeap = detail::PrivateResource::instance()->getWorkHeap();
    }

    u8* image = static_cast<u8*>(
        pWorkHeap->tryAlloc(detail::GPUMemBlockMgr::calcGPUMemorySize(rData.mDstImageSize),
                            detail::GPUMemBlockMgr::calcGPUMemoryAlignment(0x2000)));
    TGAData::copy(image, rData.mImage, rData.mSrcPixelByteSize, rData.mWidth, rData.mHeight,
                  rData.mIsRLE, rData.mWidth);

    pTextureData->initialize_(TextureType(NVN_TEXTURE_TARGET_2D), rData.mFormat, rData.mWidth,
                              rData.mHeight, 1, 1, TextureAttribute(0), MultiSampleType(0), true);
    pTextureData->setImagePtr(imagePtr);
    driver::NVNMgr::instance()->toTile(pTextureData, image);
    pWorkHeap->free(image);
}

/**
 * Loads a TGA image into a linear texture (not supported for this overload).
 * @param pTextureData texture to initialize
 * @param pData TGA file data
 * @param size size of the file data
 * @param pHeap heap to allocate from
 */
void TextureDataInitializerTGA::initializeLinearAligned(TextureData* pTextureData, void* pData,
                                                        u64 size, sead::Heap* pHeap)
{
}

/**
 * Loads a TGA image into a linear texture.
 * @param pTextureData texture to initialize
 * @param pHeap heap to allocate the image from
 * @param pData TGA file data
 * @param pWorkHeap heap for temporary memory
 */
void TextureDataInitializerTGA::initializeLinearAligned(TextureData* pTextureData,
                                                        sead::Heap* pHeap, const void* pData,
                                                        sead::Heap* pWorkHeap)
{
    TGAData data(pData, false);

    const u64 size = detail::GPUMemBlockMgr::calcGPUMemorySize(data.mDstImageSize);
    const s32 alignment = detail::GPUMemBlockMgr::calcGPUMemoryAlignment(0x2000);
    auto* block = new (pHeap, 8) GPUMemBlockU8;
    block->allocBuffer_(size, pHeap, alignment, MemoryAttribute::CpuCached);
    GPUMemVoidAddr image_ptr(*block, 0);
    initializeLinearAligned_(pTextureData, image_ptr, data, pWorkHeap);
}

/**
 * Initializes a linear texture from TGA data.
 * @param pTextureData texture to initialize
 * @param imagePtr image memory
 * @param rData decoded TGA data
 * @param pWorkHeap heap for temporary buffers
 */
void TextureDataInitializerTGA::initializeLinearAligned_(TextureData* pTextureData,
                                                         GPUMemVoidAddr imagePtr,
                                                         const TGAData& rData,
                                                         sead::Heap* pWorkHeap)
{
    pTextureData->initialize_(TextureType(NVN_TEXTURE_TARGET_2D), rData.mFormat, rData.mWidth,
                              rData.mHeight, 1, 1, TextureAttribute(1), MultiSampleType(0), true);

    const u32 stride = pTextureData->getSurface().mStride /
                       TextureFormatInfo::getPixelByteSize(TextureFormat(pTextureData->getTextureFormat()));
    TGAData::copy(static_cast<u8*>(imagePtr.getPtr()), rData.mImage, rData.mSrcPixelByteSize,
                  rData.mWidth, rData.mHeight, rData.mIsRLE, stride);

    pTextureData->setImagePtr(imagePtr);
    pTextureData->flushCPUCache();
}

/**
 * Sets up a texture from raw image data that is converted to the tiled layout.
 * @param pTextureData texture to initialize
 * @param imagePtr image storage holding the linear image
 * @param size size of the image
 * @param format texture format
 * @param width width in pixels
 * @param height height in pixels
 * @param pHeap heap to allocate from
 */
void TextureDataInitializerRAW::initialize(TextureData* pTextureData, GPUMemVoidAddr imagePtr,
                                           u64 size, TextureFormat format, s32 width, s32 height,
                                           sead::Heap* pHeap)
{
    pTextureData->initialize_(TextureType(NVN_TEXTURE_TARGET_2D), format, width, height, 1, 1,
                              TextureAttribute(0), MultiSampleType(0), true);
    pTextureData->setImagePtr(imagePtr);
    pTextureData->flushCPUCache();
    driver::NVNMgr::instance()->toTile(pTextureData);
}

/**
 * Copies a linear image into a texture and converts it to the tiled layout.
 * @param pTextureData texture to update
 * @param image linear image
 * @param size size of the image
 */
void TextureDataInitializerRAW::copyTileImage(TextureData* pTextureData,
                                              ConstGPUMemVoidAddr image, u32 size)
{
    if (pTextureData->getImagePtr().getPtr() != image.getPtr()) {
        std::memcpy(pTextureData->getImagePtr().getPtr(), image.getPtr(),
                    pTextureData->getSurface().mStorageSize);
    }

    pTextureData->flushCPUCache();
    driver::NVNMgr::instance()->toTile(pTextureData);
}

}  // namespace agl
