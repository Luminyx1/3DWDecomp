#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>
#include <cstring>

#include "Library/Yaml/ByamlIter.hpp"

namespace al {
class ByamlIter;

template <typename T, typename Compare>
void heapSortInfoArray(sead::PtrArray<T>* pArray, Compare&& cmp) {
    const s32 num = pArray->size();
    T** infos = pArray->data();

    for (s32 i = num / 2; i > 0; i--) {
        T* value = infos[i - 1];
        s32 parent = i;
        s32 child = parent * 2;

        while (child <= num) {
            if (child < num && cmp(infos[child - 1], infos[child]) < 0) {
                child++;
            }

            if (cmp(value, infos[child - 1]) >= 0) {
                break;
            }

            infos[parent - 1] = infos[child - 1];
            parent = child;
            child = parent * 2;
        }

        infos[parent - 1] = value;
    }

    for (s32 i = num; i >= 2; i--) {
        const s32 last = i - 1;
        T* value = infos[last];
        infos[last] = infos[0];
        s32 parent = 1;
        s32 child = 2;

        while (child <= last) {
            if (child < last && cmp(infos[child - 1], infos[child]) < 0) {
                child++;
            }

            if (cmp(value, infos[child - 1]) >= 0) {
                break;
            }

            infos[parent - 1] = infos[child - 1];
            parent = child;
            child = parent * 2;
        }

        infos[parent - 1] = value;
    }
}

template <typename T, typename Compare>
void shakerSortInfoArray(sead::PtrArray<T>* pArray, Compare&& cmp) {
    T** infos = pArray->data();

    if (pArray->size() < 2) {
        return;
    }

    s32 lo = 0;
    s32 hi = pArray->size() - 1;

    while (lo < hi) {
        s32 last = lo;

        for (s32 i = lo; i < hi; i++) {
            if (cmp(infos[i], infos[i + 1]) > 0) {
                T* tmp = infos[i + 1];
                infos[i + 1] = infos[i];
                infos[i] = tmp;
                last = i;
            }
        }

        hi = last;

        if (hi <= lo) {
            break;
        }

        last = hi;

        for (s32 i = hi; i > lo; i--) {
            if (cmp(infos[i], infos[i - 1]) < 0) {
                T* tmp = infos[i - 1];
                infos[i - 1] = infos[i];
                infos[i] = tmp;
                last = i;
            }
        }

        lo = last;

        if (lo == hi) {
            break;
        }
    }
}

template <typename T>
class AudioInfoList {
public:
    s32 getInfoNum() const {
        s32 num = mInfos->size();

        if (mNext != nullptr)
            return mNext->getInfoNum() + num;

        return num;
    }

    T* getInfo(s32 index) const {
        const AudioInfoList<T>* list = this;

        while (true) {
            s32 num = list->mInfos->size();

            if (index < num) {
                return list->mInfos->unsafeAt(index);
            }

            list = list->mNext;

            if (list == nullptr) {
                return nullptr;
            }

            index -= num;

            if (index < 0) {
                return nullptr;
            }
        }
    }

    T* getInfoDirect(s32 index) const {
        const AudioInfoList<T>* list = this;

        while (true) {
            const sead::PtrArray<T>* infos = list->mInfos;

            if (infos->size() > index) {
                return infos->unsafeAt(index);
            }

            index -= infos->size();
            list = list->mNext;
        }
    }

    T* findInfoDirect(s32 index) const {
        const AudioInfoList<T>* list = this;

        while (true) {
            const sead::PtrArray<T>* infos = list->mInfos;

            if (index < infos->size()) {
                return infos->unsafeAt(index);
            }

            list = list->mNext;

            if (list == nullptr) {
                return nullptr;
            }

            index -= infos->size();

            if (index < 0) {
                return nullptr;
            }
        }
    }

    T* tryGetInfoDirect(s32 index) const {
        if (index < 0) {
            return nullptr;
        }

        return findInfoDirect(index);
    }

    T* tryGetInfo(s32 index) const {
        if (index < 0) {
            return nullptr;
        }

        return getInfo(index);
    }

    s32 searchInfoIndex(const char* pKey) const {
        s32 num = mInfos->size();

        if (num == 0) {
            return -1;
        }

        s32 lo = 0;
        s32 hi = num - 1;
        T** infos = mInfos->data();

        while (lo < hi) {
            s32 mid = (lo + hi) / 2;
            s32 result = strcmp(infos[mid]->mName, pKey);

            if (result == 0) {
                return mid;
            }

            if (result < 0) {
                lo = mid + 1;
            } else {
                hi = mid;
            }
        }

        if (strcmp(infos[lo]->mName, pKey) == 0) {
            return lo;
        }

        return -1;
    }

    void sortInfo() {
        AudioInfoList<T>* list = this;

        do {
            if (list->getInfoNum() >= 10) {
                heapSortInfoArray<T>(list->mInfos, T::compareInfo);
            } else {
                shakerSortInfoArray<T>(list->mInfos, T::compareInfo);
            }

            list = list->mNext;
        } while (list != nullptr);
    }

    T* tryFindInfo(const char* pKey) const {
        const AudioInfoList<T>* list = this;
        T* info;

        do {
            info = list->tryGetInfoDirect(list->searchInfoIndex(pKey));
            list = list->mNext;
        } while (info == nullptr && list != nullptr);

        return info;
    }

    sead::PtrArray<T>* mInfos;
    AudioInfoList<T>* mNext;
};

template <typename T>
AudioInfoList<T>* createInfoList(const ByamlIter& rIter) {
    s32 size = rIter.getSize();
    AudioInfoList<T>* list = new AudioInfoList<T>;
    list->mNext = nullptr;
    list->mInfos = new sead::PtrArray<T>;
    list->mInfos->allocBuffer(size + 1, nullptr);

    for (s32 i = 0; i < size; i++) {
        ByamlIter iter;
        rIter.tryGetIterByIndex(&iter, i);
        T* info = T::createInfo(iter);

        if (info != nullptr) {
            list->mInfos->pushBack(info);
        }
    }

    list->sortInfo();
    return list;
}
}  // namespace al
