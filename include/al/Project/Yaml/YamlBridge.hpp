#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

#include "Library/Yaml/ByamlIter.hpp"
#include "Library/Yaml/ByamlUtil.hpp"
#include "Project/Yaml/YamlFormatter.hpp"

class YamlWriterBridge {
public:
    YamlWriterBridge(const al::ByamlIter* pIter) : mIter(pIter) {}

    template <typename T>
    void exec(T* pData, const char* pKey) {
        al::ByamlIter iter;
        mIter->tryGetIterByKey(&iter, pKey);
        YamlWriterBridge bridge(&iter);
        pData->serialize(bridge);
    }

    void exec(bool* pData, const char* pKey) { mIter->tryGetBoolByKey(pData, pKey); }

    void exec(s32* pData, const char* pKey) { mIter->tryGetIntByKey(pData, pKey); }

    void exec(f32* pData, const char* pKey) { mIter->tryGetFloatByKey(pData, pKey); }

    void exec(const char** pData, const char* pKey) { mIter->tryGetStringByKey(pData, pKey); }

    void exec(sead::Vector3f* pData, const char* pKey) { al::tryGetByamlV3f(pData, *mIter, pKey); }

private:
    const al::ByamlIter* mIter;
};

class YamlReaderBridge {
public:
    YamlReaderBridge(al::YamlFormatter* pFormatter) : mFormatter(pFormatter) {}

    template <typename T>
    void exec(T* pData, const char* pKey) {
        if (!pData) {
            return;
        }
        mFormatter->startHash(pKey);
        YamlReaderBridge bridge(mFormatter);
        pData->serialize(bridge);
        mFormatter->endHash();
    }

    void exec(bool* pData, const char* pKey) { mFormatter->writeHashBool(pKey, *pData); }

    void exec(s32* pData, const char* pKey) { mFormatter->writeHashInt(pKey, *pData); }

    void exec(f32* pData, const char* pKey) { mFormatter->writeHashFloat(pKey, *pData); }

    void exec(const char** pData, const char* pKey) {
        if (*pData) {
            mFormatter->writeHashString(pKey, *pData);
        }
    }

    void exec(sead::Vector3f* pData, const char* pKey) { mFormatter->writeHashV3f(pKey, *pData); }

private:
    al::YamlFormatter* mFormatter;
};
