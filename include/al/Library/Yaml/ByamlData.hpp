#pragma once

#include <basis/seadTypes.h>

namespace al {
enum class ByamlDataType : u8 {
    None = 0,
    String = 0xa0,
    Binary = 0xa1,
    Array = 0xc0,
    Hash = 0xc1,
    StringTable = 0xc2,
    Bool = 0xd0,
    Int = 0xd1,
    Float = 0xd2,
    UInt = 0xd3,
    Int64 = 0xd4,
    UInt64 = 0xd5,
    Double = 0xd6,
    Null = 0xff,
};

inline bool isContainerType(ByamlDataType type) {
    return (static_cast<u8>(type) & 0xfe) == static_cast<u8>(ByamlDataType::Array);
}

class ByamlHashPair {
public:
    s32 getKey() const { return mKeyAndType & 0xffffff; }
    ByamlDataType getType() const { return static_cast<ByamlDataType>(mKeyAndType >> 24); }
    u32 getValue() const { return mValue; }

private:
    u32 mKeyAndType;
    u32 mValue;
};

class ByamlData {
public:
    ByamlData();

    void set(const ByamlHashPair* pPair) {
        mType = static_cast<u8>(pPair->getType());
        mValue = pPair->getValue();
    }

    void setType(u8 type) { mType = type; }
    void setValue(u32 value) { mValue = value; }

    ByamlDataType getType() const { return static_cast<ByamlDataType>(mType); }
    u32 getValue() const { return mValue; }

    template <typename T>
    T getValue() const {
        return *reinterpret_cast<const T*>(&mValue);
    }

private:
    u32 mValue = 0;
    u8 mType = 0;
};

class ByamlArrayIter {
public:
    ByamlArrayIter();
    ByamlArrayIter(const u8* pData);

    u32 getSize() const;
    const u8* getTypeTable() const;
    const u32* getDataTable() const;
    bool getDataByIndex(ByamlData* pData, s32 index) const;
    const u8* getOffsetData(u32 offset) const;

private:
    const u8* mData = nullptr;
};

class ByamlHashIter {
public:
    ByamlHashIter();
    ByamlHashIter(const u8* pData);

    u32 getSize() const;
    const ByamlHashPair* getPairTable() const;
    bool getDataByIndex(ByamlData* pData, s32 index) const;
    bool getDataByKey(ByamlData* pData, s32 key) const;
    const ByamlHashPair* findPair(s32 key) const;
    const ByamlHashPair* getPairByIndex(s32 index) const;
    const u8* getOffsetData(u32 offset) const;

private:
    const u8* mData = nullptr;
};
}  // namespace al
