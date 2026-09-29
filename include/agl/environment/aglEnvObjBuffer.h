#pragma once

#include <basis/seadTypes.h>
#include <container/seadBuffer.h>
#include <container/seadPtrArray.h>
#include <container/seadSafeArray.h>
#include <prim/seadSafeString.h>

#include "environment/aglEnvObj.h"

namespace sead {
class Heap;
}

namespace agl::env {

class EnvObjBuffer {
public:
    class AllocateArg {
    public:
        AllocateArg();
        virtual ~AllocateArg() = default;

        int getCount(int type) const { return mCounts[type]; }
        int getTotal() const { return mTotal; }

        void setContainMax(int type, int count);
        void setContainMax(const TypeInfo* type, int count) { setContainMax(type->id, count); }

    protected:
        sead::SafeArray<int, EnvObj::cTypeMax> mCounts;
        int mTotal = 0;
    };

    struct TypeRange {
        u16 mStart;
        u16 mNum;
    };
    static_assert(sizeof(TypeRange) == 4);

    class ObjIterator {
    public:
        explicit ObjIterator(sead::Buffer<EnvObj*>::constIterator it) : mIt(it) {}

        bool operator==(const ObjIterator& rOther) const
        {
            return mIt.getIndex() == rOther.mIt.getIndex();
        }
        bool operator!=(const ObjIterator& rOther) const { return !(*this == rOther); }
        ObjIterator& operator++()
        {
            ++mIt;
            return *this;
        }
        EnvObj* operator*() const { return *mIt; }
        s32 getIndex() const { return mIt.getIndex(); }

    private:
        sead::Buffer<EnvObj*>::constIterator mIt;
    };

    EnvObjBuffer();
    virtual ~EnvObjBuffer();

    virtual void allocBuffer(const AllocateArg& rArg, sead::Heap* pHeap);
    virtual void freeBuffer();

    s32 searchTypeIndex(s32 type, const sead::SafeString& rName) const;
    s32 searchBufferIndex(s32 type, const sead::SafeString& rName) const;
    s32 searchTypeIndex(const EnvObj* pObj) const;
    s32 searchType(s32 bufferIndex) const;
    void setEnable(s32 type, bool enable);
    void sort(s32 type);
    void setEnableAll(bool enable);

    ObjIterator begin(s32 type) const { return ObjIterator(mObj.begin(mTypeRange[type].mStart)); }
    ObjIterator end(s32 type) const
    {
        const TypeRange& rRange = mTypeRange[type];
        return ObjIterator(mObj.begin(rRange.mStart + rRange.mNum));
    }

    s32 getObjNum(s32 type) const { return mTypeRange[type].mNum; }
    EnvObj* getObj(s32 type, s32 index) const
    {
        return *mObj.unsafeGet(mTypeRange[type].mStart + index);
    }
    EnvObj* tryGetObj(s32 type, s32 index) const
    {
        const TypeRange& rRange = mTypeRange[type];
        return index < rRange.mNum ? *mObj.unsafeGet(rRange.mStart + index) : nullptr;
    }
    s32 getBufferSize() const { return mObj.size(); }
    EnvObj* getBufferObj(s32 index) const { return mObj[index]; }

protected:
    friend class EnvObj;

    sead::PtrArray<EnvObj> mActiveObj;
    sead::Buffer<TypeRange> mTypeRange;
    sead::Buffer<EnvObj*> mObj;
};
static_assert(sizeof(EnvObjBuffer) == 0x38);

}  // namespace agl::env
