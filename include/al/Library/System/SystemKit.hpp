#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>
#include <prim/seadSafeString.h>

namespace sead {
class Heap;
class Viewport;
class FrameBuffer;
class Framework;
}  // namespace sead

namespace al {
class FileLoader;
class ResourceSystem;
class SaveDataDirector;
class MemorySystem;

class SystemKit {
public:
    SystemKit();

    void createMemorySystem(sead::Heap* pHeap, u64 a, u64 b, u64 c);
    void createFileLoader(s32 threadPriority, bool unk);
    void createResourceSystem(const char* pArchivePath, s32 threadPriority,
                              s32 decompressDestinationSize, bool useSubCore);
    void createSaveDataSystem(u32 workBufferSize, s32 threadPriority);
    sead::FrameBuffer* getFrameBufferTop();
    sead::FrameBuffer* getFrameBufferBtm();

    MemorySystem* getMemorySystem() { return mMemorySystem; }
    FileLoader* getFileLoader() { return mFileLoader; }
    ResourceSystem* getResourceSystem() { return mResourceSystem; }
    SaveDataDirector* getSaveDataDirector() { return mSaveDataDirector; }

    MemorySystem* mMemorySystem = nullptr;
    FileLoader* mFileLoader = nullptr;
    ResourceSystem* mResourceSystem = nullptr;
    SaveDataDirector* mSaveDataDirector = nullptr;
    sead::Framework* mFramework = nullptr;
    sead::SafeString _28;
    sead::SafeString _38;
};

static_assert(sizeof(SystemKit) == 0x48);
}  // namespace al

namespace alSystemKitFunction {
void applyViewportTop(const sead::Viewport& rViewport);
void applyViewportBtm(const sead::Viewport& rViewport);
}  // namespace alSystemKitFunction

namespace alProjectInterface {
al::SystemKit* getSystemKit();
const sead::Vector3f& getPlayerPos();
}  // namespace alProjectInterface
