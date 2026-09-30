#include "common/aglTextureDataImageAccessor.h"

#include <gfx/seadGraphicsContext.h>
#include <gfx/seadViewport.h>
#include <nvn/nvn_FuncPtrInline.h>

#include "common/aglDrawContext.h"
#include "common/aglTextureFormatInfo.h"
#include "detail/aglPrivateResource.h"
#include "driver/aglGraphicsDriverMgr.h"
#include "utility/aglImageFilter2D.h"

namespace agl {

namespace {

inline u32 getWidth(const TextureData& rTexture)
{
    u32 width = rTexture.getWidth();
    return width > 1 ? width : 1;
}

inline u32 getHeight(const TextureData& rTexture)
{
    s32 min = rTexture.getMinHeight_();
    s32 height = rTexture.getHeight();
    return height < min ? min : height;
}

template <typename T>
inline T& getElement(sead::Vector4<T>* pVec, s32 index)
{
    switch (index)
    {
    case 0:
        return pVec->x;
    case 1:
        return pVec->y;
    case 2:
        return pVec->z;
    case 3:
        return pVec->w;
    default:
        return pVec->w;
    }
}

inline f32 bitCastFloat(u32 bits)
{
    f32 value;
    __builtin_memcpy(&value, &bits, sizeof(value));
    return value;
}

inline f32 convertF10ToF32(u32 value)
{
    u32 exponent = (value >> 5) & 0x1f;
    u32 mantissa = value & 0x1f;

    if (exponent != 0)
    {
        exponent += 0x70;
    }
    else if (mantissa != 0)
    {
        exponent = 0x71;
        do
        {
            mantissa <<= 1;
            exponent--;
        } while ((mantissa & 0x20) == 0);
        mantissa &= 0x1f;
    }

    return bitCastFloat((exponent & 0xff) << 23 | mantissa << 18);
}

inline f32 convertF11ToF32(u32 value)
{
    u32 exponent = (value >> 6) & 0x1f;
    u32 mantissa = value & 0x3f;

    if (exponent != 0)
    {
        exponent += 0x70;
    }
    else if (mantissa != 0)
    {
        exponent = 0x71;
        do
        {
            mantissa <<= 1;
            exponent--;
        } while ((mantissa & 0x40) == 0);
        mantissa &= 0x3f;
    }

    return bitCastFloat((exponent & 0xff) << 23 | mantissa << 17);
}

inline f32 convertF16ToF32(u32 value)
{
    u32 sign = (value << 16) & 0x80000000;
    u32 exponent = (value >> 10) & 0x1f;
    u32 mantissa = value & 0x3ff;

    if (exponent != 0)
    {
        exponent += 0x70;
    }
    else if (mantissa != 0)
    {
        exponent = 0x71;
        do
        {
            mantissa <<= 1;
            exponent--;
        } while ((mantissa & 0x400) == 0);
        mantissa &= 0x3ff;
    }

    return bitCastFloat(sign | mantissa << 13 | (exponent & 0xff) << 23);
}

}  // namespace

/**
 * Constructs an accessor without an image buffer.
 */
TextureDataImageAccessor::TextureDataImageAccessor() = default;

/**
 * Releases the image buffer.
 */
TextureDataImageAccessor::~TextureDataImageAccessor()
{
    finalizeImageBuffer();
}

/**
 * Releases the image buffer unless it belongs to an external linear texture.
 */
void TextureDataImageAccessor::finalizeImageBuffer()
{
    if (!mIsLinearTextureSet && mImageAddr.isValid())
    {
        mImageAddr.deleteGPUMemBlock();
    }

    mIsLinearTextureSet = false;
    mImageAddr.invalidate();
}

/**
 * Allocates a linear image buffer matching a texture and sets it up as a render target.
 * @param rTexture texture to read from
 * @param pHeap heap to allocate from, or nullptr for the agl work heap
 */
void TextureDataImageAccessor::initializeImageBuffer(const TextureData& rTexture,
                                                     sead::Heap* pHeap)
{
    finalizeImageBuffer();

    TextureData texture;
    mFormat = TextureFormat(rTexture.getTextureFormat());

    switch (mFormat)
    {
    case TextureFormat::cTextureFormat_BC1_uNorm:
    case TextureFormat::cTextureFormat_BC2_uNorm:
    case TextureFormat::cTextureFormat_BC3_uNorm:
    case TextureFormat::cTextureFormat_BC7_uNorm:
        mFormat = TextureFormat::cTextureFormat_R8_G8_B8_A8_uNorm;
        break;
    case TextureFormat::cTextureFormat_BC1_SRGB:
    case TextureFormat::cTextureFormat_BC2_SRGB:
    case TextureFormat::cTextureFormat_BC3_SRGB:
    case TextureFormat::cTextureFormat_BC7_SRGB:
        mFormat = TextureFormat::cTextureFormat_R8_G8_B8_A8_SRGB;
        break;
    case TextureFormat::cTextureFormat_BC4_uNorm:
        mFormat = TextureFormat::cTextureFormat_R8_uNorm;
        break;
    case TextureFormat::cTextureFormat_BC4_sNorm:
        mFormat = TextureFormat::cTextureFormat_R8_sNorm;
        break;
    case TextureFormat::cTextureFormat_BC5_uNorm:
        mFormat = TextureFormat::cTextureFormat_R8_G8_uNorm;
        break;
    case TextureFormat::cTextureFormat_BC5_sNorm:
        mFormat = TextureFormat::cTextureFormat_R8_G8_sNorm;
        break;
    case TextureFormat::cTextureFormat_Depth_16:
        mFormat = TextureFormat::cTextureFormat_R16_float;
        break;
    case TextureFormat::cTextureFormat_Depth_32:
        mFormat = TextureFormat::cTextureFormat_R32_float;
        break;
    default:
        break;
    }

    texture.initialize_(TextureType(1), mFormat, getWidth(rTexture), getHeight(rTexture), 1, 1,
                        TextureAttribute(1), MultiSampleType(0), true);

    if (pHeap == nullptr)
    {
        pHeap = detail::PrivateResource::instance()->getWorkHeap();
    }

    u32 alignment = texture.getSurface().mAlignment;
    u32 size = texture.getSurface().mStorageSize;
    auto* pBlock = new (pHeap) GPUMemBlock<u8>;
    pBlock->allocBuffer_(size, pHeap, alignment, MemoryAttribute::CpuCached);
    mImageAddr = GPUMemAddrBase(*pBlock, 0);
    texture.setImagePtr(mImageAddr);

    mRenderTarget.applyTextureData(texture);
    updateLinearInfo_(texture);
}

/**
 * Updates the row stride from a linear texture.
 * @param rTexture linear texture
 */
void TextureDataImageAccessor::updateLinearInfo_(const TextureData& rTexture)
{
    mStride = rTexture.getSurface().mStride;
}

/**
 * Uses the image memory of an existing linear texture instead of an own buffer.
 * @param pTexture linear texture to read
 */
void TextureDataImageAccessor::setLinearTexture(TextureData* pTexture)
{
    mImageAddr = pTexture->getImagePtr();
    mFormat = TextureFormat(pTexture->getTextureFormat());
    updateLinearInfo_(*pTexture);
    mIsLinearTextureSet = true;
}

/**
 * Copies a texture into the linear image buffer by drawing it.
 * @param pDrawContext draw context
 * @param rTexture texture to copy
 */
void TextureDataImageAccessor::updateImageBuffer(DrawContext* pDrawContext,
                                                 const TextureData& rTexture) const
{
    RenderBuffer& rRenderBuffer = mRenderBuffer;
    f32 width = getWidth(rTexture);
    f32 height = getHeight(rTexture);
    rRenderBuffer.setVirtualSize(sead::Vector2f(width, height));
    rRenderBuffer.setPhysicalArea(0.0f, 0.0f, width, height);
    rRenderBuffer.setRenderTargetColor(&mRenderTarget);
    rRenderBuffer.bind(pDrawContext);

    sead::Viewport viewport(rRenderBuffer);
    viewport.apply(pDrawContext, rRenderBuffer);

    sead::GraphicsContext graphicsContext;
    graphicsContext.setDepthEnable(false, false);
    graphicsContext.setBlendEnable(false);
    graphicsContext.apply(pDrawContext);

    mSampler.applyTextureData(rTexture);
    utl::ImageFilter2D::drawTexture(pDrawContext, mSampler, viewport, sead::Vector2f::ones,
                                    sead::Vector2f::zero);
    mRenderTarget.invalidateGPUCache(pDrawContext);
}

/**
 * Does nothing.
 * @param rTexture unused
 */
void TextureDataImageAccessor::syncMemoryGL_(const TextureData& rTexture) const {}

/**
 * Starts reading a texture on the CPU by copying it into the linear image buffer.
 * @param pDrawContext draw context
 * @param rTexture texture to read
 * @param pHeap heap to allocate the image buffer from
 */
void TextureDataImageAccessor::beginPeek(DrawContext* pDrawContext, const TextureData& rTexture,
                                         sead::Heap* pHeap)
{
    initializeImageBuffer(rTexture, pHeap);
    updateImageBuffer(pDrawContext, rTexture);
    driver::GraphicsDriverMgr::instance()->waitDrawDone(pDrawContext);
    mRenderTarget.invalidateCPUCache();
}

/**
 * Waits for the GPU and invalidates the CPU cache of the image buffer.
 * @param pDrawContext draw context
 */
void TextureDataImageAccessor::sync(DrawContext* pDrawContext) const
{
    driver::GraphicsDriverMgr::instance()->waitDrawDone(pDrawContext);
    mRenderTarget.invalidateCPUCache();
}

/**
 * Reads the components of a pixel from the image buffer as floats.
 * @param pColor receives the components, defaulting missing ones to their default selector
 * @param x pixel column
 * @param y pixel row
 */
void TextureDataImageAccessor::peek(sead::Vector4f* pColor, s32 x, s32 y) const
{
    sead::Vector4<u32> raw;
    peek(&raw, x, y);

    for (s32 i = 0; i < 4; i++)
    {
        if (i < TextureFormatInfo::getComponentNum(mFormat))
        {
            if (TextureFormatInfo::isFloat(mFormat))
            {
                switch (TextureFormatInfo::getComponentBitSize(mFormat, i))
                {
                case 10:
                    getElement(pColor, i) = convertF10ToF32(getElement(&raw, i));
                    break;
                case 11:
                    getElement(pColor, i) = convertF11ToF32(getElement(&raw, i));
                    break;
                case 16:
                    getElement(pColor, i) = convertF16ToF32(getElement(&raw, i));
                    break;
                case 32:
                    getElement(pColor, i) = bitCastFloat(getElement(&raw, i));
                    break;
                default:
                    break;
                }
            }
            else
            {
                bool isNormalized = TextureFormatInfo::isNormalized(mFormat);
                bool isUnsigned = TextureFormatInfo::isUnsigned(mFormat);
                u32 value = getElement(&raw, i);

                if (isNormalized)
                {
                    f32 valueF = value;
                    u8 bitSize = TextureFormatInfo::getComponentBitSize(mFormat, i);

                    if (isUnsigned)
                    {
                        getElement(pColor, i) = valueF / (f32(1 << bitSize) + -1.0f);
                    }
                    else
                    {
                        f32 normalized = valueF / (f32(1 << (bitSize - 1)) + -1.0f);
                        getElement(pColor, i) = normalized > -1.0f ? normalized : -1.0f;
                    }
                }
                else if (isUnsigned)
                {
                    getElement(pColor, i) = f32(value);
                }
                else
                {
                    getElement(pColor, i) = f32(s32(value));
                }
            }
        }
        else
        {
            getElement(pColor, i) =
                TextureFormatInfo::getDefaultCompSel(mFormat, i) == cTextureCompSel_1 ? 1.0f :
                                                                                       0.0f;
        }
    }
}

/**
 * Reads the raw components of a pixel from the image buffer.
 * @param pColor receives the components, zero for missing ones
 * @param x pixel column
 * @param y pixel row
 */
void TextureDataImageAccessor::peek(sead::Vector4<u32>* pColor, s32 x, s32 y) const
{
    u8 pixelByteSize = TextureFormatInfo::getPixelByteSize(mFormat);
    const u32* pData = static_cast<const u32*>(
        GPUMemVoidAddr(mImageAddr, pixelByteSize * x + mStride * y).getPtr());

    u32 data = 0;

    switch (pixelByteSize)
    {
    case 1:
        data = *reinterpret_cast<const u8*>(pData) << 24;
        pData = &data;
        break;
    case 2:
    {
        const u8* pByte = reinterpret_cast<const u8*>(pData);
        data = pByte[1] << 24 | pByte[0] << 16;
        pData = &data;
        break;
    }
    default:
        break;
    }

    u32 shift = 0;

    for (s32 i = 0; i < 4; i++)
    {
        if (i < TextureFormatInfo::getComponentNum(mFormat))
        {
            u8 order = TextureFormatInfo::getComponentOrder(mFormat, i);
            u8 bitSize = TextureFormatInfo::getComponentBitSize(mFormat, order);
            getElement(pColor, order) = (*pData << shift) >> (32 - bitSize);
            shift += bitSize;

            if (shift == 32)
            {
                shift = 0;
                pData++;
            }
        }
        else
        {
            getElement(pColor, i) = 0;
        }
    }
}

}  // namespace agl
