#pragma once

#include <basis/seadTypes.h>

#include "Library/Yaml/ByamlHeader.hpp"

namespace al {
class ByamlData;

class ByamlIter {
public:
    ByamlIter();
    ByamlIter(const u8* pData);
    ByamlIter(const u8* pData, const u8* pRootNode);
    ByamlIter(const ByamlIter& rOther);

    ByamlIter& operator=(const ByamlIter& rOther) = default;

    bool isValid() const;
    bool isTypeHash() const;
    bool isTypeArray() const;
    bool isTypeContainer() const;
    bool isExistKey(const char* pKey) const;
    s32 getKeyIndex(const char* pKey) const;
    s32 getSize() const;
    ByamlIter getIterByIndex(s32 index) const;
    bool getByamlDataByIndex(ByamlData* pData, s32 index) const;
    ByamlIter getIterByKey(const char* pKey) const;
    bool getByamlDataByKey(ByamlData* pData, const char* pKey) const;
    bool getByamlDataByKeyIndex(ByamlData* pData, s32 index) const;
    bool getByamlDataAndKeyName(ByamlData* pData, const char** pKey, s32 index) const;
    bool getKeyName(const char** pKey, s32 index) const;
    bool tryGetIterByIndex(ByamlIter* pIter, s32 index) const;
    bool tryGetIterAndKeyNameByIndex(ByamlIter* pIter, const char** pKey, s32 index) const;
    bool tryGetIterByKey(ByamlIter* pIter, const char* pKey) const;
    bool tryGetStringByKey(const char** pValue, const char* pKey) const;
    bool tryConvertString(const char** pValue, const ByamlData* pData) const;
    bool tryGetBinaryByKey(const u8** pValue, s32* pSize, const char* pKey) const;
    bool tryConvertBinary(const u8** pValue, s32* pSize, const ByamlData* pData) const;
    bool tryGetIntByKey(s32* pValue, const char* pKey) const;
    bool tryConvertInt(s32* pValue, const ByamlData* pData) const;
    bool tryGetUIntByKey(u32* pValue, const char* pKey) const;
    bool tryConvertUInt(u32* pValue, const ByamlData* pData) const;
    bool tryGetFloatByKey(f32* pValue, const char* pKey) const;
    bool tryConvertFloat(f32* pValue, const ByamlData* pData) const;
    bool tryGetBoolByKey(bool* pValue, const char* pKey) const;
    bool tryConvertBool(bool* pValue, const ByamlData* pData) const;
    bool tryGetStringByIndex(const char** pValue, s32 index) const;
    bool tryGetBinaryByIndex(const u8** pValue, s32* pSize, s32 index) const;
    bool tryGetIntByIndex(s32* pValue, s32 index) const;
    bool tryGetUIntByIndex(u32* pValue, s32 index) const;
    bool tryGetFloatByIndex(f32* pValue, s32 index) const;
    bool tryGetBoolByIndex(bool* pValue, s32 index) const;
    bool isEqualData(const ByamlIter& rOther) const;

    const ByamlHeader* getHeader() const { return reinterpret_cast<const ByamlHeader*>(mData); }
    const u8* getData() const { return mData; }
    const u8* getRootNode() const { return mRootNode; }

private:
    const u8* mData = nullptr;
    const u8* mRootNode = nullptr;
};
}  // namespace al
