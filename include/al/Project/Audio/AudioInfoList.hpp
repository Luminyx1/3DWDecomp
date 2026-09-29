#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>
#include <cstring>

#include "Library/Yaml/ByamlIter.hpp"

namespace al {
/// A sorted list of named audio infos, optionally chained to further lists.
template <typename T>
class AudioInfoList {
public:
    static s32 compareInfoAndKey(const T* pInfo, const char* pKey) { return strcmp(pInfo->mName, pKey); }

    AudioInfoList() : mNext(nullptr) {}

    s32 getInfoNum() const {
        s32 num = mInfos->size();

        if (mNext != nullptr) {
            return mNext->getInfoNum() + num;
        }

        return num;
    }

    const T* getInfoAt(s32 index) const {
        const AudioInfoList<T>* list = this;
        do {
            s32 size = list->mInfos->size();
            if (index < size) {
                return list->mInfos->unsafeAt(index);
            }
            list = list->mNext;
            if (list == nullptr) {
                return nullptr;
            }
            index -= size;
        } while (index >= 0);
        return nullptr;
    }

    s32 findInfoIndex(const char* pKey) const {
        return mInfos->binarySearch(reinterpret_cast<const T*>(pKey),
                                    reinterpret_cast<s32 (*)(const T*, const T*)>(compareInfoAndKey));
    }

    const T* tryFindInfo(const char* pKey) const {
        const AudioInfoList<T>* list = this;
        do {
            s32 index = list->findInfoIndex(pKey);
            if (index >= 0) {
                const T* info = list->getInfoAt(index);
                if (info != nullptr) {
                    return info;
                }
            }
            list = list->mNext;
        } while (list != nullptr);
        return nullptr;
    }

    void sortInfo() {
        if (getInfoNum() >= 10) {
            mInfos->template heapSort_<T>(T::compareInfo);
        } else {
            mInfos->template sort_<T>(T::compareInfo);
        }

        if (mNext != nullptr) {
            mNext->sortInfo();
        }
    }

    sead::PtrArray<T>* mInfos;  // _0
    AudioInfoList<T>* mNext;    // _8
};

/**
 * @brief Creates a sorted info list from a yaml array, creating one info per element.
 * @param rIter The yaml iterator of the array.
 * @return The new info list.
 */
template <typename T>
AudioInfoList<T>* createInfoList(const ByamlIter& rIter) {
    s32 size = rIter.getSize();
    AudioInfoList<T>* list = new AudioInfoList<T>();
    list->mInfos = new sead::PtrArray<T>();
    list->mInfos->allocBuffer(size + 1, nullptr);
    for (s32 i = 0; i < size; i++) {
        ByamlIter iter;
        rIter.tryGetIterByIndex(&iter, i);
        T* info = T::createInfo(iter);
        list->mInfos->pushBack(info);
    }
    list->sortInfo();
    return list;
}
}  // namespace al
