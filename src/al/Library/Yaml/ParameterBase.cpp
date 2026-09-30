#include "Library/Yaml/ParameterBase.hpp"

#include <codec/seadHashCRC32.h>

#include "Library/Yaml/ByamlIter.hpp"
#include "Library/Yaml/ByamlUtil.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
/**
 * Constructs a parameter and adds it to a parameter object.
 * @param rName name of the parameter
 * @param rLabel label of the parameter
 * @param rMeta meta data of the parameter
 * @param pObj parameter object to add the parameter to, or nullptr
 * @param isPushBack whether to push the parameter back
 */
ParameterBase::ParameterBase(const sead::SafeString& rName, const sead::SafeString& rLabel,
                             const sead::SafeString& rMeta, ParameterObj* pObj, bool isPushBack) {
    initializeListNode(rName, rLabel, rMeta, pObj, false);
}

/**
 * Initializes the name of the parameter and adds it to a parameter object.
 * @param rName name of the parameter
 * @param rLabel label of the parameter
 * @param rMeta meta data of the parameter
 * @param pObj parameter object to add the parameter to, or nullptr
 * @param isPushBack whether to push the parameter back
 */
void ParameterBase::initializeListNode(const sead::SafeString& rName,
                                       const sead::SafeString& rLabel,
                                       const sead::SafeString& rMeta, ParameterObj* pObj,
                                       bool isPushBack) {
    mNext = nullptr;
    mName = rName;
    mHash = calcHash(rName);
    if (pObj) {
        pObj->pushBackListNode(this);
    }
}

/**
 * Calculates the hash of a parameter name.
 * @param rKey name to hash
 * @return the hash
 */
u32 ParameterBase::calcHash(const sead::SafeString& rKey) {
    return sead::HashCRC32::calcHash(rKey.cstr(), rKey.calcLength());
}

/**
 * Appends a parameter to the parameter list.
 * @param pParam parameter to append
 */
void ParameterObj::pushBackListNode(ParameterBase* pParam) {
    if (mTailParam) {
        mTailParam->setNext(pParam);
        mTailParam = pParam;
    } else {
        mTailParam = pParam;
        mRootParam = pParam;
    }
}

/**
 * Reads the value of the parameter from a byaml.
 * @param rIter byaml to read from
 */
void ParameterBase::tryGetParam(const ByamlIter& rIter) {
    switch (getParamType()) {
    case YamlParamType::Bool: {
        bool value;
        if (tryGetByamlBool(&value, rIter, mName.cstr())) {
            setPtrValue(value);
        }
        break;
    }
    case YamlParamType::F32: {
        f32 value;
        if (tryGetByamlF32(&value, rIter, mName.cstr())) {
            setPtrValue(value);
        }
        break;
    }
    case YamlParamType::S32: {
        s32 value;
        if (tryGetByamlS32(&value, rIter, mName.cstr())) {
            setPtrValue(value);
        }
        break;
    }
    case YamlParamType::U32: {
        u32 value;
        if (tryGetByamlU32(&value, rIter, mName.cstr())) {
            setPtrValue(value);
        }
        break;
    }
    case YamlParamType::V2f: {
        sead::Vector2f value;
        if (tryGetByamlV2f(&value, rIter, mName.cstr())) {
            *getMutableValuePtr<sead::Vector2f>() = value;
        }
        break;
    }
    case YamlParamType::V3f: {
        sead::Vector3f value;
        if (tryGetByamlV3f(&value, rIter, mName.cstr())) {
            *getMutableValuePtr<sead::Vector3f>() = value;
        }
        break;
    }
    case YamlParamType::V4f:
    case YamlParamType::Q4f: {
        sead::Vector4f value;
        if (tryGetByamlV4f(&value, rIter, mName.cstr())) {
            *getMutableValuePtr<sead::Vector4f>() = value;
        }
        break;
    }
    case YamlParamType::C4f: {
        sead::Color4f value;
        if (tryGetByamlColor(&value, rIter, mName.cstr())) {
            *getMutableValuePtr<sead::Color4f>() = value;
        }
        break;
    }
    case YamlParamType::StringRef: {
        const char* value = tryGetByamlKeyStringOrNULL(rIter, mName.cstr());
        if (value) {
            setPtrValue(value);
        }
        break;
    }
    case YamlParamType::String32:
    case YamlParamType::String64:
    case YamlParamType::String256: {
        const char* value = tryGetByamlKeyStringOrNULL(rIter, mName.cstr());
        if (value) {
            getMutableValuePtr<sead::BufferedSafeString>()->format("%s", value);
        }
        break;
    }
    default:
        break;
    }
    afterGetParam();
}

template <>
bool ParameterBase::isEqual_<const char*>(const ParameterBase& rParam) const {
    return isEqualString(getValuePtr<const char>(), rParam.getValuePtr<const char>());
}

template <typename T>
bool ParameterBase::isEqual_(const ParameterBase& rParam) const {
    return *getValuePtr<T>() == *rParam.getValuePtr<T>();
}

template <>
void ParameterBase::copyLerp_<f32>(const ParameterBase& rParamA, const ParameterBase& rParamB,
                                   f32 rate) {
    f32 valueA = *rParamA.getValuePtr<f32>();
    f32 valueB = *rParamB.getValuePtr<f32>();
    setPtrValue(valueA + (valueB - valueA) * rate);
}

template <>
void ParameterBase::copyLerp_<sead::Quatf>(const ParameterBase& rParamA,
                                           const ParameterBase& rParamB, f32 rate) {
    sead::QuatCalcCommon<f32>::slerpTo(*static_cast<sead::Quatf*>(ptr()),
                                       *rParamA.getValuePtr<sead::Quatf>(),
                                       *rParamB.getValuePtr<sead::Quatf>(), rate);
}

/**
 * Checks whether another parameter has the same type, name and value.
 * @param rParam parameter to compare with
 * @return true if the parameters are equal
 */
bool ParameterBase::isEqual(const ParameterBase& rParam) {
    if ((s32)getParamType() != (s32)rParam.getParamType()) {
        return false;
    }
    if (mHash != rParam.getHash()) {
        return false;
    }

    switch (getParamType()) {
    case YamlParamType::Bool:
        return isEqual_<bool>(rParam);
    case YamlParamType::F32:
        return isEqual_<f32>(rParam);
    case YamlParamType::S32:
        return isEqual_<s32>(rParam);
    case YamlParamType::U32:
        return isEqual_<u32>(rParam);
    case YamlParamType::V2f:
        return isEqual_<sead::Vector2f>(rParam);
    case YamlParamType::V3f:
        return isEqual_<sead::Vector3f>(rParam);
    case YamlParamType::V4f:
    case YamlParamType::Q4f:
        return isEqual_<sead::Vector4f>(rParam);
    case YamlParamType::C4f:
        return isEqual_<sead::Color4f>(rParam);
    case YamlParamType::String32:
    case YamlParamType::String64:
    case YamlParamType::String256:
        return isEqualString(getValuePtr<sead::SafeString>()->cstr(),
                             rParam.getValuePtr<sead::SafeString>()->cstr());
    case YamlParamType::StringRef:
        return isEqual_<const char*>(rParam);
    default:
        return false;
    }
}

/**
 * Copies the value of another parameter with the same type and name.
 * @param rParam parameter to copy from
 * @return true if the value was copied
 */
bool ParameterBase::copy(const ParameterBase& rParam) {
    if ((s32)getParamType() != (s32)rParam.getParamType()) {
        return false;
    }
    if (mHash != rParam.getHash()) {
        return false;
    }

    switch (rParam.getParamType()) {
    case YamlParamType::StringRef:
        *getMutableValuePtr<sead::SafeString>() = *rParam.getValuePtr<sead::SafeString>();
        return true;
    default: {
        u8* dest = getMutableValuePtr<u8>();
        const u8* source = rParam.getValuePtr<u8>();
        s32 n = size();
        for (s32 i = 0; i < n; i++) {
            *dest = *source;
            dest++;
            source++;
        }
        return true;
    }
    }
}

/**
 * Interpolates between the values of two parameters with the same type and name.
 * @param rParamA parameter at rate 0
 * @param rParamB parameter at rate 1
 * @param rate interpolation rate
 * @return always false
 */
bool ParameterBase::copyLerp(const ParameterBase& rParamA, const ParameterBase& rParamB,
                             f32 rate) {
    if ((s32)getParamType() != (s32)rParamA.getParamType() || mHash != rParamA.getHash() ||
        (s32)getParamType() != (s32)rParamB.getParamType() || mHash != rParamB.getHash()) {
        return false;
    }

    switch (getParamType()) {
    case YamlParamType::Bool:
    case YamlParamType::S32:
    case YamlParamType::U32:
    case YamlParamType::String32:
    case YamlParamType::String64:
    case YamlParamType::String256:
    case YamlParamType::StringRef:
        if (rate >= 0.5f && !(rate == 0.5f)) {
            copy(rParamB);
        } else {
            copy(rParamA);
        }
        return false;
    case YamlParamType::F32:
        copyLerp_<f32>(rParamA, rParamB, rate);
        return false;
    case YamlParamType::V2f: {
        sead::Vector2f* value = getMutableValuePtr<sead::Vector2f>();
        const sead::Vector2f* valueA = rParamA.getValuePtr<sead::Vector2f>();
        const sead::Vector2f* valueB = rParamB.getValuePtr<sead::Vector2f>();
        value->x = valueA->x + (valueB->x - valueA->x) * rate;
        value->y = valueA->y + (valueB->y - valueA->y) * rate;
        return false;
    }
    case YamlParamType::V3f: {
        sead::Vector3f* value = getMutableValuePtr<sead::Vector3f>();
        const sead::Vector3f* valueA = rParamA.getValuePtr<sead::Vector3f>();
        const sead::Vector3f* valueB = rParamB.getValuePtr<sead::Vector3f>();
        value->x = valueA->x + (valueB->x - valueA->x) * rate;
        value->y = valueA->y + (valueB->y - valueA->y) * rate;
        value->z = valueA->z + (valueB->z - valueA->z) * rate;
        return false;
    }
    case YamlParamType::V4f: {
        sead::Vector4f* value = getMutableValuePtr<sead::Vector4f>();
        const sead::Vector4f* valueA = rParamA.getValuePtr<sead::Vector4f>();
        const sead::Vector4f* valueB = rParamB.getValuePtr<sead::Vector4f>();
        value->x = valueA->x + (valueB->x - valueA->x) * rate;
        value->y = valueA->y + (valueB->y - valueA->y) * rate;
        value->z = valueA->z + (valueB->z - valueA->z) * rate;
        value->w = valueA->w + (valueB->w - valueA->w) * rate;
        return false;
    }
    case YamlParamType::Q4f:
        copyLerp_<sead::Quatf>(rParamA, rParamB, rate);
        return false;
    case YamlParamType::C4f:
        getMutableValuePtr<sead::Color4f>()->setLerp(*rParamA.getValuePtr<sead::Color4f>(),
                                                     *rParamB.getValuePtr<sead::Color4f>(), rate);
        return false;
    default:
        return false;
    }
}

/**
 * Constructs an empty parameter object.
 */
ParameterObj::ParameterObj() = default;

/**
 * Reads the parameters and arrays of the object from a byaml.
 * @param rIter byaml to read from
 */
void ParameterObj::tryGetParam(const ByamlIter& rIter) {
    const ByamlIter* iter = &rIter;
    ByamlIter keyIter;
    if (!mKey.isEmpty()) {
        rIter.tryGetIterByKey(&keyIter, mKey.cstr());
        iter = &keyIter;
        if (!keyIter.isValid()) {
            return;
        }
    }
    for (ParameterBase* param = mRootParam; param; param = param->getNext()) {
        param->tryGetParam(*iter);
    }
    for (ParameterArray* array = mParamArray; array; array = array->getNext()) {
        ByamlIter arrayIter;
        if (array->getKey().isEmpty()) {
            continue;
        }
        rIter.tryGetIterByKey(&arrayIter, array->getKey().cstr());
        if (!arrayIter.isValid()) {
            continue;
        }
        array->tryGetParam(arrayIter);
    }
}

/**
 * Reads the objects of the array from a byaml.
 * @param rIter byaml to read from
 */
void ParameterArray::tryGetParam(const ByamlIter& rIter) {
    ByamlIter arrayIter;
    rIter.tryGetIterByKey(&arrayIter, mKey.cstr());
    if (!arrayIter.isValid() || !arrayIter.isTypeArray()) {
        return;
    }
    mSize = arrayIter.getSize();
    s32 index = 0;
    for (ParameterObj* obj = mRootObjNode; obj; obj = obj->getNext()) {
        ByamlIter objIter;
        arrayIter.tryGetIterByIndex(&objIter, index);
        if (!objIter.isValid()) {
            continue;
        }
        obj->tryGetParam(objIter);
        index++;
    }
}

/**
 * Adds an array to the object.
 * @param pArray array to add
 * @param rKey key of the array
 */
void ParameterObj::addArray(ParameterArray* pArray, const sead::SafeString& rKey) {
    pArray->setKey(rKey);
    if (!mParamArray) {
        mParamArray = pArray;
        return;
    }
    ParameterArray* array = mParamArray;
    while (array->getNext()) {
        array = array->getNext();
    }
    array->setNext(pArray);
}

/**
 * Checks whether another object has equal parameters and arrays.
 * @param rObj object to compare with
 * @return true if the objects are equal
 */
bool ParameterObj::isEqual(const ParameterObj& rObj) const {
    ParameterBase* param = mRootParam;
    ParameterBase* otherParam = rObj.getRootParam();
    if (!param) {
        if (otherParam) {
            return false;
        }
    } else {
        while (param && otherParam) {
            if (!param->isEqual(*otherParam)) {
                return false;
            }
            param = param->getNext();
            otherParam = otherParam->getNext();
        }
    }

    ParameterArray* array = mParamArray;
    ParameterArray* otherArray = rObj.getParamArray();
    if (!array) {
        if (otherArray) {
            return false;
        }
    } else {
        if (!otherArray) {
            return false;
        }
        while (array && otherArray) {
            if (!array->isEqual(*otherArray)) {
                return false;
            }
            array = array->getNext();
            otherArray = otherArray->getNext();
        }
    }
    return true;
}

/**
 * Checks whether another array has equal objects.
 * @param rArray array to compare with
 * @return true if the arrays are equal
 */
bool ParameterArray::isEqual(const ParameterArray& rArray) const {
    if (mSize != rArray.getSize()) {
        return false;
    }
    ParameterObj* obj = mRootObjNode;
    ParameterObj* otherObj = rArray.getRootObjNode();
    if (!obj || !otherObj) {
        return !obj && !otherObj;
    }
    while (obj && otherObj) {
        if (!obj->isEqual(*otherObj)) {
            return false;
        }
        obj = obj->getNext();
        otherObj = otherObj->getNext();
    }
    return true;
}

/**
 * Copies the parameters and arrays of another object.
 * @param rObj object to copy from
 */
void ParameterObj::copy(const ParameterObj& rObj) {
    ParameterBase* param = mRootParam;
    ParameterBase* otherParam = rObj.getRootParam();
    if (param) {
        while (param && otherParam) {
            param->copy(*otherParam);
            param = param->getNext();
            otherParam = otherParam->getNext();
        }
    }

    ParameterArray* otherArray = rObj.getParamArray();
    ParameterArray* array = mParamArray;
    if (array) {
        while (array && otherArray) {
            array->copy(*otherArray);
            array = array->getNext();
            otherArray = otherArray->getNext();
        }
    }
}

/**
 * Copies the objects of another array.
 * @param rArray array to copy from
 */
void ParameterArray::copy(const ParameterArray& rArray) {
    ParameterObj* otherObj = rArray.getRootObjNode();
    ParameterObj* obj = mRootObjNode;
    while (obj && otherObj) {
        obj->copy(*otherObj);
        obj = obj->getNext();
        otherObj = otherObj->getNext();
    }
}

/**
 * Interpolates the parameters and arrays between two objects.
 * @param rObjA object at rate 0
 * @param rObjB object at rate 1
 * @param rate interpolation rate
 */
void ParameterObj::copyLerp(const ParameterObj& rObjA, const ParameterObj& rObjB, f32 rate) {
    ParameterBase* param = mRootParam;
    ParameterBase* paramA = rObjA.getRootParam();
    ParameterBase* paramB = rObjB.getRootParam();
    if (param) {
        if (rate <= 0.0f) {
            while (param && paramA) {
                param->copy(*paramA);
                param = param->getNext();
                paramA = paramA->getNext();
            }
        } else if (rate >= 1.0f) {
            while (param && paramB) {
                param->copy(*paramB);
                param = param->getNext();
                paramB = paramB->getNext();
            }
        } else {
            while (param && paramA && paramB) {
                param->copyLerp(*paramA, *paramB, rate);
                param = param->getNext();
                paramA = paramA->getNext();
                paramB = paramB->getNext();
            }
        }
    }

    ParameterArray* arrayB = rObjB.getParamArray();
    ParameterArray* arrayA = rObjA.getParamArray();
    ParameterArray* array = mParamArray;
    while (array && arrayA && arrayB) {
        array->copyLerp(*arrayA, *arrayB, rate);
        array = array->getNext();
        arrayA = arrayA->getNext();
        arrayB = arrayB->getNext();
    }
}

/**
 * Interpolates the objects between two arrays.
 * @param rArrayA array at rate 0
 * @param rArrayB array at rate 1
 * @param rate interpolation rate
 */
void ParameterArray::copyLerp(const ParameterArray& rArrayA, const ParameterArray& rArrayB,
                              f32 rate) {
    ParameterObj* objB = rArrayB.getRootObjNode();
    ParameterObj* objA = rArrayA.getRootObjNode();
    ParameterObj* obj = mRootObjNode;
    while (obj && objA && objB) {
        obj->copyLerp(*objA, *objB, rate);
        obj = obj->getNext();
        objA = objA->getNext();
        objB = objB->getNext();
    }
}

/**
 * Constructs an empty parameter array.
 */
ParameterArray::ParameterArray() = default;

/**
 * Adds an object to the array.
 * @param pObj object to add
 */
void ParameterArray::addObj(ParameterObj* pObj) {
    pObj->setKey(sead::SafeString::cEmptyString);
    if (!mRootObjNode) {
        mRootObjNode = pObj;
        return;
    }
    ParameterObj* obj = mRootObjNode;
    while (obj->getNext()) {
        obj = obj->getNext();
    }
    obj->setNext(pObj);
}

/**
 * Removes all objects from the array.
 */
void ParameterArray::clearObj() {
    ParameterObj* obj = mRootObjNode;
    while (obj) {
        ParameterObj* next = obj->getNext();
        obj->setNext(nullptr);
        obj = next;
    }
    mRootObjNode = nullptr;
}

/**
 * Removes an object from the array.
 * @param pObj object to remove
 */
void ParameterArray::removeObj(ParameterObj* pObj) {
    ParameterObj* prev = nullptr;
    for (ParameterObj* obj = mRootObjNode; obj; obj = obj->getNext()) {
        if (obj == pObj) {
            if (prev) {
                prev->setNext(pObj->getNext());
            } else {
                mRootObjNode = pObj->getNext();
            }
            obj->setNext(nullptr);
            return;
        }
        prev = obj;
    }
}

/**
 * Checks whether an object is in the array.
 * @param pObj object to look for
 * @return true if the object is in the array
 */
bool ParameterArray::isExistObj(ParameterObj* pObj) {
    for (ParameterObj* obj = mRootObjNode; obj; obj = obj->getNext()) {
        if (obj == pObj) {
            return true;
        }
    }
    return false;
}

/**
 * Constructs an empty parameter list.
 */
ParameterList::ParameterList() = default;

/**
 * Adds a child list.
 * @param pList list to add
 * @param rKey key of the list
 */
void ParameterList::addList(ParameterList* pList, const sead::SafeString& rKey) {
    pList->setKey(rKey);
    if (!mRootListNode) {
        mRootListNode = pList;
        return;
    }
    ParameterList* list = mRootListNode;
    while (list->getNext()) {
        list = list->getNext();
    }
    list->setNext(pList);
}

/**
 * Adds an object.
 * @param pObj object to add
 * @param rKey key of the object
 */
void ParameterList::addObj(ParameterObj* pObj, const sead::SafeString& rKey) {
    pObj->setKey(rKey);
    if (!mRootObjNode) {
        mRootObjNode = pObj;
        return;
    }
    ParameterObj* obj = mRootObjNode;
    while (obj->getNext()) {
        obj = obj->getNext();
    }
    obj->setNext(pObj);
}

/**
 * Adds an array.
 * @param pArray array to add
 * @param rKey key of the array
 */
void ParameterList::addArray(ParameterArray* pArray, const sead::SafeString& rKey) {
    pArray->setKey(rKey);
    if (!mRootArrayNode) {
        mRootArrayNode = pArray;
        return;
    }
    ParameterArray* array = mRootArrayNode;
    while (array->getNext()) {
        array = array->getNext();
    }
    array->setNext(pArray);
}

/**
 * Removes all child lists.
 */
void ParameterList::clearList() {
    ParameterList* list = mRootListNode;
    while (list) {
        ParameterList* next = list->getNext();
        list->setNext(nullptr);
        list = next;
    }
    mRootListNode = nullptr;
}

/**
 * Removes all objects.
 */
void ParameterList::clearObj() {
    ParameterObj* obj = mRootObjNode;
    while (obj) {
        ParameterObj* next = obj->getNext();
        obj->setNext(nullptr);
        obj = next;
    }
    mRootObjNode = nullptr;
}

/**
 * Removes a child list.
 * @param pList list to remove
 */
void ParameterList::removeList(ParameterList* pList) {
    ParameterList* prev = nullptr;
    for (ParameterList* list = mRootListNode; list; list = list->getNext()) {
        if (list == pList) {
            if (prev) {
                prev->setNext(pList->getNext());
            } else {
                mRootListNode = pList->getNext();
            }
            list->setNext(nullptr);
            return;
        }
        prev = list;
    }
}

/**
 * Removes an object.
 * @param pObj object to remove
 */
void ParameterList::removeObj(ParameterObj* pObj) {
    ParameterObj* prev = nullptr;
    for (ParameterObj* obj = mRootObjNode; obj; obj = obj->getNext()) {
        if (obj == pObj) {
            if (prev) {
                prev->setNext(pObj->getNext());
            } else {
                mRootObjNode = pObj->getNext();
            }
            obj->setNext(nullptr);
            return;
        }
        prev = obj;
    }
}

/**
 * Checks whether an object is in the list.
 * @param pObj object to look for
 * @return true if the object is in the list
 */
bool ParameterList::isExistObj(ParameterObj* pObj) {
    for (ParameterObj* obj = mRootObjNode; obj; obj = obj->getNext()) {
        if (obj == pObj) {
            return true;
        }
    }
    return false;
}

/**
 * Reads all objects, arrays and child lists from a byaml.
 * @param rIter byaml to read from
 */
void ParameterList::tryGetParam(const ByamlIter& rIter) {
    for (ParameterObj* obj = mRootObjNode; obj; obj = obj->getNext()) {
        obj->tryGetParam(rIter);
    }
    for (ParameterArray* array = mRootArrayNode; array; array = array->getNext()) {
        array->tryGetParam(rIter);
    }
    for (ParameterList* list = mRootListNode; list; list = list->getNext()) {
        list->tryGetParam(rIter);
    }
}
}  // namespace al
