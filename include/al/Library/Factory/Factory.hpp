#pragma once

#include <basis/seadTypes.h>

#include "Project/Base/StringUtil.hpp"

namespace al {
    class MemorySceneHeapCustomAlloc;

    template <typename T>
    struct NameToCreator {
        const char* name;
        T func;
    };
    
    template<typename T>
    class Factory {
    public:
        inline Factory(const char *pName) : mName(pName), mFuncs(nullptr), mNumEntries(0) {
            
        }

        template <s32 N>
        inline void initFactory(const NameToCreator<T> (&rEntries)[N]) {
            mFuncs = rEntries;
            mNumEntries = N;
        }

        virtual const char* convertName(const char* pName) const { return pName; }

        s32 getNumFactoryEntries() const { return mNumEntries; }

        s32 getEntryIndex(T* pCreator, const char* pEntryName) const {
            const char* name = convertName(pEntryName);
            s32 num = mNumEntries;
            const NameToCreator<T>* entries = mFuncs;
            for (s32 i = 0; i < num; i++) {
                if (isEqualString(name, entries[i].name)) {
                    *pCreator = entries[i].func;
                    return i;
                }
            }

            return -1;
        }

        const char* mName;                          // 0x08
        const NameToCreator<T>* mFuncs;             // 0x10
        s32 mNumEntries;                            // 0x18
    };
};