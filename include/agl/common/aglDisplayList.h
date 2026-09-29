#pragma once

#include <basis/seadTypes.h>
#include <nvn/nvn.h>
#include <prim/seadBitFlag.h>

#include "common/aglGPUMemAddr.h"

namespace sead {
class Heap;
namespace hostio {
class Context;
}
}  // namespace sead

namespace agl {

class DrawContext;

struct ExecuteDisplayList {
    enum Flag {
        cFlag_Valid = 1 << 0,
        cFlag_Recorded = 1 << 1,
    };

    sead::BitFlag8 mFlags;
    NVNcommandHandle mHandle;
};
static_assert(sizeof(ExecuteDisplayList) == 0x10);

class DisplayList {
    friend class DrawContext;

public:
    static constexpr u32 cDefaultControlMemorySize = 0x200;

    enum Flag {
        cFlag_Valid = 1 << 0,
        cFlag_Recorded = 1 << 1,
        cFlag_Recording = 1 << 3,
        cFlag_UserControlMemory = 1 << 4,
    };

    DisplayList();
    virtual ~DisplayList();

    void setControlMemory(void* pMemory, u64 size);
    void clear();
    void setBuffer(GPUMemAddr<u8> buffer, u64 size);
    void setValidSize_(u64 size);
    void copyTo(DisplayList* pOther) const;
    void copyTo(ExecuteDisplayList* pOther) const;
    bool beginDisplayList();
    u32 endDisplayList();
    bool beginDisplayListBuffer(GPUMemAddr<u8> buffer, u64 size, bool invalidateCPUCache);
    void endDisplayListBuffer(sead::Heap* pHeap);
    void adjustValidSize();
    void invalidateCPUCache() const;
    void callDirect(DrawContext* pDrawContext) const;
    void dump() const;
    static u64 suspend(void** ppMemory);
    static void resume(void* pMemory, u64 size);
    static u64 calcRemainingSize();
    u32 calcUsedSize() const;
    void genMessage(sead::hostio::Context* pContext);

    GPUMemAddr<u8> getBuffer() const { return mBuffer; }
    u32 getValidSize() const { return mValidSize; }
    bool isValid() const { return mFlags.isOn(cFlag_Valid); }
    bool isRecording() const { return mFlags.isOn(cFlag_Recording); }
    u32 getControlMemoryUsed() const { return mControlMemoryUsed; }
    const char* getName() const { return mName; }
    void setName(const char* pName) { mName = pName; }
    void invalidate() { mFlags.reset(cFlag_Valid | cFlag_Recorded); }
    bool isUserControlMemory() const { return mFlags.isOn(cFlag_UserControlMemory); }
    void* getControlMemory() const { return mControlMemory; }
    NVNcommandHandle getHandle() const { return mHandle; }
    const NVNcommandHandle* getHandlePtr() const { return &mHandle; }

private:
    static void outOfMemoryCallback_(NVNcommandBuffer* pCommandBuffer,
                                     NVNcommandBufferMemoryEvent event, u64 minSize,
                                     void* pUserData);

    void* getDefaultControlMemory_()
    {
        return reinterpret_cast<void*>((reinterpret_cast<uintptr_t>(mDefaultControlMemory) + 7) &
                                       ~uintptr_t(7));
    }

    GPUMemAddr<u8> mBuffer;
    u32 mBufferSize;
    sead::BitFlag8 mFlags;
    u32 mValidSize;
    alignas(8) u8 mDefaultControlMemory[cDefaultControlMemorySize + 8];
    u32 mControlMemorySize;
    u32 mControlMemoryUsed;
    void* mControlMemory;
    NVNcommandHandle mHandle;
    NVNcommandBuffer* mNvnCommandBuffer;
    const char* mName;
};
static_assert(sizeof(DisplayList) == 0x260);

}  // namespace agl
