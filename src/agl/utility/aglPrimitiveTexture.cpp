#include "utility/aglPrimitiveTexture.h"
#include <hostio/seadHostIOPropertyEvent.h>
#include <prim/seadEndian.h>
#include <prim/seadBitUtil.h>
#include "common/aglGPUMemAddr.h"
#include "common/aglGPUMemBlock.h"
#include "common/aglTextureData.h"
#include "common/aglTextureSampler.h"
#include "detail/aglPrivateResource.h"
#include "driver/aglNVNMgr.h"

namespace agl::utl
{

namespace
{

const char* const cTextureName[PrimitiveTexture::cType_Num] = {
    "Black1D",   "Black1DArray", "Black2D",     "Black2DArray",     "Black3D",
    "BlackCube", "BlackCubeArray", "White2D",   "WhiteCubeArray",   "Zero2D",
    "Zero1D",    "Zero1DArray",  "Zero2DArray", "Zero3D",           "Red2D",
    "Green2D",   "Blue2D",       "Gray2D",      "DarkRed2D",        "DarkGreen2D",
    "DarkBlue2D", "Depth32_0",   "Depth32_1",   "DepthShadow",      "DepthShadowArray",
    "MipLevel",
};

const u32 cMipLevelColor[16] = {
    0xff0000ff, 0x00ff00ff, 0x0000ffff, 0xffff00ff, 0xff00ffff, 0x00ffffff,
    0x7f0000ff, 0x007f00ff, 0x00007fff, 0x7f7f00ff, 0x7f007fff, 0x007f7fff,
    0x3f0000ff, 0x003f00ff, 0x00003fff, 0x000000ff,
};

}  // namespace

SEAD_SINGLETON_DISPOSER_IMPL(PrimitiveTexture)

/**
 * Constructs the primitive texture holder without any textures.
 */
PrimitiveTexture::PrimitiveTexture()
{
    for (s32 i = 0; i < cType_Num; i++)
    {
        mSamplers[i] = nullptr;
    }
}

/**
 * Destroys the textures.
 */
PrimitiveTexture::~PrimitiveTexture()
{
    destroy_();
}

/**
 * Frees the textures and their samplers.
 */
void PrimitiveTexture::destroy_()
{
    for (s32 i = 0; i < cType_Num; i++)
    {
        mSamplers[i]->getTextureData().getImagePtr().deleteGPUMemBlock();
        delete mSamplers[i];
        mSamplers[i] = nullptr;
    }
}

/**
 * Creates the primitive textures and their samplers.
 * @param pHeap heap used for the textures
 */
void PrimitiveTexture::initialize(sead::Heap* pHeap)
{
    mDebugTexturePage.setUp(1, "agl::utl::PrimitiveTexture", pHeap);

    TextureData texture;
    TextureFormat format = TextureFormat::cTextureFormat_R8_G8_B8_A8_uNorm;
    u32 color;

    for (s32 i = 0; i < cType_Num; i++)
    {
        texture.setDebugLabel(cTextureName[i]);

        switch (i)
        {
        case cType_Black1D:
        case cType_Black1DArray:
        case cType_Black2D:
        case cType_Black2DArray:
        case cType_Black3D:
        case cType_BlackCube:
        case cType_BlackCubeArray:
            color = sead::Endian::swapU32(0x000000ff);
            break;
        case cType_White2D:
        case cType_WhiteCubeArray:
            color = sead::Endian::swapU32(0xffffffff);
            break;
        case cType_Zero2D:
        case cType_Zero1D:
        case cType_Zero1DArray:
        case cType_Zero2DArray:
        case cType_Zero3D:
            color = sead::Endian::swapU32(0x00000000);
            break;
        case cType_Red2D:
            color = sead::Endian::swapU32(0xff0000ff);
            break;
        case cType_Green2D:
            color = sead::Endian::swapU32(0x00ff00ff);
            break;
        case cType_Blue2D:
            color = sead::Endian::swapU32(0x0000ffff);
            break;
        case cType_Gray2D:
            color = sead::Endian::swapU32(0x808080ff);
            break;
        case cType_DarkRed2D:
            color = sead::Endian::swapU32(0x8b0000ff);
            break;
        case cType_DarkGreen2D:
            color = sead::Endian::swapU32(0x006400ff);
            break;
        case cType_DarkBlue2D:
            color = sead::Endian::swapU32(0x00008bff);
            break;
        case cType_Depth32_0:
            format = TextureFormat::cTextureFormat_R32_float;
            color = sead::BitUtil::bitCast<u32>(0.0f);
            break;
        case cType_Depth32_1:
        case cType_DepthShadow:
        case cType_DepthShadowArray:
            format = TextureFormat::cTextureFormat_R32_float;
            color = sead::BitUtil::bitCast<u32>(1.0f);
            break;
        case cType_MipLevel:
            break;
        default:
            color = 0;
            break;
        }

        switch (i)
        {
        case cType_Black1D:
        case cType_Zero1D:
            texture.initialize_(TextureType(NVN_TEXTURE_TARGET_1D), format, 4, 1, 1, 1,
                                TextureAttribute(0), MultiSampleType(0), true);
            break;
        case cType_Black1DArray:
        case cType_Zero1DArray:
            texture.initialize_(TextureType(NVN_TEXTURE_TARGET_1D_ARRAY), format, 4, 1, 1, 1,
                                TextureAttribute(0), MultiSampleType(0), true);
            break;
        case cType_Black2DArray:
        case cType_Zero2DArray:
        case cType_DepthShadowArray:
            texture.initialize_(TextureType(NVN_TEXTURE_TARGET_2D_ARRAY), format, 4, 4, 1, 1,
                                TextureAttribute(0), MultiSampleType(0), true);
            break;
        case cType_Black3D:
        case cType_Zero3D:
            texture.initialize_(TextureType(NVN_TEXTURE_TARGET_3D), format, 4, 4, 1, 1,
                                TextureAttribute(0), MultiSampleType(0), true);
            break;
        case cType_BlackCube:
            texture.initialize_(TextureType(NVN_TEXTURE_TARGET_CUBEMAP), format, 4, 4, 6, 1,
                                TextureAttribute(0), MultiSampleType(0), true);
            break;
        case cType_BlackCubeArray:
        case cType_WhiteCubeArray:
            texture.initializeCubeMapArray(format, 4, 4, 1, 1, TextureAttribute(0));
            break;
        case cType_Zero2D:
            texture.initialize_(TextureType(NVN_TEXTURE_TARGET_2D), format, 4, 4, 1, 1,
                                TextureAttribute(0), MultiSampleType(0), true);
            break;
        case cType_MipLevel:
            texture.initialize_(TextureType(NVN_TEXTURE_TARGET_2D),
                                TextureFormat::cTextureFormat_R8_G8_B8_A8_uNorm, 16, 8, 1, 1,
                                TextureAttribute(0), MultiSampleType(0), true);
            break;
        default:
            texture.initialize_(TextureType(NVN_TEXTURE_TARGET_2D), format, 4, 4, 1, 1,
                                TextureAttribute(0), MultiSampleType(0), true);
            break;
        }

        auto* block = new (pHeap, 8) GPUMemBlockU8;
        block->allocBuffer_(texture.getSurface().mStorageSize, pHeap,
                            texture.getSurface().mAlignment, MemoryAttribute(8));
        GPUMemVoidAddr addr(*block, 0);

        if (i == cType_MipLevel)
        {
            u32* image = static_cast<u32*>(addr.getPtr());

            for (u32 y = 0; y < u32(texture.getMipHeight(0)); y++)
            {
                for (u32 x = 0; x < u32(texture.getMipWidth(0)); x++)
                {
                    image[y * texture.getMipWidth(0) + x] = sead::Endian::swapU32(cMipLevelColor[x]);
                }
            }
        }
        else
        {
            u32* image = static_cast<u32*>(addr.getPtr());

            for (u32 j = 0; j < texture.getSurface().mStorageSize / 4; j++)
            {
                image[j] = color;
            }
        }

        texture.setImagePtr(addr, 0);
        driver::NVNMgr::instance()->toTile(&texture);

        mSamplers[i] = new (pHeap, 8) TextureSampler(texture);

        if (i == cType_DepthShadow || i == cType_DepthShadowArray)
        {
            mSamplers[i]->setWrap(7, 7, 7);
            mSamplers[i]->setDepthCompareEnable(true);
            mSamplers[i]->setDepthCompareFunc(8);
        }

        mSamplers[i]->updateRegs();
    }
}

/**
 * Registers the debug texture page.
 */
void PrimitiveTexture::entryDebugPage() {}

/**
 * Generates the host IO message for the debug texture page.
 * @param pContext host IO context
 */
void PrimitiveTexture::genMessage(sead::hostio::Context* pContext)
{
    mDebugTexturePage.genMessagePage(pContext, this);
}

/**
 * Recreates the textures when requested from host IO.
 * @param pEvent property event
 */
void PrimitiveTexture::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent)
{
    if (reinterpret_cast<uintptr_t>(pEvent->getId()) != 0x10000)
    {
        return;
    }

    if (detail::PrivateResource::instance()->getDebugHeap() == nullptr)
    {
        return;
    }

    destroy_();
    initialize(detail::PrivateResource::instance()->getDebugHeap());
}

}  // namespace agl::utl
