#pragma once

#include <heap/seadExpHeap.h>

#include <basis/seadTypes.h>

namespace al {
class EffectHeap : public sead::ExpHeap {
public:
    static EffectHeap* create(u32 size, const char* pName);

    EffectHeap(const char* pName, void* pAddress, u32 size);

    void* tryAlloc(size_t size, s32 alignment) override;
};
}  // namespace al
