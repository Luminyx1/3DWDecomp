#pragma once

#include <basis/seadTypes.h>

namespace al {
class ByamlHeader {
public:
    u16 getTag() const { return mTag; }
    u16 getVersion() const { return mVersion; }
    u32 getHashKeyTableOffset() const { return mHashKeyTableOffset; }
    u32 getStringTableOffset() const { return mStringTableOffset; }
    u32 getDataOffset() const { return mDataOffset; }

private:
    u16 mTag;
    u16 mVersion;
    u32 mHashKeyTableOffset;
    u32 mStringTableOffset;
    u32 mDataOffset;
};

class ByamlStringTableIter {
public:
    ByamlStringTableIter();
    ByamlStringTableIter(const u8* pData);

    s32 getSize() const;
    const u32* getAddressTable() const;
    u32 getStringAddress(s32 index) const;
    u32 getEndAddress() const;
    const char* getString(s32 index) const;
    s32 getStringSize(s32 index) const;
    s32 findStringIndex(const char* pStr) const;

private:
    const u8* mData = nullptr;
};
}  // namespace al

namespace alByamlLocalUtil {
const char* getDataTypeString(s32 type);
bool verifiByaml(const u8* pData);
bool verifiByamlHeader(const u8* pData);
bool verifiByamlStringTable(const u8* pData);
}  // namespace alByamlLocalUtil
