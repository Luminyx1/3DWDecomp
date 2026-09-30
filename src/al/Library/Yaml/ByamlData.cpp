#include "Library/Yaml/ByamlData.hpp"

#include <cstring>

#include "Library/Yaml/ByamlHeader.hpp"

namespace al {
/**
 * Constructs empty data.
 */
ByamlData::ByamlData() = default;

/**
 * Constructs an invalid string table iterator.
 */
ByamlStringTableIter::ByamlStringTableIter() = default;

/**
 * Constructs an iterator over a string table node.
 * @param pData string table node
 */
ByamlStringTableIter::ByamlStringTableIter(const u8* pData) : mData(pData) {}

/**
 * Gets the number of strings in the table.
 * @return string count
 */
s32 ByamlStringTableIter::getSize() const {
    return *reinterpret_cast<const u32*>(mData) >> 8;
}

/**
 * Gets the string offset table.
 * @return offset table
 */
const u32* ByamlStringTableIter::getAddressTable() const {
    return reinterpret_cast<const u32*>(mData + 4);
}

/**
 * Gets the offset of a string relative to the table node.
 * @param index string index
 * @return string offset
 */
u32 ByamlStringTableIter::getStringAddress(s32 index) const {
    return getAddressTable()[index];
}

/**
 * Gets the end offset of the last string.
 * @return end offset
 */
u32 ByamlStringTableIter::getEndAddress() const {
    return getAddressTable()[getSize()];
}

/**
 * Gets a string.
 * @param index string index
 * @return string
 */
const char* ByamlStringTableIter::getString(s32 index) const {
    return reinterpret_cast<const char*>(&mData[getStringAddress(index)]);
}

/**
 * Gets the length of a string.
 * @param index string index
 * @return string length without the terminator
 */
s32 ByamlStringTableIter::getStringSize(s32 index) const {
    return getStringAddress(index + 1) - getStringAddress(index) - 1;
}

/**
 * Binary searches the sorted table for a string.
 * @param pStr string to look for
 * @return string index, or -1 if not found
 */
s32 ByamlStringTableIter::findStringIndex(const char* pStr) const {
    const u32* table = getAddressTable();
    s32 lower = 0;
    s32 upper = getSize();

    while (lower < upper) {
        s32 mid = (lower + upper) / 2;
        s32 result = std::strcmp(pStr, reinterpret_cast<const char*>(&mData[table[mid]]));

        if (result == 0) {
            return mid;
        }

        if (result > 0) {
            lower = mid + 1;
        } else {
            upper = mid;
        }
    }

    return -1;
}

/**
 * Constructs an invalid array iterator.
 */
ByamlArrayIter::ByamlArrayIter() = default;

/**
 * Constructs an iterator over an array node.
 * @param pData array node
 */
ByamlArrayIter::ByamlArrayIter(const u8* pData) : mData(pData) {}

/**
 * Gets the number of entries in the array.
 * @return entry count
 */
u32 ByamlArrayIter::getSize() const {
    return *reinterpret_cast<const u32*>(mData) >> 8;
}

/**
 * Gets the entry type table.
 * @return type table
 */
const u8* ByamlArrayIter::getTypeTable() const {
    return mData + 4;
}

/**
 * Gets the entry value table, which follows the 4-byte aligned type table.
 * @return value table
 */
const u32* ByamlArrayIter::getDataTable() const {
    return reinterpret_cast<const u32*>(getTypeTable() + ((getSize() + 3) & ~3u));
}

/**
 * Gets the data of an entry.
 * @param pData output data
 * @param index entry index
 * @return true if the index is in range
 */
bool ByamlArrayIter::getDataByIndex(ByamlData* pData, s32 index) const {
    if (index < 0) {
        return false;
    }

    if (index >= static_cast<s32>(getSize())) {
        return false;
    }

    pData->setType(getTypeTable()[index]);
    pData->setValue(getDataTable()[index]);
    return true;
}

/**
 * Gets a pointer into the array node.
 * @param offset offset from the node start
 * @return pointer to the offset
 */
const u8* ByamlArrayIter::getOffsetData(u32 offset) const {
    return &mData[offset];
}
}  // namespace al

namespace alByamlLocalUtil {
/**
 * Gets the name of a byaml data type.
 * @param type data type
 * @return type name
 */
const char* getDataTypeString(s32 type) {
    switch (type) {
    case 0x00:
        return "None";
    case 0xa0:
        return "String";
    case 0xc0:
        return "Array";
    case 0xc1:
        return "Hash";
    case 0xc2:
        return "StringTable";
    case 0xd0:
        return "Bool";
    case 0xd1:
        return "Int";
    case 0xd2:
        return "Float";
    case 0xd3:
        return "UInt";
    case 0xff:
        return "NULL";
    default:
        return "Unknown";
    }
}

/**
 * Checks the header and string tables of a byaml file.
 * @param pData byaml file data
 * @return true if the file is valid
 */
bool verifiByaml(const u8* pData) {
    if (!verifiByamlHeader(pData)) {
        return false;
    }

    const u32* header = reinterpret_cast<const u32*>(pData);

    u32 hashKeyEnd = 0;
    u32 hashKeyOffset = header[1];

    if (hashKeyOffset) {
        const u8* table = &pData[hashKeyOffset];

        if (!verifiByamlStringTable(table)) {
            return false;
        }

        u32 size = *reinterpret_cast<const u32*>(table) >> 8;
        hashKeyEnd = reinterpret_cast<const u32*>(table + 4)[size];
    }

    u32 stringEnd = 0;
    u32 stringOffset = header[2];

    if (stringOffset) {
        const u8* table = &pData[stringOffset];

        if (!verifiByamlStringTable(table)) {
            return false;
        }

        u32 size = *reinterpret_cast<const u32*>(table) >> 8;
        stringEnd = reinterpret_cast<const u32*>(table + 4)[size];
    }

    u32 dataOffset = header[3];

    if ((hashKeyOffset || stringOffset) && !dataOffset) {
        return false;
    }

    if (hashKeyOffset) {
        if (stringOffset && hashKeyEnd > stringOffset) {
            return false;
        }

        if (dataOffset && hashKeyEnd > dataOffset) {
            return false;
        }
    }

    return !stringOffset || !dataOffset || stringEnd <= dataOffset;
}

/**
 * Checks the magic and version of a byaml header.
 * @param pData byaml file data
 * @return true if the header is valid
 */
bool verifiByamlHeader(const u8* pData) {
    const al::ByamlHeader* header = reinterpret_cast<const al::ByamlHeader*>(pData);
    return header->getTag() == 0x4259 && static_cast<u16>(header->getVersion() - 1) < 2;
}

/**
 * Checks that a string table is sorted and its strings are terminated.
 * @param pData string table node
 * @return true if the table is valid
 */
bool verifiByamlStringTable(const u8* pData) {
    const u32* addressTable = reinterpret_cast<const u32*>(pData + 4);

    u32 typeAndSize = *reinterpret_cast<const u32*>(pData);

    if ((typeAndSize & 0xff) != 0xc2) {
        return false;
    }

    s32 size = typeAndSize >> 8;

    if (size < 1) {
        return false;
    }

    for (s32 i = 1; i <= size; i++) {
        if (pData[addressTable[i] - 1] != '\0') {
            return false;
        }
    }

    for (s32 i = 0; i < size; i++) {
        if (addressTable[i] >= addressTable[i + 1]) {
            return false;
        }
    }

    u32 firstString = size * 4 + 8;

    if (addressTable[0] != firstString) {
        return false;
    }

    for (s32 i = 0; i < size - 1; i++) {
        const char* str = reinterpret_cast<const char*>(&pData[addressTable[i]]);
        const char* next = reinterpret_cast<const char*>(&pData[addressTable[i + 1]]);

        if (std::strcmp(str, next) > 0) {
            return false;
        }
    }

    return true;
}
}  // namespace alByamlLocalUtil

namespace al {
/**
 * Constructs an invalid hash iterator.
 */
ByamlHashIter::ByamlHashIter() = default;

/**
 * Constructs an iterator over a hash node.
 * @param pData hash node
 */
ByamlHashIter::ByamlHashIter(const u8* pData) : mData(pData) {}

/**
 * Gets the number of entries in the hash.
 * @return entry count, 0 if invalid
 */
u32 ByamlHashIter::getSize() const {
    if (!mData) {
        return 0;
    }

    return *reinterpret_cast<const u32*>(mData) >> 8;
}

/**
 * Gets the key/value pair table.
 * @return pair table, null if invalid
 */
const ByamlHashPair* ByamlHashIter::getPairTable() const {
    if (!mData) {
        return nullptr;
    }

    return reinterpret_cast<const ByamlHashPair*>(mData + 4);
}

/**
 * Gets the data of an entry.
 * @param pData output data
 * @param index entry index
 * @return true if the hash is not empty
 */
bool ByamlHashIter::getDataByIndex(ByamlData* pData, s32 index) const {
    if (!mData) {
        return false;
    }

    if (static_cast<s32>(getSize()) < 1) {
        return false;
    }

    pData->set(&getPairTable()[index]);
    return true;
}

/**
 * Gets the data of the entry with a key index.
 * @param pData output data
 * @param key key index in the hash key table
 * @return true if the key was found
 */
bool ByamlHashIter::getDataByKey(ByamlData* pData, s32 key) const {
    if (!mData) {
        return false;
    }

    if (static_cast<s32>(getSize()) < 1) {
        return false;
    }

    const ByamlHashPair* pair = findPair(key);

    if (!pair) {
        return false;
    }

    pData->set(pair);
    return true;
}

/**
 * Binary searches the sorted pair table for a key index.
 * @param key key index in the hash key table
 * @return pair, or null if not found
 */
const ByamlHashPair* ByamlHashIter::findPair(s32 key) const {
    const ByamlHashPair* table = getPairTable();

    if (!mData) {
        return nullptr;
    }

    s32 lower = 0;
    s32 upper = getSize();

    while (lower < upper) {
        s32 mid = (lower + upper) / 2;
        const ByamlHashPair* pair = &table[mid];
        s32 result = key - pair->getKey();

        if (result == 0) {
            return pair;
        }

        if (result > 0) {
            lower = mid + 1;
        } else {
            upper = mid;
        }
    }

    return nullptr;
}

/**
 * Gets the pair of an entry.
 * @param index entry index
 * @return pair, or null if out of range
 */
const ByamlHashPair* ByamlHashIter::getPairByIndex(s32 index) const {
    if (index < 0) {
        return nullptr;
    }

    if (static_cast<s32>(getSize()) <= index) {
        return nullptr;
    }

    return &getPairTable()[index];
}

/**
 * Gets a pointer into the hash node.
 * @param offset offset from the node start
 * @return pointer to the offset
 */
const u8* ByamlHashIter::getOffsetData(u32 offset) const {
    return &mData[offset];
}
}  // namespace al
