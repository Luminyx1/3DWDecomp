#pragma once

#include "heap/seadHeap.h"

namespace sead
{
class SeparateHeap : public Heap
{
public:
    static SeparateHeap* create(const SafeString& name, void* managementArea,
                                size_t managementAreaSize, void* heapStart, size_t heapSize,
                                bool enableLock);
    static size_t getManagementAreaSize(size_t nodeNum);

    s32 getNodeNumMax() const { return mNodeNumMax; }
    s32 getUsedNodeNum() const { return mUsedNodeNum; }

private:
    u8 _dc[0xf0 - 0xdc];
    s32 mNodeNumMax;
    u8 _f4[0x108 - 0xf4];
    s32 mUsedNodeNum;
};

}  // namespace sead
