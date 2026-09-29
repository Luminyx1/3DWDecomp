#pragma once

#include <basis/seadTypes.h>
#include <container/seadBuffer.h>
#include <nn/prepo.h>
#include <prim/seadSafeString.h>

namespace sead {
class Heap;
}

namespace erepo {

class Struct {
public:
    Struct();
    virtual ~Struct();

    static s32 calcBufferSize(s32 memberNum);
    static Struct* createWithMemberNum(s32 memberNum, sead::Heap* pHeap, bool isNothrow);
    static Struct* createWithBufferSize(s32 bufferSize, sead::Heap* pHeap, bool isNothrow);

    void addData(const sead::SafeString& rKey, bool value);
    void addData(const sead::SafeString& rKey, s32 value);
    void addData(const sead::SafeString& rKey, u32 value);
    void addData(const sead::SafeString& rKey, s64 value);
    void addData(const sead::SafeString& rKey, u64 value);
    void addData(const sead::SafeString& rKey, f32 value);
    void addData(const sead::SafeString& rKey, const char* value);
    void addData(const sead::SafeString& rKey, const sead::SafeString& rValue);

    const nn::prepo::Struct& getStruct() const { return mStruct; }
    bool isBufferReady() const { return mStruct.GetBuffer() != nullptr; }

private:
    bool constructBuffer_(s32 size, sead::Heap* pHeap);

    nn::prepo::Struct mStruct;
    sead::Buffer<u8> mBuffer;
};

}  // namespace erepo
