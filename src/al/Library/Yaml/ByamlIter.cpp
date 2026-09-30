#include "Library/Yaml/ByamlIter.hpp"

#include "Library/Yaml/ByamlData.hpp"
#include "Library/Yaml/ByamlHeader.hpp"

namespace al {
namespace {
inline ByamlDataType getContainerType(const u8* pNode) {
    return static_cast<ByamlDataType>(*pNode);
}

inline const u8* getHashKeyTable(const u8* pData) {
    const ByamlHeader* header = reinterpret_cast<const ByamlHeader*>(pData);
    return &pData[header->getHashKeyTableOffset()];
}

inline const u8* getStringTable(const u8* pData) {
    const ByamlHeader* header = reinterpret_cast<const ByamlHeader*>(pData);
    return &pData[header->getStringTableOffset()];
}
}  // namespace

/**
 * Constructs an invalid iterator.
 */
ByamlIter::ByamlIter() = default;

/**
 * Constructs an iterator over the root node of a byaml file, or an invalid one if the file is broken.
 * @param pData byaml file data
 */
ByamlIter::ByamlIter(const u8* pData) : mData(pData) {
    if (!pData) {
        return;
    }
    if (!alByamlLocalUtil::verifiByaml(pData)) {
        mData = nullptr;
        mRootNode = nullptr;
        return;
    }

    u32 offset = getHeader()->getDataOffset();
    if (!offset) {
        return;
    }
    mRootNode = &mData[offset];
}

/**
 * Copies an iterator.
 * @param rOther iterator to copy
 */
ByamlIter::ByamlIter(const ByamlIter& rOther) : mData(rOther.mData), mRootNode(rOther.mRootNode) {}

/**
 * Constructs an iterator over a node of a byaml file.
 * @param pData byaml file data
 * @param pRootNode node to iterate
 */
ByamlIter::ByamlIter(const u8* pData, const u8* pRootNode) : mData(pData), mRootNode(pRootNode) {}

/**
 * Checks whether the iterator references byaml data.
 * @return true if valid
 */
bool ByamlIter::isValid() const {
    return mData != nullptr;
}

/**
 * Checks whether the node is a hash.
 * @return true if the node is a hash
 */
bool ByamlIter::isTypeHash() const {
    return mRootNode ? getContainerType(mRootNode) == ByamlDataType::Hash : false;
}

/**
 * Checks whether the node is an array.
 * @return true if the node is an array
 */
bool ByamlIter::isTypeArray() const {
    return mRootNode ? getContainerType(mRootNode) == ByamlDataType::Array : false;
}

/**
 * Checks whether the node is a hash or an array.
 * @return true if the node is a container
 */
bool ByamlIter::isTypeContainer() const {
    return isTypeHash() || isTypeArray();
}

/**
 * Checks whether the hash node contains a key.
 * @param pKey key name
 * @return true if the key exists
 */
bool ByamlIter::isExistKey(const char* pKey) const {
    if (!mRootNode || getContainerType(mRootNode) != ByamlDataType::Hash) {
        return false;
    }

    s32 index = getKeyIndex(pKey);
    if (index < 0) {
        return false;
    }

    ByamlHashIter iter(mRootNode);
    return iter.findPair(index) != nullptr;
}

/**
 * Looks up a key in the hash key table.
 * @param pKey key name
 * @return key index, or -1 if not found
 */
s32 ByamlIter::getKeyIndex(const char* pKey) const {
    ByamlStringTableIter table(getHashKeyTable(mData));
    return table.findStringIndex(pKey);
}

/**
 * Gets the number of entries in the container node.
 * @return entry count, or 0 if the node is not a container
 */
s32 ByamlIter::getSize() const {
    if (!mRootNode) {
        return 0;
    }

    ByamlDataType type = getContainerType(mRootNode);
    if (type == ByamlDataType::Array || type == ByamlDataType::Hash) {
        return *reinterpret_cast<const u32*>(mRootNode) >> 8;
    }
    return 0;
}

/**
 * Gets an iterator for the container at an index.
 * @param index entry index
 * @return child iterator, invalid if the entry is not a container
 */
ByamlIter ByamlIter::getIterByIndex(s32 index) const {
    ByamlData data;
    if (!getByamlDataByIndex(&data, index)) {
        return ByamlIter();
    }
    if (data.getType() == ByamlDataType::Array || data.getType() == ByamlDataType::Hash) {
        return ByamlIter(mData, &mData[data.getValue()]);
    }
    if (data.getType() == ByamlDataType::Null) {
        return ByamlIter(mData, nullptr);
    }
    return ByamlIter();
}

/**
 * Gets the data of the entry at an index.
 * @param pData output data
 * @param index entry index
 * @return true on success
 */
bool ByamlIter::getByamlDataByIndex(ByamlData* pData, s32 index) const {
    if (!mRootNode) {
        return false;
    }
    if (getContainerType(mRootNode) == ByamlDataType::Array) {
        ByamlArrayIter iter(mRootNode);
        return iter.getDataByIndex(pData, index);
    }
    if (getContainerType(mRootNode) == ByamlDataType::Hash) {
        ByamlHashIter iter(mRootNode);
        return iter.getDataByIndex(pData, index);
    }
    return false;
}

/**
 * Gets an iterator for the container at a key.
 * @param pKey key name
 * @return child iterator, invalid if the entry is not a container
 */
ByamlIter ByamlIter::getIterByKey(const char* pKey) const {
    ByamlData data;
    if (!getByamlDataByKey(&data, pKey)) {
        return ByamlIter();
    }
    if (data.getType() == ByamlDataType::Array || data.getType() == ByamlDataType::Hash) {
        return ByamlIter(mData, &mData[data.getValue()]);
    }
    if (data.getType() == ByamlDataType::Null) {
        return ByamlIter(mData, nullptr);
    }
    return ByamlIter();
}

/**
 * Gets the data of the entry at a key.
 * @param pData output data
 * @param pKey key name
 * @return true on success
 */
bool ByamlIter::getByamlDataByKey(ByamlData* pData, const char* pKey) const {
    if (!mRootNode || getContainerType(mRootNode) != ByamlDataType::Hash) {
        return false;
    }

    s32 index = getKeyIndex(pKey);
    if (index < 0) {
        return false;
    }

    ByamlHashIter iter(mRootNode);
    return iter.getDataByKey(pData, index);
}

/**
 * Gets the data of the entry with a key index.
 * @param pData output data
 * @param index key index in the hash key table
 * @return true on success
 */
bool ByamlIter::getByamlDataByKeyIndex(ByamlData* pData, s32 index) const {
    if (!mRootNode || getContainerType(mRootNode) != ByamlDataType::Hash) {
        return false;
    }

    ByamlHashIter iter(mRootNode);
    return iter.getDataByKey(pData, index);
}

/**
 * Gets the data and key name of the hash entry at an index.
 * @param pData output data, may be null
 * @param pKey output key name
 * @param index entry index
 * @return true on success
 */
bool ByamlIter::getByamlDataAndKeyName(ByamlData* pData, const char** pKey, s32 index) const {
    if (!mRootNode || getContainerType(mRootNode) != ByamlDataType::Hash) {
        return false;
    }

    ByamlHashIter iter(mRootNode);
    const ByamlHashPair* pair = iter.getPairByIndex(index);
    if (!pair) {
        return false;
    }

    if (pData) {
        pData->set(pair);
    }
    ByamlStringTableIter table(getHashKeyTable(mData));
    *pKey = table.getString(pair->getKey());
    return true;
}

/**
 * Gets the key name of the hash entry at an index.
 * @param pKey output key name
 * @param index entry index
 * @return true on success
 */
bool ByamlIter::getKeyName(const char** pKey, s32 index) const {
    return getByamlDataAndKeyName(nullptr, pKey, index);
}

/**
 * Gets an iterator for the container at an index.
 * @param pIter output iterator
 * @param index entry index
 * @return true if the output iterator is valid
 */
bool ByamlIter::tryGetIterByIndex(ByamlIter* pIter, s32 index) const {
    *pIter = getIterByIndex(index);
    return pIter->isValid();
}

/**
 * Gets an iterator and the key name for the entry at an index.
 * @param pIter output iterator
 * @param pKey output key name, null if the node is not a hash
 * @param index entry index
 * @return true on success
 */
bool ByamlIter::tryGetIterAndKeyNameByIndex(ByamlIter* pIter, const char** pKey, s32 index) const {
    ByamlData data;
    if (getByamlDataAndKeyName(&data, pKey, index)) {
        if (isContainerType(data.getType())) {
            *pIter = ByamlIter(mData, &mData[data.getValue()]);
        } else if (data.getType() == ByamlDataType::Null) {
            *pIter = ByamlIter(mData, nullptr);
        }
        return true;
    }

    *pKey = nullptr;
    return tryGetIterByIndex(pIter, index);
}

/**
 * Gets an iterator for the container at a key.
 * @param pIter output iterator
 * @param pKey key name
 * @return true if the output iterator is valid
 */
bool ByamlIter::tryGetIterByKey(ByamlIter* pIter, const char* pKey) const {
    *pIter = getIterByKey(pKey);
    return pIter->isValid();
}

/**
 * Gets a string value by key.
 * @param pValue output string
 * @param pKey key name
 * @return true on success
 */
bool ByamlIter::tryGetStringByKey(const char** pValue, const char* pKey) const {
    ByamlData data;
    if (!getByamlDataByKey(&data, pKey)) {
        return false;
    }
    return tryConvertString(pValue, &data);
}

/**
 * Converts string data to a string.
 * @param pValue output string
 * @param pData data to convert
 * @return true if the data is a string
 */
bool ByamlIter::tryConvertString(const char** pValue, const ByamlData* pData) const {
    if (pData->getType() != ByamlDataType::String) {
        return false;
    }

    ByamlStringTableIter table(getStringTable(mData));
    *pValue = table.getString(pData->getValue());
    return true;
}

/**
 * Gets binary data by key.
 * @param pValue output data
 * @param pSize output size
 * @param pKey key name
 * @return true on success
 */
bool ByamlIter::tryGetBinaryByKey(const u8** pValue, s32* pSize, const char* pKey) const {
    ByamlData data;
    if (!getByamlDataByKey(&data, pKey)) {
        return false;
    }
    return tryConvertBinary(pValue, pSize, &data);
}

/**
 * Converts string data to binary data.
 * @param pValue output data
 * @param pSize output size
 * @param pData data to convert
 * @return true if the data is a string
 */
bool ByamlIter::tryConvertBinary(const u8** pValue, s32* pSize, const ByamlData* pData) const {
    if (pData->getType() != ByamlDataType::String) {
        return false;
    }

    ByamlStringTableIter table(getStringTable(mData));
    *pValue = reinterpret_cast<const u8*>(table.getString(pData->getValue()));
    *pSize = table.getStringSize(pData->getValue());
    return true;
}

/**
 * Gets an int value by key.
 * @param pValue output value
 * @param pKey key name
 * @return true on success
 */
bool ByamlIter::tryGetIntByKey(s32* pValue, const char* pKey) const {
    ByamlData data;
    if (!getByamlDataByKey(&data, pKey)) {
        return false;
    }
    return tryConvertInt(pValue, &data);
}

/**
 * Converts int data to an int.
 * @param pValue output value
 * @param pData data to convert
 * @return true if the data is an int
 */
bool ByamlIter::tryConvertInt(s32* pValue, const ByamlData* pData) const {
    if (pData->getType() != ByamlDataType::Int) {
        return false;
    }

    *pValue = pData->getValue();
    return true;
}

/**
 * Gets an unsigned int value by key.
 * @param pValue output value
 * @param pKey key name
 * @return true on success
 */
bool ByamlIter::tryGetUIntByKey(u32* pValue, const char* pKey) const {
    ByamlData data;
    if (!getByamlDataByKey(&data, pKey)) {
        return false;
    }
    if (!tryConvertUInt(pValue, &data)) {
        return false;
    }
    return true;
}

/**
 * Converts int or unsigned int data to an unsigned int.
 * @param pValue output value, clamped to 0 for negative ints
 * @param pData data to convert
 * @return true if the data is an unsigned int or a non-negative int
 */
bool ByamlIter::tryConvertUInt(u32* pValue, const ByamlData* pData) const {
    if (pData->getType() == ByamlDataType::Int) {
        s32 value = pData->getValue<s32>();
        if (value < 0) {
            *pValue = 0;
            return false;
        }
        *pValue = value;
        return true;
    }
    if (pData->getType() == ByamlDataType::UInt) {
        *pValue = pData->getValue();
        return true;
    }
    return false;
}

/**
 * Gets a float value by key.
 * @param pValue output value
 * @param pKey key name
 * @return true on success
 */
bool ByamlIter::tryGetFloatByKey(f32* pValue, const char* pKey) const {
    ByamlData data;
    if (!getByamlDataByKey(&data, pKey)) {
        return false;
    }
    return tryConvertFloat(pValue, &data);
}

/**
 * Converts float data to a float.
 * @param pValue output value
 * @param pData data to convert
 * @return true if the data is a float
 */
bool ByamlIter::tryConvertFloat(f32* pValue, const ByamlData* pData) const {
    if (pData->getType() != ByamlDataType::Float) {
        return false;
    }

    *pValue = pData->getValue<f32>();
    return true;
}

/**
 * Gets a bool value by key.
 * @param pValue output value
 * @param pKey key name
 * @return true on success
 */
bool ByamlIter::tryGetBoolByKey(bool* pValue, const char* pKey) const {
    ByamlData data;
    if (!getByamlDataByKey(&data, pKey)) {
        return false;
    }
    return tryConvertBool(pValue, &data);
}

/**
 * Converts bool data to a bool.
 * @param pValue output value
 * @param pData data to convert
 * @return true if the data is a bool
 */
bool ByamlIter::tryConvertBool(bool* pValue, const ByamlData* pData) const {
    if (pData->getType() != ByamlDataType::Bool) {
        return false;
    }

    *pValue = pData->getValue() != 0;
    return true;
}

/**
 * Gets a string value by index.
 * @param pValue output string
 * @param index entry index
 * @return true on success
 */
bool ByamlIter::tryGetStringByIndex(const char** pValue, s32 index) const {
    ByamlData data;
    if (!getByamlDataByIndex(&data, index)) {
        return false;
    }
    return tryConvertString(pValue, &data);
}

/**
 * Gets binary data by index.
 * @param pValue output data
 * @param pSize output size
 * @param index entry index
 * @return true on success
 */
bool ByamlIter::tryGetBinaryByIndex(const u8** pValue, s32* pSize, s32 index) const {
    ByamlData data;
    if (!getByamlDataByIndex(&data, index)) {
        return false;
    }
    return tryConvertBinary(pValue, pSize, &data);
}

/**
 * Gets an int value by index.
 * @param pValue output value
 * @param index entry index
 * @return true on success
 */
bool ByamlIter::tryGetIntByIndex(s32* pValue, s32 index) const {
    ByamlData data;
    if (!getByamlDataByIndex(&data, index)) {
        return false;
    }
    return tryConvertInt(pValue, &data);
}

/**
 * Gets an unsigned int value by index.
 * @param pValue output value
 * @param index entry index
 * @return true on success
 */
bool ByamlIter::tryGetUIntByIndex(u32* pValue, s32 index) const {
    ByamlData data;
    if (!getByamlDataByIndex(&data, index)) {
        return false;
    }
    if (!tryConvertUInt(pValue, &data)) {
        return false;
    }
    return true;
}

/**
 * Gets a float value by index.
 * @param pValue output value
 * @param index entry index
 * @return true on success
 */
bool ByamlIter::tryGetFloatByIndex(f32* pValue, s32 index) const {
    ByamlData data;
    if (!getByamlDataByIndex(&data, index)) {
        return false;
    }
    return tryConvertFloat(pValue, &data);
}

/**
 * Gets a bool value by index.
 * @param pValue output value
 * @param index entry index
 * @return true on success
 */
bool ByamlIter::tryGetBoolByIndex(bool* pValue, s32 index) const {
    ByamlData data;
    if (!getByamlDataByIndex(&data, index)) {
        return false;
    }
    return tryConvertBool(pValue, &data);
}

/**
 * Checks whether two valid iterators reference the same node.
 * @param rOther iterator to compare with
 * @return true if both reference the same node
 */
bool ByamlIter::isEqualData(const ByamlIter& rOther) const {
    if (!mData || !rOther.mData) {
        return false;
    }
    return mData == rOther.mData && mRootNode == rOther.mRootNode;
}
}  // namespace al
