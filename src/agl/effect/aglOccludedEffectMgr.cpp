#include "effect/aglOccludedEffectMgr.h"

#include <filedevice/nin/seadNinHostIOFileDevice.h>
#include <filedevice/seadFileDeviceMgr.h>
#include <hostio/seadHostIOPropertyEvent.h>
#include <prim/seadSafeString.h>

#include "common/aglVertexAttribute.h"
#include "detail/aglFileIOMgr.h"
#include "detail/aglGPUMemBlockMgr.h"
#include "detail/aglPrivateResource.h"
#include "detail/aglRootNode.h"
#include "environment/aglEnvObjMgr.h"
#include "g3d/aglG3DDecl.h"
#include "g3d/aglNW4FToNN.h"
#include "g3d/aglTextureDataInitializerG3D.h"
#include "postfx/aglPostFxUtil.h"

namespace agl::fx
{

namespace
{

inline void genMessageDummy(sead::hostio::Context* pContext, const char* pLabel) {}

inline void genMessageDummy(sead::hostio::Context* pContext, const sead::SafeString& rLabel) {}

template <typename T>
inline bool isPropertyOf_(const sead::hostio::PropertyEvent* pEvent, const T* pValue)
{
    if ((pEvent->getType() & 2) != 0)
    {
        return false;
    }

    const void* id = pEvent->getId();
    return id < pValue + 1 && id >= pValue;
}

const char* const cTextureFormatName[] = {
    "???",
    "r8 uN",
    "r8 uI",
    "r8 sN",
    "r8 sI",
    "r16 uN",
    "r16 uI",
    "r16 sN",
    "r16 sI",
    "r16 f",
    "rg8 uN",
    "rg8 uI",
    "rg8 sN",
    "rg8 sI",
    "r5g6b5 uN",
    "a1bgr5 uN",
    "rgba4 uN",
    "rgb5a1 uN",
    "r32 uI",
    "r32 sI",
    "r32 f",
    "rg16 uN",
    "rg16 uI",
    "rg16 sN",
    "rg16 sI",
    "rg16 f",
    "r11g11b10 f",
    "a2bgr10 uN",
    "a2bgr10 uI",
    "rgba8 uN",
    "rgba8 uI",
    "rgba8 sN",
    "rgba8 sI",
    "rgba8 SRGB",
    "rgb10a2 uN",
    "rgb10a2 uI",
    "rg32 uI",
    "rg32 sI",
    "rg32 f",
    "rgba16 uN",
    "rgba16 uI",
    "rgba16 sN",
    "rgba16 sI",
    "rgba16 f",
    "rgba32 uI",
    "rgba32 sI",
    "rgba32 f",
    "BC1 sRGB",
    "BC2 uN",
    "BC2 sRGB",
    "BC3 uN",
    "BC3 sRGB",
    "BC4 uN",
    "BC4 sN",
    "BC5 uN",
    "BC5 sN",
    "Depth16",
    "Depth32",
    "Depth24 Stencil8",
};

const char* const cCompSelName[] = {"0", "1", "r", "g", "b", "a"};

inline const char* getCompSelName(s8 compSel)
{
    if (compSel >= cTextureCompSel_0 && compSel <= cTextureCompSel_A)
    {
        return cCompSelName[compSel];
    }

    return "0";
}

const sead::SafeString cRawDirPath[3] = {"%AGL_ROOT%/tools/temporary", "temp_ofx.bofx",
                                         "temp_ofx.txt"};

}  // namespace

/**
 * Constructs the manager with an empty resource state.
 */
OccludedEffectMgr::OccludedEffectMgr() : utl::IParameterIO("aglofx", 0) {}

/**
 * Releases the resources, texture slots, menus and vertex streams.
 */
OccludedEffectMgr::~OccludedEffectMgr()
{
    mResource[0].release();
    mResource[1].release();

    for (s32 i = 0; i < mTextureInfo.capacity(); i++)
    {
        delete mTextureInfo[i];
    }

    mTextureInfo.freeBuffer();
    mMenuNodePresetRoot.freeBuffer();
    mMenuInstance.freeBuffer();
    mPresetList.freeBuffer();

    mVtxQuad.mVertexBlock.freeBuffer();
    mVtxQuad.mIndexBlock.freeBuffer();
    mVtxQuadDouble.mVertexBlock.freeBuffer();
    mVtxQuadDouble.mIndexBlock.freeBuffer();
    mVtxOctagon.mVertexBlock.freeBuffer();
    mVtxOctagon.mIndexBlock.freeBuffer();
    mVtxOctagonDouble.mVertexBlock.freeBuffer();
    mVtxOctagonDouble.mIndexBlock.freeBuffer();
}

/**
 * Cleans up the resource file unless it is external and marks the resource invalid.
 */
void OccludedEffectMgr::Resource::release()
{
    if (mResFile != nullptr && !mIsExternal)
    {
        g3d::ResFile::Cleanup(nn::g3d::ResFile::ResCast(mResFile));
    }

    mResFile = nullptr;
    mIsValid = false;
}

void OccludedEffectMgr::initialize(const CreateArg& rArg, sead::Heap* pHeap)
{
    mCreateArg = rArg;
    createVtxStream_(pHeap);
    mType = "aglofx";
    mVersion = 0;

    if (mCreateArg.mTypeNum > 0)
    {
        mPresetList.allocBufferAssert(mCreateArg.mTypeNum, pHeap);
    }

    if (mCreateArg.mInstanceMenuNum > 0)
    {
        mMenuInstance.allocBuffer(mCreateArg.mInstanceMenuNum, pHeap);
    }

    if (mCreateArg.mTypeNum > 0)
    {
        mMenuNodePresetRoot.allocBufferAssert(mCreateArg.mTypeNum, pHeap);
    }

    OfxBase::PresetBase::CreateArg presetArg;
    presetArg.mViewNum = rArg.mViewNum;
    OfxBase::CreateArg ofxArg;
    ofxArg.mViewNum = rArg.mViewNum;

    mTextureInfo.allocBuffer(rArg.mTextureNum, pHeap);
    addList(&mTextureList, "TextureList");

    for (s32 i = 0; i < mTextureInfo.capacity(); i++)
    {
        TextureInfo* pInfo = new (pHeap) TextureInfo(i);
        mTextureInfo.pushBack(pInfo);
        mTextureList.addObj(pInfo, sead::FormatFixedSafeString<64>("TextureInfo%d", i));
    }

    for (s32 type = 0; type < mCreateArg.mTypeNum; type++)
    {
        const s32& rOfxTypeId = mCreateArg.mOfxTypeId[type];

        for (s32 i = 0; i < mCreateArg.mOfxNum[type]; i++)
        {
            OfxBase* pOfx = sead::DynamicCast<OfxBase>(mCreateArg.getObjRef_(rOfxTypeId, i));

            if (pOfx != nullptr)
            {
                pOfx->getMgr()->removeObj(pOfx);
                pOfx->initializeOfx(ofxArg, type, nullptr, pHeap, this);
                pOfx->setEditable(false);
            }
        }

        if (mCreateArg.mPresetNum[type] > 0)
        {
            addList(&mPresetList[type], env::EnvObj::getTypeData(rOfxTypeId).mName);
            s32 num = mCreateArg.mPresetNum[type];

            for (s32 i = 0; i < num; i++)
            {
                OfxBase::PresetBase* pPreset = mCreateArg.getPreset(type, i);

                if (pPreset == nullptr)
                {
                    continue;
                }

                pPreset->getMgr()->removeObj(pPreset);
                OfxBase* pOfx = sead::DynamicCast<OfxBase>(mCreateArg.getObjRef_(rOfxTypeId, 0));

                if (pOfx == nullptr)
                {
                    continue;
                }

                pOfx->initializePreset(pPreset, presetArg, i, pHeap);
                pPreset->setEditable(false);
                mPresetList[type].addObj(pPreset, sead::FormatFixedSafeString<32>("%d", i));
            }
        }
    }

    mResource[1].mResTexInfo.tryAllocBuffer(rArg.mResTextureNum, pHeap);
    mResource[0].mResTexInfo.tryAllocBuffer(rArg.mResTextureNum, pHeap);

    mFlag.reset(0x20000);
    mResState = 0;
    mStatus = 0;
    _c60 = "";
    _d78 = "";
    detail::RootNode::setNodeMeta(this, "Icon=EFFECT");
    mFlag.set(3);
}

/**
 * Creates the quad and octagon vertex and index streams and their doubled texture coordinate
 * variants.
 * @param pHeap heap used for allocations
 */
void OccludedEffectMgr::createVtxStream_(sead::Heap* pHeap)
{
    using pfx::detail::getBufferPtr;

    const f32 cEdge = 0.41421354f;
    const f32 cTexMin = 0.29289323f;
    const f32 cTexMax = 0.70710677f;

    mVtxQuad.mVertexBlock.allocBuffer(64, pHeap, 8, MemoryAttribute(0));
    GPUMemAddr<Vtx> quadAddr(mVtxQuad.mVertexBlock, 0);
    getBufferPtr<Vtx>(mVtxQuad.mVertexBlock)[0].mPos = sead::Vector2f(-1.0f, -1.0f);
    getBufferPtr<Vtx>(mVtxQuad.mVertexBlock)[1].mPos = sead::Vector2f(1.0f, -1.0f);
    getBufferPtr<Vtx>(mVtxQuad.mVertexBlock)[2].mPos = sead::Vector2f(-1.0f, 1.0f);
    getBufferPtr<Vtx>(mVtxQuad.mVertexBlock)[3].mPos = sead::Vector2f(1.0f, 1.0f);
    getBufferPtr<Vtx>(mVtxQuad.mVertexBlock)[0].mTexCoord = sead::Vector2f(0.0f, 1.0f);
    getBufferPtr<Vtx>(mVtxQuad.mVertexBlock)[1].mTexCoord = sead::Vector2f(1.0f, 1.0f);
    getBufferPtr<Vtx>(mVtxQuad.mVertexBlock)[2].mTexCoord = sead::Vector2f(0.0f, 0.0f);
    getBufferPtr<Vtx>(mVtxQuad.mVertexBlock)[3].mTexCoord = sead::Vector2f(1.0f, 0.0f);
    mVtxQuad.mVertexBuffer.setUpBuffer(ConstGPUMemVoidAddr(mVtxQuad.mVertexBlock, 0), sizeof(Vtx),
                                       sizeof(Vtx) * 4);
    mVtxQuad.mVertexBuffer.setUpStream(0, VertexStreamFormat(22), 0, false);
    mVtxQuad.mVertexBuffer.setUpStream(1, VertexStreamFormat(22), 8, false);

    mVtxQuad.mIndexBlock.allocBuffer(8, pHeap, 8, MemoryAttribute(0));
    GPUMemAddr<u16> quadIndexAddr(mVtxQuad.mIndexBlock, 0);
    getBufferPtr<u16>(mVtxQuad.mIndexBlock)[0] = 0;
    getBufferPtr<u16>(mVtxQuad.mIndexBlock)[1] = 1;
    getBufferPtr<u16>(mVtxQuad.mIndexBlock)[2] = 2;
    getBufferPtr<u16>(mVtxQuad.mIndexBlock)[3] = 3;
    mVtxQuad.mIndexStream.setUpStream(GPUMemAddr<u16>(mVtxQuad.mIndexBlock, 0), 4);
    mVtxQuad.mIndexStream.setPrimitiveType(NVN_DRAW_PRIMITIVE_TRIANGLE_STRIP);

    mVtxQuadDouble.mVertexBlock.allocBuffer(64, pHeap, 8, MemoryAttribute(0));
    GPUMemAddr<Vtx> quadDoubleAddr(mVtxQuadDouble.mVertexBlock, 0);

    for (s32 i = 0; i < 4; i++)
    {
        getBufferPtr<Vtx>(mVtxQuadDouble.mVertexBlock)[i].mPos =
            getBufferPtr<Vtx>(mVtxQuad.mVertexBlock)[i].mPos;
        getBufferPtr<Vtx>(mVtxQuadDouble.mVertexBlock)[i].mTexCoord =
            getBufferPtr<Vtx>(mVtxQuad.mVertexBlock)[i].mTexCoord * 2.0f;
    }

    mVtxQuadDouble.mVertexBuffer.setUpBuffer(ConstGPUMemVoidAddr(mVtxQuadDouble.mVertexBlock, 0),
                                             sizeof(Vtx), sizeof(Vtx) * 4);
    mVtxQuadDouble.mVertexBuffer.setUpStream(0, VertexStreamFormat(22), 0, false);
    mVtxQuadDouble.mVertexBuffer.setUpStream(1, VertexStreamFormat(22), 8, false);

    mVtxQuadDouble.mIndexBlock.allocBuffer(8, pHeap, 8, MemoryAttribute(0));
    GPUMemAddr<u16> quadDoubleIndexAddr(mVtxQuadDouble.mIndexBlock, 0);

    for (s32 i = 0; i < 4; i++)
    {
        getBufferPtr<u16>(mVtxQuadDouble.mIndexBlock)[i] =
            getBufferPtr<u16>(mVtxQuad.mIndexBlock)[i];
    }

    mVtxQuadDouble.mIndexStream.setUpStream(GPUMemAddr<u16>(mVtxQuadDouble.mIndexBlock, 0), 4);
    mVtxQuadDouble.mIndexStream.setPrimitiveType(NVN_DRAW_PRIMITIVE_TRIANGLE_STRIP);

    mVtxOctagon.mVertexBlock.allocBuffer(128, pHeap, 8, MemoryAttribute(0));
    GPUMemAddr<Vtx> octagonAddr(mVtxOctagon.mVertexBlock, 0);
    getBufferPtr<Vtx>(mVtxOctagon.mVertexBlock)[0].mPos = sead::Vector2f(-cEdge, -1.0f);
    getBufferPtr<Vtx>(mVtxOctagon.mVertexBlock)[1].mPos = sead::Vector2f(cEdge, -1.0f);
    getBufferPtr<Vtx>(mVtxOctagon.mVertexBlock)[2].mPos = sead::Vector2f(1.0f, -cEdge);
    getBufferPtr<Vtx>(mVtxOctagon.mVertexBlock)[3].mPos = sead::Vector2f(1.0f, cEdge);
    getBufferPtr<Vtx>(mVtxOctagon.mVertexBlock)[4].mPos = sead::Vector2f(cEdge, 1.0f);
    getBufferPtr<Vtx>(mVtxOctagon.mVertexBlock)[5].mPos = sead::Vector2f(-cEdge, 1.0f);
    getBufferPtr<Vtx>(mVtxOctagon.mVertexBlock)[6].mPos = sead::Vector2f(-1.0f, cEdge);
    getBufferPtr<Vtx>(mVtxOctagon.mVertexBlock)[7].mPos = sead::Vector2f(-1.0f, -cEdge);
    getBufferPtr<Vtx>(mVtxOctagon.mVertexBlock)[0].mTexCoord = sead::Vector2f(cTexMin, 1.0f);
    getBufferPtr<Vtx>(mVtxOctagon.mVertexBlock)[1].mTexCoord = sead::Vector2f(cTexMax, 1.0f);
    getBufferPtr<Vtx>(mVtxOctagon.mVertexBlock)[2].mTexCoord = sead::Vector2f(1.0f, cTexMax);
    getBufferPtr<Vtx>(mVtxOctagon.mVertexBlock)[3].mTexCoord = sead::Vector2f(1.0f, cTexMin);
    getBufferPtr<Vtx>(mVtxOctagon.mVertexBlock)[4].mTexCoord = sead::Vector2f(cTexMax, 0.0f);
    getBufferPtr<Vtx>(mVtxOctagon.mVertexBlock)[5].mTexCoord = sead::Vector2f(cTexMin, 0.0f);
    getBufferPtr<Vtx>(mVtxOctagon.mVertexBlock)[6].mTexCoord = sead::Vector2f(0.0f, cTexMin);
    getBufferPtr<Vtx>(mVtxOctagon.mVertexBlock)[7].mTexCoord = sead::Vector2f(0.0f, cTexMax);
    mVtxOctagon.mVertexBuffer.setUpBuffer(ConstGPUMemVoidAddr(mVtxOctagon.mVertexBlock, 0),
                                          sizeof(Vtx), sizeof(Vtx) * 8);
    mVtxOctagon.mVertexBuffer.setUpStream(0, VertexStreamFormat(22), 0, false);
    mVtxOctagon.mVertexBuffer.setUpStream(1, VertexStreamFormat(22), 8, false);

    mVtxOctagon.mIndexBlock.allocBuffer(16, pHeap, 8, MemoryAttribute(0));
    GPUMemAddr<u16> octagonIndexAddr(mVtxOctagon.mIndexBlock, 0);
    getBufferPtr<u16>(mVtxOctagon.mIndexBlock)[0] = 0;
    getBufferPtr<u16>(mVtxOctagon.mIndexBlock)[1] = 1;
    getBufferPtr<u16>(mVtxOctagon.mIndexBlock)[2] = 7;
    getBufferPtr<u16>(mVtxOctagon.mIndexBlock)[3] = 2;
    getBufferPtr<u16>(mVtxOctagon.mIndexBlock)[4] = 6;
    getBufferPtr<u16>(mVtxOctagon.mIndexBlock)[5] = 3;
    getBufferPtr<u16>(mVtxOctagon.mIndexBlock)[6] = 5;
    getBufferPtr<u16>(mVtxOctagon.mIndexBlock)[7] = 4;
    mVtxOctagon.mIndexStream.setUpStream(GPUMemAddr<u16>(mVtxOctagon.mIndexBlock, 0), 8);
    mVtxOctagon.mIndexStream.setPrimitiveType(NVN_DRAW_PRIMITIVE_TRIANGLE_STRIP);

    mVtxOctagonDouble.mVertexBlock.allocBuffer(128, pHeap, 8, MemoryAttribute(0));
    GPUMemAddr<Vtx> octagonDoubleAddr(mVtxOctagonDouble.mVertexBlock, 0);

    for (s32 i = 0; i < 8; i++)
    {
        getBufferPtr<Vtx>(mVtxOctagonDouble.mVertexBlock)[i].mPos =
            getBufferPtr<Vtx>(mVtxOctagon.mVertexBlock)[i].mPos;
        getBufferPtr<Vtx>(mVtxOctagonDouble.mVertexBlock)[i].mTexCoord =
            getBufferPtr<Vtx>(mVtxOctagon.mVertexBlock)[i].mTexCoord * 2.0f;
    }

    mVtxOctagonDouble.mVertexBuffer.setUpBuffer(
        ConstGPUMemVoidAddr(mVtxOctagonDouble.mVertexBlock, 0), sizeof(Vtx), sizeof(Vtx) * 8);
    mVtxOctagonDouble.mVertexBuffer.setUpStream(0, VertexStreamFormat(22), 0, false);
    mVtxOctagonDouble.mVertexBuffer.setUpStream(1, VertexStreamFormat(22), 8, false);

    mVtxOctagonDouble.mIndexBlock.allocBuffer(16, pHeap, 8, MemoryAttribute(0));
    GPUMemAddr<u16> octagonDoubleIndexAddr(mVtxOctagonDouble.mIndexBlock, 0);

    for (s32 i = 0; i < 8; i++)
    {
        getBufferPtr<u16>(mVtxOctagonDouble.mIndexBlock)[i] =
            getBufferPtr<u16>(mVtxOctagon.mIndexBlock)[i];
    }

    mVtxOctagonDouble.mIndexStream.setUpStream(GPUMemAddr<u16>(mVtxOctagonDouble.mIndexBlock, 0),
                                               8);
    mVtxOctagonDouble.mIndexStream.setPrimitiveType(NVN_DRAW_PRIMITIVE_TRIANGLE_STRIP);
}

/**
 * Allocates the texture slots of the resource.
 * @param num number of texture slots
 * @param pHeap heap used for allocations
 */
void OccludedEffectMgr::Resource::initialize(s32 num, sead::Heap* pHeap)
{
    mResTexInfo.tryAllocBuffer(num, pHeap);
}

/**
 * Sets up the original resource from a binary and applies its embedded setting.
 * @param pBinary resource binary
 * @param size binary size (unused)
 * @param rName resource name
 * @param isOwned whether the binary is external (not set up by the manager)
 * @return always true
 */
bool OccludedEffectMgr::loadBinary(void* pBinary, u32 size, const sead::SafeString& rName,
                                   bool isOwned)
{
    mFlag.set(0x20000);
    mResState = 1;
    mStatus = 0;
    mResource[0].setupWithBinary(pBinary, rName, isOwned);

    if (mResource[0].mIsValid)
    {
        mFlag.set(0x10000);
    }

    loadOriginalResource_();
    return true;
}

/**
 * Sets up the resource file, its textures and the embedded setting file.
 * @param pBinary resource binary
 * @param rName resource path
 * @param isExternal whether the file is external (skips setup and cleanup)
 * @return always true
 */
bool OccludedEffectMgr::Resource::setupWithBinary(void* pBinary, const sead::SafeString& rName,
                                                  bool isExternal)
{
    release();
    mIsExternal = isExternal;
    mResFile = pBinary;
    nn::g3d::ResFile* pResFile = nn::g3d::ResFile::ResCast(pBinary);

    if (!mIsExternal)
    {
        g3d::ResFile::Setup(pResFile);
    }

    mPath = rName;

    s32 texNum = g3d::ResFile::GetTextureCount(pResFile);
    mTexNum = texNum > mResTexInfo.size() ? mResTexInfo.size() : texNum;

    for (s32 i = 0; i < mTexNum; i++)
    {
        nn::gfx::ResTexture* pTexture = g3d::ResFile::GetTexture(pResFile, i);
        g3d::TextureDataInitializerG3D::initialize(&mResTexInfo[i].mTextureData, *pTexture);
        mResTexInfo[i].mSampler.applyTextureData(mResTexInfo[i].mTextureData);
        mResTexInfo[i].mName.copy(pTexture->ToData().pName.Get()->GetData());
    }

    s32 fileNum = pResFile->GetExternalFileCount();

    for (s32 i = 0; i < fileNum; i++)
    {
        sead::FixedSafeString<256> fileName(pResFile->GetExternalFileName(i));

        if (fileName.findIndex(".baglofx") >= 0)
        {
            mIsValid = true;
            mSettingFile = pResFile->GetExternalFile(i);
            mName = fileName;
            mName.chop(sead::SafeString("baglofx").calcLength());
            mName.append("aglofx");
        }
    }

    return true;
}

/**
 * Switches back to the original resource and applies its embedded setting if present.
 */
void OccludedEffectMgr::loadOriginalResource_()
{
    mResState = 0;
    mStatus = 0;

    if (mFlag.isOn(0x20000))
    {
        mResState = 1;

        if (mFlag.isOn(0x10000))
        {
            mStatus = 2;
            u32 size = mResource[0].mSettingFile->size;
            loadSetting(mResource[0].mSettingFile->pData.Get(), size, nullptr, false);
        }
        else
        {
            mStatus = 1;
        }
    }
}

/**
 * Loads the original resource from a file and applies its embedded setting.
 * @param rPath file path
 * @param pHeap heap used for allocations
 * @return whether the file could be loaded
 */
bool OccludedEffectMgr::loadFile(const sead::SafeString& rPath, sead::Heap* pHeap)
{
    mFlag.set(0x20000);
    mResState = 1;
    mStatus = 0;
    sead::FileDevice* pDevice = sead::FileDeviceMgr::instance()->findDeviceFromPath(rPath, nullptr);

    if (!mResource[0].setupWithFile(pDevice, rPath, pHeap))
    {
        return false;
    }

    if (mResource[0].mIsValid)
    {
        mFlag.set(0x10000);
    }

    loadOriginalResource_();
    return true;
}

/**
 * Loads a resource file and sets it up.
 * @param pDevice file device to load from
 * @param rPath file path
 * @param pHeap heap used for allocations
 * @return whether the file could be loaded
 */
bool OccludedEffectMgr::Resource::setupWithFile(sead::FileDevice* pDevice,
                                                const sead::SafeString& rPath, sead::Heap* pHeap)
{
    sead::FileDevice::LoadArg arg;
    arg.alignment = detail::GPUMemBlockMgr::calcGPUMemoryAlignment(0x2000);
    arg.path = rPath;
    arg.heap = pHeap;
    u8* pData = pDevice->tryLoad(arg);

    if (pData == nullptr)
    {
        return false;
    }

    setupWithBinary(pData, rPath, false);
    return true;
}

/**
 * Releases both resources.
 */
void OccludedEffectMgr::releaseResource()
{
    mResource[0].release();
    mResource[1].release();
    mResState = 0;
    mStatus = 1;
}

/**
 * Rebuilds the enabled instance list if needed and updates every effect instance.
 */
void OccludedEffectMgr::calc()
{
    if (mFlag.isOn(1))
    {
        mMenuInstance.clear();

        for (s32 type = 0; type < mCreateArg.mTypeNum; type++)
        {
            for (s32 i = 0; i < mCreateArg.mOfxNum[type]; i++)
            {
                OfxBase* pOfx = mCreateArg.getInstance(type, i);

                if (pOfx->isEnable())
                {
                    mMenuInstance.pushBack(pOfx);
                }
            }
        }

        mFlag.reset(1);
    }

    for (s32 type = 0; type < mCreateArg.mTypeNum; type++)
    {
        for (s32 i = 0; i < mCreateArg.mOfxNum[type]; i++)
        {
            mCreateArg.getInstance(type, i)->calc();
        }
    }

    if (mFlag.isOn(2))
    {
        constructHostIO_();
        mFlag.reset(2);
    }
}

/**
 * Walks all instances and presets to rebuild the host IO tree (empty in release).
 */
void OccludedEffectMgr::constructHostIO_()
{
    for (s32 type = 0; type < mCreateArg.mTypeNum; type++)
    {
        for (s32 i = 0; i < mCreateArg.mOfxNum[type]; i++)
        {
            mCreateArg.getInstance(type, i);
        }
    }

    for (s32 type = 0; type < mCreateArg.mTypeNum; type++)
    {
        for (s32 i = 0; i < mCreateArg.mOfxNum[type]; i++)
        {
            mCreateArg.getInstance(type, i);
        }
    }

    for (s32 type = 0; type < mCreateArg.mTypeNum; type++)
    {
        for (s32 i = 0; i < mCreateArg.mPresetNum[type]; i++)
        {
            mCreateArg.getPreset(type, i);
        }
    }

    for (s32 type = 0; type < mCreateArg.mTypeNum; type++)
    {
        for (s32 i = 0; i < mCreateArg.mPresetNum[type]; i++)
        {
            mCreateArg.getPreset(type, i);
        }
    }
}

/**
 * Calculates the per-view context of every enabled instance.
 * @param viewIndex view index
 * @param rViewMtx view matrix
 * @param rProjMtx projection matrix
 * @param near near clip distance
 * @param far far clip distance
 * @param fovy vertical field of view
 * @param aspect aspect ratio
 * @param rOffset projection offset
 */
void OccludedEffectMgr::calcView(s32 viewIndex, const sead::Matrix34f& rViewMtx,
                                 const sead::Matrix44f& rProjMtx, f32 near, f32 far, f32 fovy,
                                 f32 aspect, const sead::Vector2f& rOffset)
{
    if (!isDrawable_())
    {
        return;
    }

    for (OfxBase& rOfx : mMenuInstance)
    {
        rOfx.calcContext(viewIndex, rViewMtx, rProjMtx, near, far, fovy, aspect, rOffset);
    }
}

/**
 * Updates GPU resources of every enabled instance.
 */
void OccludedEffectMgr::updateGPU()
{
    if (!isDrawable_())
    {
        return;
    }

    for (OfxBase& rOfx : mMenuInstance)
    {
        rOfx.updateGPU();
    }
}

/**
 * Updates per-view GPU resources of every enabled instance.
 * @param viewIndex view index
 * @param rRenderBuffer render buffer to draw into
 */
void OccludedEffectMgr::updateViewGPU(s32 viewIndex, const RenderBuffer& rRenderBuffer) const
{
    if (!isDrawable_())
    {
        return;
    }

    for (OfxBase& rOfx : mMenuInstance)
    {
        rOfx.updateViewGPU(viewIndex, rRenderBuffer);
    }
}

/**
 * Draws every enabled instance.
 * @param pDrawContext draw context that receives the commands
 * @param viewIndex view index
 * @param rRenderBuffer render buffer to draw into
 * @param rViewport viewport to draw with
 * @param rDepth scene depth target
 */
void OccludedEffectMgr::draw(DrawContext* pDrawContext, s32 viewIndex,
                             const RenderBuffer& rRenderBuffer, const sead::Viewport& rViewport,
                             const RenderTargetDepth& rDepth) const
{
    if (!isDrawable_())
    {
        return;
    }

    for (OfxBase& rOfx : mMenuInstance)
    {
        rOfx.draw(pDrawContext, viewIndex, rRenderBuffer, rViewport, rDepth);
    }
}

/**
 * Draws the debug shapes of every enabled instance.
 * @param pDrawContext draw context that receives the commands
 * @param viewIndex view index
 */
void OccludedEffectMgr::drawDebug(DrawContext* pDrawContext, s32 viewIndex) const
{
    if (!isDrawable_())
    {
        return;
    }

    for (OfxBase& rOfx : mMenuInstance)
    {
        rOfx.drawDebugOfx(pDrawContext, viewIndex);
    }
}

/**
 * Enables or disables every effect instance.
 * @param enable whether to enable
 */
void OccludedEffectMgr::setEnableAll(bool enable)
{
    s32 typeNum = mCreateArg.mTypeNum;

    for (s32 type = 0; type < typeNum; type++)
    {
        for (s32 i = 0; i < mCreateArg.mOfxNum[type]; i++)
        {
            mCreateArg.getInstance(type, i)->setEnable(enable);
        }
    }
}

/**
 * Saves the setting parameters.
 * @param type unused
 * @param rName destination path
 * @param pOut output buffer (selects the empty path when non-null)
 * @return whether saving succeeded
 */
bool OccludedEffectMgr::saveSetting(s32 type, const sead::SafeString& rName,
                                    sead::BufferedSafeString* pOut) const
{
    if (pOut == nullptr)
    {
        return save(rName, 0x2000000);
    }

    return save(sead::SafeString::cEmptyString, 0x2000000);
}

/**
 * Loads the setting parameters and reapplies presets and texture placements.
 * @param pData parameter archive binary
 * @param size binary size (unused)
 * @param pName path of the setting file
 * @param isFile whether to load from pName instead of pData
 */
void OccludedEffectMgr::loadSetting(const void* pData, u32 size, const sead::SafeString* pName,
                                    bool isFile)
{
    initTexturePlacement_();

    if (isFile)
    {
        load(*pName, false);
    }
    else
    {
        utl::ResParameterArchive archive(pData);
        applyResParameterArchive(archive);
    }

    for (s32 type = 0; type < mCreateArg.mTypeNum; type++)
    {
        for (s32 i = 0; i < mCreateArg.mOfxNum[type]; i++)
        {
            mCreateArg.getInstance(type, i)->mFlag.set(0x20);
        }
    }

    mFlag.set(3);
    updateTexturePlacement_();
}

/**
 * Clears the texture references of every texture slot.
 */
void OccludedEffectMgr::initTexturePlacement_()
{
    for (TextureInfo& rInfo : mTextureInfo)
    {
        rInfo.mPlacement.mRefTexName->copy(sead::SafeString::cEmptyString);
        rInfo.mPlacement.mResIndex = -1;
    }
}

/**
 * Binds every texture slot to the matching texture of the current resource.
 */
void OccludedEffectMgr::updateTexturePlacement_()
{
    const Resource& rRes = getCurrRes_();

    for (auto it = mTextureInfo.begin(), end = mTextureInfo.end(); it != end; ++it)
    {
        TextureInfo::Placement& rPlacement = it->mPlacement;
        it->mIsValid = false;

        if (rPlacement.mResIndex >= rRes.mTexNum)
        {
            rPlacement.mResIndex = -1;
            rPlacement.mRefTexName->copy(sead::SafeString::cEmptyString);
            it->mIsValid = false;
        }

        if (rPlacement.mRefTexName->isEqual(sead::SafeString::cEmptyString))
        {
            if (rPlacement.mResIndex == -1)
            {
                it->mName.format("----", it->mIndex);
                continue;
            }
        }
        else if (rPlacement.mResIndex == -1)
        {
            for (auto itRes = rRes.mResTexInfo.begin(), endRes = rRes.mResTexInfo.end();
                 itRes != endRes; ++itRes)
            {
                if (rPlacement.mRefTexName->isEqual(itRes->mName))
                {
                    it->mSampler.applyTextureData(itRes->mSampler.getTextureData());

                    if (*it->mPlacement.mIsQuarter)
                    {
                        it->mSampler.setWrap(6, 6, 6);
                    }
                    else
                    {
                        it->mSampler.setWrap(7, 7, 7);
                    }

                    it->mName.copy(itRes->mName);
                    it->mIsValid = true;
                    rPlacement.mResIndex = itRes.getIndex();
                    break;
                }
            }

            continue;
        }

        it->mName.copy(rRes.mResTexInfo[rPlacement.mResIndex].mName);
        it->mSampler.applyTextureData(
            rRes.mResTexInfo[rPlacement.mResIndex].mSampler.getTextureData());
        if (*it->mPlacement.mIsQuarter)
        {
            it->mSampler.setWrap(6, 6, 6);
        }
        else
        {
            it->mSampler.setWrap(7, 7, 7);
        }

        it->mIsValid = true;
        rPlacement.mRefTexName->copy(rRes.mResTexInfo[rPlacement.mResIndex].mName);
    }
}

/**
 * Searches the presets of a type by name.
 * @param type effect type index
 * @param rName preset name
 * @return matching preset, or null
 */
OfxBase::PresetBase* OccludedEffectMgr::searchPresetByName(s32 type,
                                                           const sead::SafeString& rName) const
{
    for (s32 i = 0; i < mCreateArg.mPresetNum[type]; i++)
    {
        OfxBase::PresetBase* pPreset = mCreateArg.getPreset(type, i);

        if (pPreset != nullptr && pPreset->mPresetName->isEqual(rName))
        {
            return pPreset;
        }
    }

    return nullptr;
}

OfxBase::PresetBase* OccludedEffectMgr::getPresetByIndex(s32 type, s32 index) const
{
    return mCreateArg.getPreset(type, index);
}

/**
 * Loads a setting file chosen in a host file dialog.
 */
void OccludedEffectMgr::loadSettingFromFile()
{
    detail::FileIOMgr::DialogArg arg;
    arg.mFilter = "aglofx";
    arg.mId = "aglofx";
    arg.mOutPath = &mSettingPath;
    s32 handle = detail::FileIOMgr::instance()->load(arg);

    if (handle >= 0)
    {
        detail::FileIOMgr::instance()->close(handle);
        loadSetting(nullptr, 0, &mSettingPath, true);
        mStatus = 5;
    }
}

/**
 * Sets up a vertex attribute for the quad stream.
 * @param pAttr vertex attribute to set up
 * @param pHeap heap used for allocations
 */
void OccludedEffectMgr::getVertexAttrQuad(VertexAttribute* pAttr, sead::Heap* pHeap) const
{
    pAttr->create(2, pHeap);
    pAttr->setVertexStream(0, &mVtxQuad.mVertexBuffer, 0);
    pAttr->setVertexStream(1, &mVtxQuad.mVertexBuffer, 1);
    pAttr->setUp();
}

/**
 * Sets up a vertex attribute for the quad stream with doubled texture coordinates.
 * @param pAttr vertex attribute to set up
 * @param pHeap heap used for allocations
 */
void OccludedEffectMgr::getVertexAttrQuadDouble(VertexAttribute* pAttr, sead::Heap* pHeap) const
{
    pAttr->create(2, pHeap);
    pAttr->setVertexStream(0, &mVtxQuadDouble.mVertexBuffer, 0);
    pAttr->setVertexStream(1, &mVtxQuadDouble.mVertexBuffer, 1);
    pAttr->setUp();
}

/**
 * Sets up a vertex attribute for the octagon stream.
 * @param pAttr vertex attribute to set up
 * @param pHeap heap used for allocations
 */
void OccludedEffectMgr::getVertexAttrOctagon(VertexAttribute* pAttr, sead::Heap* pHeap) const
{
    pAttr->create(2, pHeap);
    pAttr->setVertexStream(0, &mVtxOctagon.mVertexBuffer, 0);
    pAttr->setVertexStream(1, &mVtxOctagon.mVertexBuffer, 1);
    pAttr->setUp();
}

/**
 * Sets up a vertex attribute for the octagon stream with doubled texture coordinates.
 * @param pAttr vertex attribute to set up
 * @param pHeap heap used for allocations
 */
void OccludedEffectMgr::getVertexAttrOctagonDouble(VertexAttribute* pAttr, sead::Heap* pHeap) const
{
    pAttr->create(2, pHeap);
    pAttr->setVertexStream(0, &mVtxOctagonDouble.mVertexBuffer, 0);
    pAttr->setVertexStream(1, &mVtxOctagonDouble.mVertexBuffer, 1);
    pAttr->setUp();
}

bool OccludedEffectMgr::mountRawDir_(bool isSkip, bool isLoadSetting, bool unused)
{
    sead::FileDevice* pDevice = detail::FileIOMgr::instance()->getDevice();
    sead::Heap* pHeap = detail::PrivateResource::instance()->getDebugHeap();
    sead::HeapSafeString dir(pHeap, 0x200);

    if (isSkip)
    {
        return false;
    }

    dir.copy(mRawDir);
    {
        sead::HeapSafeString command(pHeap, 0x4000);
        command.appendWithFormat(
            "File = %%AGL_ROOT%%/tools/bat/convertOfxRes.bat, NoWindow = True, WindowStyle = "
            "Hidden, WaitEnd = True, Dir = %s, Arg = ./ %s/%s",
            dir.cstr(), cRawDirPath[0].cstr(), cRawDirPath[1].cstr());
    }

    if (mRawText != nullptr)
    {
        delete mRawText;
        mRawText = nullptr;
    }

    {
        sead::FileDevice::LoadArg arg;
        arg.path =
            sead::FormatFixedSafeString<256>("%s/%s", cRawDirPath[0].cstr(), cRawDirPath[2].cstr());
        arg.heap = pHeap;
        u8* pText = pDevice->tryLoad(arg);

        if (pText != nullptr)
        {
            mRawText = new (pHeap) sead::BufferedSafeString(new (pHeap) char[0x4000](), 0x4000);
            mRawText->copy(sead::SafeString(reinterpret_cast<const char*>(pText)), arg.read_size);
            return false;
        }
    }

    {
        sead::HeapSafeString path(pHeap, 0x400);
        path.format("%s/%s", cRawDirPath[0].cstr(), cRawDirPath[1].cstr());
        sead::FileDevice::LoadArg arg;
        arg.alignment = detail::GPUMemBlockMgr::calcGPUMemoryAlignment(0x2000);
        arg.path = path;
        arg.heap = pHeap;
        u8* pData = pDevice->tryLoad(arg);

        if (pData == nullptr)
        {
            return false;
        }

        mResource[1].setupWithBinary(pData, path, false);
        mResState = 2;
        mRawDir.copy(dir);
    }

    if (isLoadSetting)
    {
        mStatus = mResource[1].mIsValid ? 4 : 3;

        if (mResource[1].mIsValid)
        {
            u32 size = mResource[1].mSettingFile->size;
            loadSetting(mResource[1].mSettingFile->pData.Get(), size, nullptr, false);
        }
    }

    updateTexturePlacement_();
    return true;
}

/**
 * Copies a parameter into the owned preset of every instance using the named preset.
 * @param type effect type index
 * @param rPresetName preset name
 * @param rParam parameter to copy
 * @return whether any parameter was copied
 */
bool OccludedEffectMgr::setInstanceParameterAll(s32 type, const sead::SafeString& rPresetName,
                                                const utl::ParameterBase& rParam)
{
    bool result = false;

    for (s32 i = 0; i < mCreateArg.mOfxNum[type]; i++)
    {
        OfxBase* pOfx = mCreateArg.getInstance(type, i);

        if (pOfx == nullptr)
        {
            continue;
        }

        OfxBase::PresetBase* pPreset = pOfx->getPreset_<OfxBase::PresetBase>();

        if (!pPreset->mPresetName->isEqual(rPresetName))
        {
            continue;
        }

        for (utl::ParameterBase* p = pPreset->getParamListHead(); p != nullptr; p = p->getNext())
        {
            if (p->copy(rParam))
            {
                result = true;
                break;
            }
        }
    }

    return result;
}

/**
 * Reloads the named preset into every instance using it.
 * @param type effect type index
 * @param rPresetName preset name
 */
void OccludedEffectMgr::updateInstancePresetAll(s32 type, const sead::SafeString& rPresetName)
{
    for (s32 i = 0; i < mCreateArg.mOfxNum[type]; i++)
    {
        OfxBase* pOfx = mCreateArg.getInstance(type, i);

        if (pOfx != nullptr &&
            pOfx->getPreset_<OfxBase::PresetBase>()->mPresetName->isEqual(rPresetName))
        {
            pOfx->loadPresetByName(rPresetName, false);
        }
    }
}

/**
 * Toggles debug drawing of every instance using the named preset.
 * @param type effect type index
 * @param rPresetName preset name
 * @param enable whether to draw debug shapes
 */
void OccludedEffectMgr::setInstanceDebugDrawAll(s32 type, const sead::SafeString& rPresetName,
                                                bool enable)
{
    for (s32 i = 0; i < mCreateArg.mOfxNum[type]; i++)
    {
        OfxBase* pOfx = mCreateArg.getInstance(type, i);

        if (pOfx != nullptr &&
            pOfx->getPreset_<OfxBase::PresetBase>()->mPresetName->isEqual(rPresetName))
        {
            pOfx->mFlag.change(8, enable);
        }
    }
}

/**
 * Sets the debug colors of every instance using the named preset.
 * @param type effect type index
 * @param rPresetName preset name
 * @param rColor0 first debug color
 * @param rColor1 second debug color
 * @param rColor2 third debug color
 */
void OccludedEffectMgr::setInstanceDebugDrawColorAll(s32 type, const sead::SafeString& rPresetName,
                                                     const sead::Color4f& rColor0,
                                                     const sead::Color4f& rColor1,
                                                     const sead::Color4f& rColor2)
{
    for (s32 i = 0; i < mCreateArg.mOfxNum[type]; i++)
    {
        OfxBase* pOfx = mCreateArg.getInstance(type, i);

        if (pOfx != nullptr &&
            pOfx->getPreset_<OfxBase::PresetBase>()->mPresetName->isEqual(rPresetName))
        {
            pOfx->mDebugColor0 = rColor0;
            pOfx->mDebugColor1 = rColor1;
            pOfx->mDebugColor2 = rColor2;
        }
    }
}

/**
 * Generates a texture selection host IO item.
 * @param pContext host IO context
 * @param pIndex texture index being edited
 * @param pLabel label text
 */
void OccludedEffectMgr::genMessageTextureSelect(sead::hostio::Context* pContext, s32* pIndex,
                                                const char* pLabel)
{
    s32 num = mTextureInfo.capacity();

    for (s32 i = 0; i < num; i++)
    {
        sead::FormatFixedSafeString<256> item("%d: %s", i,
                                              mTextureInfo.unsafeAt(i)->mName.getStringTop());
    }
}

void OccludedEffectMgr::genMessage(sead::hostio::Context* pContext)
{
    if (mRawText != nullptr)
    {
        genMessageDummy(pContext, mRawText->cstr());
    }

    switch (mResState)
    {
    case 1:
        genMessageDummy(pContext, mResource[0].mPath.cstr());
        break;
    case 2:
        genMessageDummy(pContext, mRawDir.cstr());
        break;
    }

    switch (mStatus)
    {
    case 2:
        genMessageDummy(pContext, mResource[0].mPath.cstr());
        genMessageDummy(pContext, mResource[0].mName.cstr());
        break;
    case 4:
        genMessageDummy(pContext, mRawDir.cstr());
        genMessageDummy(pContext, mResource[1].mName.cstr());
        break;
    case 5:
        genMessageDummy(pContext, sead::FormatFixedSafeString<512>("%s", mSettingPath.cstr()));
        break;
    }

    if (mResState == 0)
    {
        return;
    }

    switch (mResState)
    {
    case 1:
    case 2:
        break;
    default:
        return;
    }

    const Resource& rRes = getCurrRes_();

    for (auto it = mTextureInfo.begin(), end = mTextureInfo.end(); it != end; ++it)
    {
        genMessageDummy(pContext, sead::FormatFixedSafeString<32>("ID: %d", it->mIndex).cstr());
        genMessageDummy(pContext, sead::FormatFixedSafeString<256>("ID: %d", it->mIndex).cstr());

        for (s32 i = 0; i < rRes.mTexNum; i++)
        {
            genMessageDummy(pContext, rRes.mResTexInfo[i].mName.cstr());
        }
    }

    genMessageDummy(pContext, sead::FormatFixedSafeString<256>(
                                  "GroupHeader = list of textures loaded from resource, Layout = "
                                  "Grid, NumCol = 5, NumRow = %d",
                                  rRes.mTexNum + 1));
    for (s32 i = 0; i < rRes.mTexNum; i++)
    {
        const Resource::ResTexInfo& rInfo = rRes.mResTexInfo[i];
        const TextureData& rData = rInfo.mTextureData;
        genMessageDummy(pContext, sead::FormatFixedSafeString<256>("%s", rInfo.mName.cstr()));
        genMessageDummy(pContext, sead::FormatFixedSafeString<256>("%d x %d", rData.getWidth(0),
                                                                   rData.getHeight(0)));
        genMessageDummy(pContext, sead::FormatFixedSafeString<256>(
                                      "%s", cTextureFormatName[rData.getTextureFormat()]));
        const detail::CompSelData& rCompSel = rData.getSurface().mCompSel;
        genMessageDummy(pContext,
                        sead::FormatFixedSafeString<256>(
                            "%s%s%s%s", getCompSelName(rCompSel.mR), getCompSelName(rCompSel.mG),
                            getCompSelName(rCompSel.mB), getCompSelName(rCompSel.mA)));
        genMessageDummy(pContext, sead::FormatFixedSafeString<256>(
                                      "%.1f KB", rData.getImageByteSize() * (1.0f / 1024.0f)));
    }
}

/**
 * Returns the resource currently in use.
 * @return raw resource when mounted, otherwise the original resource
 */
const OccludedEffectMgr::Resource& OccludedEffectMgr::getCurrRes_() const
{
    if (mResState == 1)
    {
        return mResource[0];
    }

    if (mResState == 2)
    {
        return mResource[1];
    }

    return mResource[0];
}

/**
 * Handles host IO edits of texture slots and the resource commands.
 * @param pEvent property event
 */
void OccludedEffectMgr::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent)
{
    for (TextureInfo& rInfo : mTextureInfo)
    {
        if (isPropertyOf_(pEvent, &rInfo.mPlacement.mResIndex))
        {
            s32 resIndex = rInfo.mPlacement.mResIndex;
            TextureInfo* pTarget = mTextureInfo[rInfo.mIndex];
            pTarget->mPlacement.mRefTexName->copy(sead::SafeString::cEmptyString);
            pTarget->mPlacement.mResIndex = resIndex;
            updateTexturePlacement_();
            break;
        }

        if (isPropertyOf_(pEvent, &*rInfo.mPlacement.mIsQuarter))
        {
            updateTexturePlacement_();
            break;
        }
    }

    switch (pEvent->getIdValue())
    {
    case 101003:
        loadOriginalResource_();
        break;
    case 101004:
    {
        sead::HeapSafeString path(detail::PrivateResource::instance()->getDebugHeap(), 0x200);
        break;
    }
    case 101005:
        mountRawDir_(false, false, false);
        break;
    case 101006:
        releaseResource();
        break;
    case 101007:
        constructHostIO_();
        save(sead::SafeString::cEmptyString, 0x2000000);
        break;
    case 101008:
        constructHostIO_();

        if (mStatus == 5)
        {
            save(mSettingPath, 0x2000000);
        }
        else if (mStatus == 4)
        {
            sead::FormatFixedSafeString<512> path("%s\\%s", mRawDir.cstr(),
                                                  mResource[1].mName.cstr());
            if (save(path, 0x2000000))
            {
                mountRawDir_(false, true, false);
            }
        }

        break;
    case 101009:
        loadSettingFromFile();
        break;
    }
}

/**
 * Generates the host IO instance menu.
 * @param pContext host IO context
 */
void OccludedEffectMgr::genMessageMenuInstance(sead::hostio::Context* pContext)
{
    genMessageDummy(pContext, sead::FormatFixedSafeString<256>(
                                  "GroupHeader = instance list, Layout = Grid, NumCol = 5, "
                                  "NumRow = %d",
                                  mCreateArg.mInstanceMenuNum + 1));
    for (s32 type = 0; type < mCreateArg.mTypeNum; type++)
    {
        for (s32 i = 0; i < mCreateArg.mOfxNum[type]; i++)
        {
            OfxBase* pOfx = mCreateArg.getInstance(type, i);

            if (pOfx == nullptr)
            {
                continue;
            }

            genMessageDummy(pContext, pOfx->getOfxLabel());

            for (s32 j = 0; j < mCreateArg.mPresetNum[type]; j++)
            {
                mCreateArg.getPreset(type, j);
            }

            pOfx->genMessageSimple(pContext);
        }
    }
}

/**
 * Handles host IO edits made in the instance menu.
 * @param pEvent property event
 */
void OccludedEffectMgr::listenPropertyEventMenuInstance(const sead::hostio::PropertyEvent* pEvent)
{
    for (s32 type = 0; type < mCreateArg.mTypeNum; type++)
    {
        for (s32 i = 0; i < mCreateArg.mOfxNum[type]; i++)
        {
            OfxBase* pOfx = mCreateArg.getInstance(type, i);

            if (pOfx == nullptr)
            {
                continue;
            }

            if ((pEvent->getType() & 2) == 0)
            {
                const void* id = pEvent->getId();

                if (id < &pOfx->mPresetIndex + 1 && id >= &pOfx->mPresetIndex)
                {
                    pOfx->mFlag.set(0x40);
                    break;
                }
            }

            pOfx->listenPropertyEventSimple(pEvent);
        }
    }
}

/**
 * Frees the texture slots.
 */
OccludedEffectMgr::Resource::~Resource()
{
    mResTexInfo.freeBuffer();
}

/**
 * Constructs the texture reference parameters of a texture slot.
 * @param index slot index used in the parameter names
 * @param pInfo texture slot that owns the parameters
 */
OccludedEffectMgr::TextureInfo::Placement::Placement(s32 index, TextureInfo* pInfo)
    : mIndex(index), mRefTexName(sead::FixedSafeString<64>(sead::SafeString::cEmptyString),
                                 sead::FormatFixedSafeString<64>("RefTexName%d", index),
                                 "参照テクスチャ名", pInfo),
      mIsQuarter(false, sead::FormatFixedSafeString<64>("IsQuarter%d", index), "1/4テクスチャ",
                 pInfo)
{
}

}  // namespace agl::fx
