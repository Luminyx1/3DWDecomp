#include "Library/System/SystemKit.hpp"

#include <heap/seadHeapMgr.h>
#include <resource/seadParallelSZSDecompressor.h>
#include <resource/seadResourceMgr.h>
#include <resource/seadSZSDecompressor.h>
#include <resource/seadSharcArchiveRes.h>

#include <framework/seadFramework.h>
#include "Project/File/FileLoader.hpp"
#include "Project/Memory/MemorySystem.hpp"
#include "Project/Resource/ResourceSystem.hpp"
#include "Project/SaveData/SaveDataDirector.hpp"

namespace al {
/**
 * Constructs an empty system kit.
 */
SystemKit::SystemKit() = default;

/**
 * Creates the memory system with the given heap as current.
 * @param pHeap Parent heap.
 * @param a First heap size.
 * @param b Second heap size.
 * @param c Third heap size.
 */
void SystemKit::createMemorySystem(sead::Heap* pHeap, u64 a, u64 b, u64 c) {
    sead::ScopedCurrentHeapSetter setter(pHeap);
    mMemorySystem = new MemorySystem(pHeap, a, b, c);
}

/**
 * Creates the file loader.
 * @param threadPriority Loader thread priority.
 * @param unk Unknown flag.
 */
void SystemKit::createFileLoader(s32 threadPriority, bool unk) {
    mFileLoader = new FileLoader(threadPriority, unk);
}

/**
 * Registers the archive factory and SZS decompressor and creates the resource system.
 * @param pArchivePath Path of the archive list.
 * @param threadPriority Decompressor thread priority, or -1 for a synchronous decompressor.
 * @param decompressDestinationSize Decompression buffer size, or a negative value for the default.
 * @param useSubCore Whether to decompress on a sub core.
 */
void SystemKit::createResourceSystem(const char* pArchivePath, s32 threadPriority,
                                     s32 decompressDestinationSize, bool useSubCore) {
    sead::ResourceMgr::instance()->registerFactory(
        new sead::DirectResourceFactory<sead::SharcArchiveRes>(), "sarc");

    decompressDestinationSize =
        decompressDestinationSize >= 0 ? decompressDestinationSize : 0x400000;
    u8* decompressDestination = new (0x20) u8[decompressDestinationSize];

    sead::ResourceMgr* instance = sead::ResourceMgr::instance();

    if (threadPriority == -1) {
        instance->registerDecompressor(
            new sead::SZSDecompressor(decompressDestinationSize / 2, decompressDestination), "szs");
    } else {
        sead::CoreId core = useSubCore ? sead::CoreId::cSub2 : sead::CoreId::cMain;
        instance->registerDecompressor(new sead::ParallelSZSDecompressor(
                                           decompressDestinationSize / 2, threadPriority, nullptr,
                                           decompressDestination, sead::CoreIdMask(core)),
                                       "szs");
    }

    mResourceSystem = new ResourceSystem(pArchivePath);
}

/**
 * Creates the save data director.
 * @param workBufferSize Save data work buffer size.
 * @param threadPriority Save data thread priority.
 */
void SystemKit::createSaveDataSystem(u32 workBufferSize, s32 threadPriority) {
    mSaveDataDirector = new SaveDataDirector(workBufferSize, threadPriority);
}

/**
 * Gets the frame buffer of the top screen.
 * @return The frame buffer.
 */
sead::FrameBuffer* SystemKit::getFrameBufferTop() {
    return mFramework->getMethodFrameBuffer(3);
}

/**
 * Gets the frame buffer of the bottom screen, which doesn't exist.
 * @return Always nullptr.
 */
sead::FrameBuffer* SystemKit::getFrameBufferBtm() {
    return nullptr;
}
}  // namespace al
