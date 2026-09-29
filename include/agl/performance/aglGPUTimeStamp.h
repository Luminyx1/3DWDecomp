#pragma once

#include <container/seadBuffer.h>
#include <nvn/nvn.h>
#include <thread/seadAtomic.h>

#include "common/aglGPUMemAddr.h"
#include "common/aglGPUMemBlock.h"

namespace sead {
class Heap;
}

namespace agl {
class DrawContext;
}

namespace agl::perf {

class GPUTimeStamp {
public:
    GPUTimeStamp() {}
    explicit GPUTimeStamp(const GPUMemAddr<NVNcounterData>& rMemAddr) : mMemAddr(rMemAddr) {}
    ~GPUTimeStamp() {}

    void startTop(DrawContext* pDrawContext);
    void startBottom(DrawContext* pDrawContext);
    u64 get() const;

    void setAddress(NVNbufferAddress address) { mAddress = address; }

private:
    NVNbufferAddress mAddress;
    GPUMemAddr<NVNcounterData> mMemAddr;
};
static_assert(sizeof(GPUTimeStamp) == 0x20);

class GPUTimeStampArray {
public:
    enum Type {
        cType_Top = 0,
        cType_Begin = 1,
        cType_End = 2,
        cType_Num = 3,
    };

    GPUTimeStampArray();
    virtual ~GPUTimeStampArray();

    void allocBuffer(s32 num, sead::Heap* pHeap);
    GPUTimeStamp* getNext(Type type);

    GPUTimeStamp& getStamp(Type type, s32 index = 0) { return mEntries[type].mBuffer[index]; }

private:
    struct Entry {
        sead::Buffer<GPUTimeStamp> mBuffer;
        sead::Atomic<u32> mIndex;
    };

    Entry mEntries[cType_Num];
    GPUMemBlock<NVNcounterData> mMemBlock;
    NVNbuffer mNvnBuffer;
};

}  // namespace agl::perf
