#pragma once

#include <math/seadMatrix.h>

namespace al {
    /// Holds named pointers to matrices.
    class MtxPtrHolder {
    public:
        MtxPtrHolder();

        void init(s32 num);
        void setMtxPtrAndName(s32 index, const char* pName, const sead::Matrix34f* pMtx);
        void setMtxPtr(const char* pName, const sead::Matrix34f* pMtx);
        s32 findIndex(const char* pName) const;
        const sead::Matrix34f* findMtxPtr(const char* pName) const;
        const sead::Matrix34f* tryFindMtxPtr(const char* pName) const;
        s32 tryFindIndex(const char* pName) const;

        const char** mNames;                // _0
        const sead::Matrix34f** mMtxPtrs;   // _8
        s32 mNum;                           // _10
    };
};
