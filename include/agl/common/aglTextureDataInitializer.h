#pragma once

#include <basis/seadTypes.h>
#include "common/aglGPUMemAddr.h"
#include "common/aglGPUMemBlock.h"
#include "common/aglTextureEnum.h"

namespace sead {
class Heap;
}

namespace agl {

class TextureData;

class TextureDataInitializerGTX {
public:
    static bool checkFileHeader(const void* pData);
    static bool checkBlockHeader(const void* pData);
    static void initialize(TextureData* pTextureData, void* pData, u32 size,
                           GPUMemBlock<u8>** ppBlock, sead::Heap* pHeap);
};

class TextureDataInitializerTGA {
public:
    struct Header {
        u8 mIDLength;
        u8 mColorMapType;
        u8 mImageType;
        u8 mColorMapSpec[5];
        u16 mOriginX;
        u16 mOriginY;
        u16 mWidth;
        u16 mHeight;
        u8 mBitsPerPixel;
        u8 mDescriptor;
    };
    static_assert(sizeof(Header) == 0x12);

    class TGAData {
    public:
        TGAData(const void* pData, bool);

        static void copy(u8* pDst, const u8* pSrc, s32 pixelByteSize, s32 width, s32 height,
                         bool isRLE, s32 dstStride);

        s32 mWidth;
        s32 mHeight;
        const u8* mImage;
        TextureFormat mFormat;
        s32 mSrcPixelByteSize;
        s32 mDstPixelByteSize;
        u32 mSrcImageSize;
        u32 mDstImageSize;
        bool mIsRLE;
    };

    static u32 checkTGAHeader(const void* pData);
    static u32 checkTGAHeader(const void* pData, s32 pixelByteSize, s32 width, s32 height);
    static void initialize(TextureData* pTextureData, void* pData, u64 size, sead::Heap* pHeap);
    static void initialize(TextureData* pTextureData, sead::Heap* pHeap, const void* pData,
                           sead::Heap* pWorkHeap);
    static void initializeLinearAligned(TextureData* pTextureData, void* pData, u64 size,
                                        sead::Heap* pHeap);
    static void initializeLinearAligned(TextureData* pTextureData, sead::Heap* pHeap,
                                        const void* pData, sead::Heap* pWorkHeap);

private:
    static void initializeTiling_(TextureData* pTextureData, GPUMemVoidAddr imagePtr,
                                  const TGAData& rData, sead::Heap* pWorkHeap);
    static void initializeLinearAligned_(TextureData* pTextureData, GPUMemVoidAddr imagePtr,
                                         const TGAData& rData, sead::Heap* pWorkHeap);
};

class TextureDataInitializerRAW {
public:
    static void initialize(TextureData* pTextureData, GPUMemVoidAddr imagePtr, u64 size,
                           TextureFormat format, s32 width, s32 height, sead::Heap* pHeap);
    static void copyTileImage(TextureData* pTextureData, ConstGPUMemVoidAddr image, u32 size);
};

}  // namespace agl
