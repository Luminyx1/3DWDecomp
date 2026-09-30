#include "Library/Yaml/ByamlUtil.hpp"

#include "Library/Yaml/ByamlData.hpp"
#include "Library/Yaml/ByamlIter.hpp"

namespace al {
namespace {
struct PrintParams {
    s32 depth;
    u32 offset;
    const PrintParams* parent;
};

template <ByamlDataType Type>
inline bool isTypeByIndex(const ByamlIter& rIter, s32 index) {
    ByamlData data;
    if (rIter.getByamlDataByIndex(&data, index)) {
        return data.getType() == Type;
    }
    return false;
}

template <ByamlDataType Type>
inline bool isTypeByKey(const ByamlIter& rIter, const char* pKey) {
    ByamlData data;
    if (rIter.getByamlDataByKey(&data, pKey)) {
        return data.getType() == Type;
    }
    return false;
}

void printByamlIter_(const ByamlIter& rIter, const PrintParams* pParams);
}  // namespace

/**
 * Gets an int value by key and stores it as a u8.
 * @param pValue output value
 * @param rIter hash iterator
 * @param pKey key name
 * @return true on success
 */
bool tryGetByamlU8(u8* pValue, const ByamlIter& rIter, const char* pKey) {
    s32 value = 0;
    if (rIter.tryGetIntByKey(&value, pKey)) {
        *pValue = value;
        return true;
    }
    return false;
}

/**
 * Gets an int value by key and stores it as a u16.
 * @param pValue output value
 * @param rIter hash iterator
 * @param pKey key name
 * @return true on success
 */
bool tryGetByamlU16(u16* pValue, const ByamlIter& rIter, const char* pKey) {
    s32 value = 0;
    if (rIter.tryGetIntByKey(&value, pKey)) {
        *pValue = value;
        return true;
    }
    return false;
}

/**
 * Gets an int value by key and stores it as an s16.
 * @param pValue output value
 * @param rIter hash iterator
 * @param pKey key name
 * @return true on success
 */
bool tryGetByamlS16(s16* pValue, const ByamlIter& rIter, const char* pKey) {
    s32 value = 0;
    if (rIter.tryGetIntByKey(&value, pKey)) {
        *pValue = value;
        return true;
    }
    return false;
}

/**
 * Gets an int value by key.
 * @param pValue output value
 * @param rIter hash iterator
 * @param pKey key name
 * @return true on success
 */
bool tryGetByamlS32(s32* pValue, const ByamlIter& rIter, const char* pKey) {
    return rIter.tryGetIntByKey(pValue, pKey);
}

/**
 * Gets an int value by key and stores it as a u32.
 * @param pValue output value
 * @param rIter hash iterator
 * @param pKey key name
 * @return true on success
 */
bool tryGetByamlU32(u32* pValue, const ByamlIter& rIter, const char* pKey) {
    s32 value = 0;
    bool result = rIter.tryGetIntByKey(&value, pKey);
    if (result) {
        *pValue = value;
    }
    return result;
}

/**
 * Gets a float value by key.
 * @param pValue output value
 * @param rIter hash iterator
 * @param pKey key name
 * @return true on success
 */
bool tryGetByamlF32(f32* pValue, const ByamlIter& rIter, const char* pKey) {
    f32 value = 0;
    if (rIter.tryGetFloatByKey(&value, pKey)) {
        *pValue = value;
        return true;
    }
    return false;
}

/**
 * Reads the X and Y members of a hash; missing members are 0.
 * @param pValue output vector
 * @param rIter hash iterator
 * @return true if any member exists
 */
bool tryGetByamlV2f(sead::Vector2f* pValue, const ByamlIter& rIter) {
    f32 x = 0;
    bool result = rIter.tryGetFloatByKey(&x, "X");
    f32 y = 0;
    result |= rIter.tryGetFloatByKey(&y, "Y");
    *pValue = {x, y};
    return result;
}

/**
 * Reads the X, Y and Z members of a hash; missing members are 0.
 * @param pValue output vector
 * @param rIter hash iterator
 * @return true if any member exists
 */
bool tryGetByamlV3f(sead::Vector3f* pValue, const ByamlIter& rIter) {
    f32 x = 0;
    bool result = rIter.tryGetFloatByKey(&x, "X");
    f32 y = 0;
    result |= rIter.tryGetFloatByKey(&y, "Y");
    f32 z = 0;
    result |= rIter.tryGetFloatByKey(&z, "Z");
    *pValue = {x, y, z};
    return result;
}

/**
 * Reads the X, Y, Z and W members of a hash; W is read into Z and the result W is always 0.
 * @param pValue output vector
 * @param rIter hash iterator
 * @return true if any member exists
 */
bool tryGetByamlV4f(sead::Vector4f* pValue, const ByamlIter& rIter) {
    f32 x = 0;
    bool result = rIter.tryGetFloatByKey(&x, "X");
    f32 y = 0;
    result |= rIter.tryGetFloatByKey(&y, "Y");
    f32 z = 0;
    result |= rIter.tryGetFloatByKey(&z, "Z");
    f32 w = 0;
    result = rIter.tryGetFloatByKey(&z, "W") | result;
    *pValue = {x, y, z, w};
    return result;
}

/**
 * Reads the Min and Max members of a hash; missing members are 0.
 * @param pValue output range (x = min, y = max)
 * @param rIter hash iterator
 * @return true if any member exists
 */
bool tryGetByamlMinMax(sead::Vector2f* pValue, const ByamlIter& rIter) {
    f32 min = 0;
    bool result = rIter.tryGetFloatByKey(&min, "Min");
    f32 max = 0;
    result |= rIter.tryGetFloatByKey(&max, "Max");
    *pValue = {min, max};
    return result;
}

/**
 * Reads the X, Y and Z members of a hash; missing members are 1.
 * @param pValue output scale
 * @param rIter hash iterator
 * @return true if any member exists
 */
bool tryGetByamlScale(sead::Vector3f* pValue, const ByamlIter& rIter) {
    f32 x = 1;
    bool result = rIter.tryGetFloatByKey(&x, "X");
    f32 y = 1;
    result |= rIter.tryGetFloatByKey(&y, "Y");
    f32 z = 1;
    result |= rIter.tryGetFloatByKey(&z, "Z");
    *pValue = {x, y, z};
    return result;
}

/**
 * Reads the integer X, Y and Z members of a hash; missing members are 0.
 * @param pValue output vector
 * @param rIter hash iterator
 * @return true if any member exists
 */
bool tryGetByamlV3s32(sead::Vector3i* pValue, const ByamlIter& rIter) {
    s32 x = 0;
    bool result = rIter.tryGetIntByKey(&x, "X");
    s32 y = 0;
    result |= rIter.tryGetIntByKey(&y, "Y");
    s32 z = 0;
    result |= rIter.tryGetIntByKey(&z, "Z");
    *pValue = {x, y, z};
    return result;
}

/**
 * Reads the Min and Max vectors of a hash.
 * @param pValue output box
 * @param rIter hash iterator
 * @return true if both vectors exist
 */
bool tryGetByamlBox3f(sead::BoundBox3f* pValue, const ByamlIter& rIter) {
    sead::Vector3f min;
    sead::Vector3f max;
    if (!tryGetByamlV3f(&min, rIter, "Min")) {
        return false;
    }
    if (!tryGetByamlV3f(&max, rIter, "Max")) {
        return false;
    }
    pValue->set(min, max);
    return true;
}

/**
 * Reads a vector stored in a hash under a key.
 * @param pValue output vector
 * @param rIter parent hash iterator
 * @param pKey key name
 * @return true if the key exists and any member exists
 */
bool tryGetByamlV3f(sead::Vector3f* pValue, const ByamlIter& rIter, const char* pKey) {
    ByamlIter iter;
    if (!rIter.tryGetIterByKey(&iter, pKey)) {
        return false;
    }
    return tryGetByamlV3f(pValue, iter);
}

/**
 * Reads a vector stored in a hash under a key.
 * @param pValue output vector
 * @param rIter parent hash iterator
 * @param pKey key name
 * @return true if the key exists and any member exists
 */
bool tryGetByamlV2f(sead::Vector2f* pValue, const ByamlIter& rIter, const char* pKey) {
    ByamlIter iter;
    if (!rIter.tryGetIterByKey(&iter, pKey)) {
        return false;
    }
    return tryGetByamlV2f(pValue, iter);
}

/**
 * Reads a vector stored in a hash under a key.
 * @param pValue output vector
 * @param rIter parent hash iterator
 * @param pKey key name
 * @return true if the key exists and any member exists
 */
bool tryGetByamlV4f(sead::Vector4f* pValue, const ByamlIter& rIter, const char* pKey) {
    ByamlIter iter;
    if (!rIter.tryGetIterByKey(&iter, pKey)) {
        return false;
    }
    return tryGetByamlV4f(pValue, iter);
}

/**
 * Reads a min/max range stored in a hash under a key.
 * @param pValue output range
 * @param rIter parent hash iterator
 * @param pKey key name
 * @return true if the key exists and any member exists
 */
bool tryGetByamlMinMax(sead::Vector2f* pValue, const ByamlIter& rIter, const char* pKey) {
    ByamlIter iter;
    if (!rIter.tryGetIterByKey(&iter, pKey)) {
        return false;
    }
    return tryGetByamlMinMax(pValue, iter);
}

/**
 * Reads a scale stored in a hash under a key.
 * @param pValue output scale
 * @param rIter parent hash iterator
 * @param pKey key name
 * @return true if the key exists and any member exists
 */
bool tryGetByamlScale(sead::Vector3f* pValue, const ByamlIter& rIter, const char* pKey) {
    ByamlIter iter;
    if (!rIter.tryGetIterByKey(&iter, pKey)) {
        return false;
    }
    return tryGetByamlScale(pValue, iter);
}

/**
 * Reads an integer vector stored in a hash under a key.
 * @param pValue output vector
 * @param rIter parent hash iterator
 * @param pKey key name
 * @return true if the key exists and any member exists
 */
bool tryGetByamlV3s32(sead::Vector3i* pValue, const ByamlIter& rIter, const char* pKey) {
    ByamlIter iter;
    if (!rIter.tryGetIterByKey(&iter, pKey)) {
        return false;
    }
    return tryGetByamlV3s32(pValue, iter);
}

/**
 * Reads a box stored in a hash under a key.
 * @param pValue output box
 * @param rIter parent hash iterator
 * @param pKey key name
 * @return true if the key and both vectors exist
 */
bool tryGetByamlBox3f(sead::BoundBox3f* pValue, const ByamlIter& rIter, const char* pKey) {
    ByamlIter iter;
    if (!rIter.tryGetIterByKey(&iter, pKey)) {
        return false;
    }
    return tryGetByamlBox3f(pValue, iter);
}

/**
 * Gets a string value by key.
 * @param pValue output string
 * @param rIter hash iterator
 * @param pKey key name
 * @return true on success
 */
bool tryGetByamlString(const char** pValue, const ByamlIter& rIter, const char* pKey) {
    return rIter.tryGetStringByKey(pValue, pKey);
}

/**
 * Reads the R, G, B and A members of a hash; missing members are 0.
 * @param pValue output color
 * @param rIter hash iterator
 * @return true if any member exists
 */
bool tryGetByamlColor(sead::Color4f* pValue, const ByamlIter& rIter) {
    f32 r = 0;
    bool result = rIter.tryGetFloatByKey(&r, "R");
    f32 g = 0;
    result |= rIter.tryGetFloatByKey(&g, "G");
    f32 b = 0;
    result |= rIter.tryGetFloatByKey(&b, "B");
    f32 a = 0;
    result |= rIter.tryGetFloatByKey(&a, "A");
    *pValue = {r, g, b, a};
    return result;
}

/**
 * Reads a color stored in a hash under a key.
 * @param pValue output color
 * @param rIter parent hash iterator
 * @param pKey key name
 * @return true if the key exists and any member exists
 */
bool tryGetByamlColor(sead::Color4f* pValue, const ByamlIter& rIter, const char* pKey) {
    ByamlIter iter;
    if (!rIter.tryGetIterByKey(&iter, pKey)) {
        return false;
    }
    return tryGetByamlColor(pValue, iter);
}

/**
 * Gets a bool value by key.
 * @param pValue output value
 * @param rIter hash iterator
 * @param pKey key name
 * @return true on success
 */
bool tryGetByamlBool(bool* pValue, const ByamlIter& rIter, const char* pKey) {
    return rIter.tryGetBoolByKey(pValue, pKey);
}

/**
 * Gets a string value by key.
 * @param rIter hash iterator
 * @param pKey key name
 * @return string, or null if missing
 */
const char* getByamlKeyString(const ByamlIter& rIter, const char* pKey) {
    return tryGetByamlKeyStringOrNULL(rIter, pKey);
}

/**
 * Gets an int value by key.
 * @param rIter hash iterator
 * @param pKey key name
 * @return value, or 0 if missing
 */
s32 getByamlKeyInt(const ByamlIter& rIter, const char* pKey) {
    return tryGetByamlKeyIntOrZero(rIter, pKey);
}

/**
 * Gets a float value by key.
 * @param rIter hash iterator
 * @param pKey key name
 * @return value, or 0 if missing
 */
f32 getByamlKeyFloat(const ByamlIter& rIter, const char* pKey) {
    return tryGetByamlKeyFloatOrZero(rIter, pKey);
}

/**
 * Gets a bool value by key.
 * @param rIter hash iterator
 * @param pKey key name
 * @return value, or false if missing
 */
bool getByamlKeyBool(const ByamlIter& rIter, const char* pKey) {
    return tryGetByamlKeyBoolOrFalse(rIter, pKey);
}

/**
 * Gets a string value by key.
 * @param rIter hash iterator
 * @param pKey key name
 * @return string, or null if missing
 */
const char* tryGetByamlKeyStringOrNULL(const ByamlIter& rIter, const char* pKey) {
    const char* value = nullptr;
    if (rIter.tryGetStringByKey(&value, pKey)) {
        return value;
    }
    return nullptr;
}

/**
 * Gets an int value by key.
 * @param rIter hash iterator
 * @param pKey key name
 * @return value, or 0 if missing
 */
s32 tryGetByamlKeyIntOrZero(const ByamlIter& rIter, const char* pKey) {
    s32 value = 0;
    if (rIter.tryGetIntByKey(&value, pKey)) {
        return value;
    }
    return 0;
}

/**
 * Gets a float value by key.
 * @param rIter hash iterator
 * @param pKey key name
 * @return value, or 0 if missing
 */
f32 tryGetByamlKeyFloatOrZero(const ByamlIter& rIter, const char* pKey) {
    f32 value = 0;
    if (rIter.tryGetFloatByKey(&value, pKey)) {
        return value;
    }
    return 0;
}

/**
 * Gets a bool value by key.
 * @param rIter hash iterator
 * @param pKey key name
 * @return value, or false if missing
 */
bool tryGetByamlKeyBoolOrFalse(const ByamlIter& rIter, const char* pKey) {
    bool value = false;
    if (rIter.tryGetBoolByKey(&value, pKey)) {
        return value;
    }
    return false;
}

/**
 * Gets an iterator for the container at a key.
 * @param pIter output iterator
 * @param rIter hash iterator
 * @param pKey key name
 * @return true if the output iterator is valid
 */
bool tryGetByamlIterByKey(ByamlIter* pIter, const ByamlIter& rIter, const char* pKey) {
    return rIter.tryGetIterByKey(pIter, pKey);
}

/**
 * Gets an iterator for the container at a key.
 * @param pIter output iterator
 * @param rIter hash iterator
 * @param pKey key name
 */
void getByamlIterByKey(ByamlIter* pIter, const ByamlIter& rIter, const char* pKey) {
    rIter.tryGetIterByKey(pIter, pKey);
}

/**
 * Gets an iterator for the container at an index.
 * @param pIter output iterator
 * @param rIter container iterator
 * @param index entry index
 */
void getByamlIterByIndex(ByamlIter* pIter, const ByamlIter& rIter, s32 index) {
    rIter.tryGetIterByIndex(pIter, index);
}

/**
 * Checks whether the entry at an index is a bool.
 * @param rIter container iterator
 * @param index entry index
 * @return true if the entry is a bool
 */
bool isTypeBoolByIndex(const ByamlIter& rIter, s32 index) {
    return isTypeByIndex<ByamlDataType::Bool>(rIter, index);
}

/**
 * Checks whether the entry at a key is a bool.
 * @param rIter hash iterator
 * @param pKey key name
 * @return true if the entry is a bool
 */
bool isTypeBoolByKey(const ByamlIter& rIter, const char* pKey) {
    return isTypeByKey<ByamlDataType::Bool>(rIter, pKey);
}

/**
 * Checks whether the entry at an index is an int.
 * @param rIter container iterator
 * @param index entry index
 * @return true if the entry is an int
 */
bool isTypeIntByIndex(const ByamlIter& rIter, s32 index) {
    return isTypeByIndex<ByamlDataType::Int>(rIter, index);
}

/**
 * Checks whether the entry at a key is an int.
 * @param rIter hash iterator
 * @param pKey key name
 * @return true if the entry is an int
 */
bool isTypeIntByKey(const ByamlIter& rIter, const char* pKey) {
    return isTypeByKey<ByamlDataType::Int>(rIter, pKey);
}

/**
 * Checks whether the entry at an index is a float.
 * @param rIter container iterator
 * @param index entry index
 * @return true if the entry is a float
 */
bool isTypeFloatByIndex(const ByamlIter& rIter, s32 index) {
    return isTypeByIndex<ByamlDataType::Float>(rIter, index);
}

/**
 * Checks whether the entry at a key is a float.
 * @param rIter hash iterator
 * @param pKey key name
 * @return true if the entry is a float
 */
bool isTypeFloatByKey(const ByamlIter& rIter, const char* pKey) {
    return isTypeByKey<ByamlDataType::Float>(rIter, pKey);
}

/**
 * Checks whether the entry at an index is a string.
 * @param rIter container iterator
 * @param index entry index
 * @return true if the entry is a string
 */
bool isTypeStringByIndex(const ByamlIter& rIter, s32 index) {
    return isTypeByIndex<ByamlDataType::String>(rIter, index);
}

/**
 * Checks whether the entry at a key is a string.
 * @param rIter hash iterator
 * @param pKey key name
 * @return true if the entry is a string
 */
bool isTypeStringByKey(const ByamlIter& rIter, const char* pKey) {
    return isTypeByKey<ByamlDataType::String>(rIter, pKey);
}

/**
 * Checks whether the entry at an index is an array.
 * @param rIter container iterator
 * @param index entry index
 * @return true if the entry is an array
 */
bool isTypeArrayByIndex(const ByamlIter& rIter, s32 index) {
    return isTypeByIndex<ByamlDataType::Array>(rIter, index);
}

/**
 * Checks whether the entry at a key is an array.
 * @param rIter hash iterator
 * @param pKey key name
 * @return true if the entry is an array
 */
bool isTypeArrayByKey(const ByamlIter& rIter, const char* pKey) {
    return isTypeByKey<ByamlDataType::Array>(rIter, pKey);
}

/**
 * Checks whether the entry at an index is a hash.
 * @param rIter container iterator
 * @param index entry index
 * @return true if the entry is a hash
 */
bool isTypeHashByIndex(const ByamlIter& rIter, s32 index) {
    return isTypeByIndex<ByamlDataType::Hash>(rIter, index);
}

/**
 * Checks whether the entry at a key is a hash.
 * @param rIter hash iterator
 * @param pKey key name
 * @return true if the entry is a hash
 */
bool isTypeHashByKey(const ByamlIter& rIter, const char* pKey) {
    return isTypeByKey<ByamlDataType::Hash>(rIter, pKey);
}

/**
 * Gets the key name and int value of the hash entry at an index.
 * @param pKey output key name
 * @param pValue output value
 * @param rIter hash iterator
 * @param index entry index
 * @return true if the entry exists and is an int
 */
bool tryGetByamlKeyAndIntByIndex(const char** pKey, s32* pValue, const ByamlIter& rIter, s32 index) {
    ByamlData data;
    if (!rIter.getByamlDataAndKeyName(&data, pKey, index)) {
        return false;
    }
    if (!rIter.tryConvertInt(pValue, &data)) {
        return false;
    }
    return true;
}

/**
 * Gets the number of entries in a container.
 * @param rIter container iterator
 * @return entry count
 */
s32 getByamlIterDataNum(const ByamlIter& rIter) {
    return rIter.getSize();
}

/**
 * Walks a byaml file for debug printing.
 * @param pData byaml file data
 */
void printByamlIter(const u8* pData) {
    ByamlIter iter(pData);
    printByamlIter(iter);
}

/**
 * Walks a byaml tree for debug printing.
 * @param rIter root iterator
 */
void printByamlIter(const ByamlIter& rIter) {
    PrintParams params = {0, rIter.getHeader()->getDataOffset(), nullptr};
    printByamlIter_(rIter, &params);
}

namespace {
void printByamlIter_(const ByamlIter& rIter, const PrintParams* pParams) {
    s32 size = rIter.getSize();
    for (s32 i = 0; i < size; i++) {
        ByamlData data;
        if (rIter.isTypeArray()) {
            rIter.getByamlDataByIndex(&data, i);
        } else if (rIter.isTypeHash()) {
            const char* key = nullptr;
            rIter.getByamlDataAndKeyName(&data, &key, i);
        }

        if (isContainerType(data.getType())) {
            const PrintParams* params = pParams;
            do {
                if (params->offset == data.getValue()) {
                    goto next;
                }
                params = params->parent;
            } while (params);

            {
                ByamlIter child;
                if (rIter.tryGetIterByIndex(&child, i)) {
                    PrintParams childParams = {pParams->depth + 1, data.getValue(), pParams};
                    printByamlIter_(child, &childParams);
                }
            }
        } else {
            ByamlDataType type = data.getType();
            if (type == ByamlDataType::Float) {
                f32 value;
                rIter.tryConvertFloat(&value, &data);
            } else if (type == ByamlDataType::Int) {
                s32 value;
                rIter.tryConvertInt(&value, &data);
            } else if (type == ByamlDataType::String) {
                const char* value;
                rIter.tryConvertString(&value, &data);
            } else if (type == ByamlDataType::Bool) {
                bool value;
                rIter.tryConvertBool(&value, &data);
            }
        }
    next:;
    }
}
}  // namespace
}  // namespace al
