#ifndef SEAD_HEAP_POLICIES_H_
#define SEAD_HEAP_POLICIES_H_

#include <basis/seadTypes.h>
#include <heap/seadHeapMgr.h>

namespace sead
{
class Heap;

class HeapArray
{
public:
    Heap* getHeap(s32 index) const
    {
        if (u32(index) < u32(HeapMgr::getRootHeapNum()))
            return mHeaps[index];
        return mHeaps[0];
    }

    Heap* getPrimaryHeap() const { return mHeaps[mPrimaryIndex]; }

    Heap* mHeaps[4];
    bool mAdjusted[4]{};
    s32 mPrimaryIndex{};
};

struct HeapPolicy
{
    Heap* parent{};
    u32 size{};
    u32 create_slack{};
    u32 adjust_slack{};
    u8 adjust{};
    u8 temporary{};
    u8 dont_create{};
};

struct HeapPolicies
{
    const HeapPolicy& getPolicy(s32 index) const
    {
        if (u32(index) < u32(HeapMgr::getRootHeapNum()))
            return mPolicies[index];
        return mPolicies[0];
    }

    HeapPolicy mPolicies[4];
    s32 mPrimaryIndex{};
};

}  // namespace sead

#endif  // SEAD_HEAP_POLICIES_H_
