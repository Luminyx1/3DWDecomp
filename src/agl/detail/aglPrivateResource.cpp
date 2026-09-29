#include "detail/aglPrivateResource.h"

#include <cstring>
#include <filedevice/seadArchiveFileDevice.h>
#include <heap/seadExpHeap.h>
#include <resource/seadArchiveRes.h>

#include "common/aglGPUMemAddr.h"
#include "common/aglTextureData.h"
#include "common/aglTextureDataInitializer.h"
#include "common/aglTextureSampler.h"
#include "utility/aglParameterStringMgr.h"

namespace agl::detail {

namespace {

/**
 * Default debug print callback that discards the message.
 * @param rString message
 */
void debugPrint(const sead::SafeString& rString) {}

}  // namespace

SEAD_SINGLETON_DISPOSER_IMPL(PrivateResource)

/**
 * Constructs the private resource with the default debug print callback.
 */
PrivateResource::PrivateResource() : mDebugPrintFn(debugPrint) {}

/**
 * Destroys the cursor sampler and the parameter string manager.
 */
PrivateResource::~PrivateResource()
{
    if (mCursorTextureSampler)
    {
        mCursorTextureSampler->~TextureSampler();
    }

    if (utl::ParameterStringMgr::instance())
    {
        utl::ParameterStringMgr::deleteInstance();
    }
}

/**
 * Creates the work heap and allocates the cursor texture memory.
 * @param pHeap heap to allocate from
 * @param pDebugHeap debug heap
 * @param workHeapSize size of the work heap
 * @param unused unused size
 */
void PrivateResource::initialize(sead::Heap* pHeap, sead::Heap* pDebugHeap, u64 workHeapSize,
                                 u64 unused)
{
    mWorkHeap = sead::ExpHeap::create(workHeapSize, "agl::WorkHeap", pHeap, 8,
                                      sead::Heap::cHeapDirection_Forward, true);
    mDebugHeap = pDebugHeap;

    for (auto& rMemory : mLockedCacheMemory)
    {
        rMemory.mpBuffer = nullptr;
        rMemory.mSize = 0;
    }

    mCursorTextureSamplerBuffer = new (pHeap, 8) u8[sizeof(TextureSampler)];

    TextureData textureData;
    textureData.initialize_(TextureType::cTextureType_2D,
                            TextureFormat::cTextureFormat_R8_G8_B8_A8_uNorm, 32, 32, 1, 1,
                            TextureAttribute(), MultiSampleType(), true);
    mCursorTextureMemory.allocBuffer_(textureData.getImageByteSize(), pHeap, 0x2000,
                                      MemoryAttribute(8));
    GPUMemAddrBase addr(mCursorTextureMemory, 0);
}

/**
 * Mounts the agl archive and creates the debug cursor texture from it.
 * @param pArchive agl resource archive
 */
void PrivateResource::createArchive(sead::ArchiveRes* pArchive)
{
    mArchive = pArchive;
    mArchiveFileDevice = new (mWorkHeap, 8) sead::ArchiveFileDevice(mArchive);

    const void* pFile = getFileFromArc("arrow.raw");
    if (!pFile)
    {
        return;
    }

    TextureData textureData;
    textureData.setDebugLabel("Cursour(agl debug)");
    textureData.initialize_(TextureType::cTextureType_2D,
                            TextureFormat::cTextureFormat_R8_G8_B8_A8_uNorm, 32, 32, 1, 1,
                            TextureAttribute(), MultiSampleType(), true);
    std::memcpy(GPUMemAddrBase(mCursorTextureMemory, 0).getPtr(), pFile,
                textureData.getImageByteSize());
    TextureDataInitializerRAW::initialize(&textureData,
                                          GPUMemVoidAddr(GPUMemAddrBase(mCursorTextureMemory, 0)),
                                          mCursorTextureMemory.getSize(),
                                          TextureFormat::cTextureFormat_R8_G8_B8_A8_uNorm, 32, 32,
                                          nullptr);

    mCursorTextureSampler = new (mCursorTextureSamplerBuffer) TextureSampler();
    mCursorTextureSampler->applyTextureData(textureData);
}

/**
 * Gets a file from the agl archive.
 * @param rPath path of the file
 * @return the file data, or nullptr if not found
 */
const void* PrivateResource::getFileFromArc(const sead::SafeString& rPath)
{
    return mArchive->getFile(rPath);
}

/**
 * Registers a locked cache memory region.
 * @param index slot index
 * @param pBuffer memory region
 * @param size size of the region
 */
void PrivateResource::setLockedCacheMemory(u32 index, void* pBuffer, u32 size)
{
    LockedCacheMemory& rMemory = mLockedCacheMemory[static_cast<s32>(index)];
    rMemory.mpBuffer = pBuffer;
    rMemory.mSize = size;
}

/**
 * Generates the host IO message (empty in release builds).
 * @param pContext host IO context
 */
void PrivateResource::genMessage(sead::hostio::Context* pContext) {}

/**
 * Handles a host IO property event (empty in release builds).
 * @param pEvent property event
 */
void PrivateResource::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent) {}

}  // namespace agl::detail
