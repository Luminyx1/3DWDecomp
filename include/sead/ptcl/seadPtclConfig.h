#pragma once

#include <basis/seadTypes.h>
#include <heap/seadHeap.h>
#include <nn/vfx/Config.h>
#include <nn/vfx/Heap.h>

namespace sead {
namespace ptcl {
class Heap : public nn::vfx::Heap {
public:
    Heap() {}
    explicit Heap(sead::Heap* pHeap) : mHeap(pHeap) {}

    void* Alloc(size_t size, size_t alignment) override { return mHeap->tryAlloc(size, alignment); }
    void Free(void* ptr) override { mHeap->free(ptr); }

    void setHeap(sead::Heap* pHeap) { mHeap = pHeap; }
    sead::Heap* getHeap() const { return mHeap; }

private:
    sead::Heap* mHeap;
};

class Config : public nn::vfx::Config {
public:
    Config() {
        mSystemHeap = nullptr;
        mGpuHeap = nullptr;
        mResourceHeap = nullptr;
        mEditorHeap = nullptr;
        mEditorGpuHeap = nullptr;
        mResourceNum = 4;
        mEditorPriority[0] = 7;
        mEditorPriority[1] = 9;
        mEditorPriority[2] = 7;
        mEditorPriority[3] = 5;
        mEditorChannel = 0x12;
    }

    sead::Heap* getSystemHeap() const { return mSystemHeap; }
    sead::Heap* getGpuHeap() const { return mGpuHeap; }
    sead::Heap* getResourceHeap() const { return mResourceHeap; }
    sead::Heap* getEditorHeap() const { return mEditorHeap; }
    sead::Heap* getEditorGpuHeap() const { return mEditorGpuHeap; }

    void setSystemHeap(sead::Heap* pHeap) { mSystemHeap = pHeap; }
    void setGpuHeap(sead::Heap* pHeap) { mGpuHeap = pHeap; }
    void setResourceNum(s32 num) { mResourceNum = num; }

private:
    sead::Heap* mSystemHeap;
    sead::Heap* mGpuHeap;
    sead::Heap* mResourceHeap;
    sead::Heap* mEditorHeap;
    sead::Heap* mEditorGpuHeap;
    s32 mResourceNum;
    u8 mEditorPriority[4];
    u8 mEditorChannel;
};

static_assert(sizeof(Config) == 0x90);
}  // namespace ptcl
}  // namespace sead
