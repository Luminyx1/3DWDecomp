#pragma once

#include <basis/seadTypes.h>
#include <gfx/seadColor.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>
#include <prim/seadEnum.h>
#include <prim/seadSafeString.h>

namespace al {
class ByamlIter;
class ParameterArray;
class ParameterList;
class ParameterObj;

SEAD_ENUM(YamlParamType, Invalid, Bool, F32, S32, U32, V2f, V3f, V4f, Q4f, C4f, String32, String64,
          String256, StringRef)

class ParameterBase {
public:
    ParameterBase() { initializeListNode("default", "parameter", "", nullptr, true); }

    ParameterBase(const sead::SafeString& rName, const sead::SafeString& rLabel,
                  const sead::SafeString& rMeta, ParameterObj* pObj, bool isPushBack);

    virtual YamlParamType getParamType() const = 0;
    virtual const void* ptr() const = 0;
    virtual void* ptr() = 0;

    virtual void afterGetParam() {}

    virtual s32 size() const = 0;
    virtual bool isEqual(const ParameterBase& rParam);
    virtual bool copy(const ParameterBase& rParam);
    virtual bool copyLerp(const ParameterBase& rParamA, const ParameterBase& rParamB, f32 rate);

    void initializeListNode(const sead::SafeString& rName, const sead::SafeString& rLabel,
                            const sead::SafeString& rMeta, ParameterObj* pObj, bool isPushBack);
    void tryGetParam(const ByamlIter& rIter);

    static u32 calcHash(const sead::SafeString& rKey);

    ParameterBase* getNext() const { return mNext; }

    void setNext(ParameterBase* pParam) { mNext = pParam; }

    u32 getHash() const { return mHash; }

    template <typename T>
    T* getMutableValuePtr() {
        return static_cast<T*>(ptr());
    }

    template <typename T>
    const T* getValuePtr() const {
        return static_cast<const T*>(ptr());
    }

    template <typename T>
    void setPtrValue(T value) {
        *getMutableValuePtr<T>() = value;
    }

private:
    template <typename T>
    bool isEqual_(const ParameterBase& rParam) const;

    template <typename T>
    void copyLerp_(const ParameterBase& rParamA, const ParameterBase& rParamB, f32 rate);

protected:
    ParameterBase* mNext;
    sead::FixedSafeString<0x40> mName;
    u32 mHash;
};

template <typename T>
class Parameter : public ParameterBase {
public:
    Parameter(const T& value, const sead::SafeString& rName, const sead::SafeString& rLabel,
              const sead::SafeString& rMeta, ParameterObj* pObj, bool isPushBack)
        : mValue() {
        initializeListNode(rName, rLabel, rMeta, pObj, isPushBack);
        mValue = value;
    }

    YamlParamType getParamType() const override { return YamlParamType::Invalid; }

    const T& getValue() const { return mValue; }

    void setValue(const T& value) { mValue = value; }

    const void* ptr() const override { return &mValue; }

    void* ptr() override { return &mValue; }

    s32 size() const override { return sizeof(T); }

private:
    T mValue;
};

#define AL_PARAMETER_TYPE_DEF(NAME, TYPE)                                                          \
    class Parameter##NAME : public Parameter<TYPE> {                                               \
    public:                                                                                        \
        Parameter##NAME(const TYPE& value, const sead::SafeString& rName,                          \
                        const sead::SafeString& rLabel, const sead::SafeString& rMeta,             \
                        ParameterObj* pObj, bool isPushBack)                                       \
            : Parameter(value, rName, rLabel, rMeta, pObj, isPushBack) {}                          \
                                                                                                   \
        Parameter##NAME(const TYPE& value, ParameterObj* pObj, const sead::SafeString& rName,      \
                        const sead::SafeString& rLabel, const sead::SafeString& rMeta,             \
                        bool isPushBack)                                                           \
            : Parameter(value, rName, rLabel, rMeta, pObj, isPushBack) {}                          \
                                                                                                   \
        YamlParamType getParamType() const override { return YamlParamType::NAME; }                \
    };

AL_PARAMETER_TYPE_DEF(Bool, bool)
AL_PARAMETER_TYPE_DEF(F32, f32)
AL_PARAMETER_TYPE_DEF(S32, s32)
AL_PARAMETER_TYPE_DEF(U32, u32)
AL_PARAMETER_TYPE_DEF(V2f, sead::Vector2f)
AL_PARAMETER_TYPE_DEF(V3f, sead::Vector3f)
AL_PARAMETER_TYPE_DEF(V4f, sead::Vector4f)
AL_PARAMETER_TYPE_DEF(Q4f, sead::Quatf)
AL_PARAMETER_TYPE_DEF(C4f, sead::Color4f)
AL_PARAMETER_TYPE_DEF(String32, sead::FixedSafeString<32>)
AL_PARAMETER_TYPE_DEF(String64, sead::FixedSafeString<64>)
AL_PARAMETER_TYPE_DEF(String256, sead::FixedSafeString<256>)
AL_PARAMETER_TYPE_DEF(StringRef, const char*)

#undef AL_PARAMETER_TYPE_DEF

class ParameterObj {
public:
    ParameterObj();

    void pushBackListNode(ParameterBase* pParam);
    void tryGetParam(const ByamlIter& rIter);
    void addArray(ParameterArray* pArray, const sead::SafeString& rKey);
    bool isEqual(const ParameterObj& rObj) const;
    void copy(const ParameterObj& rObj);
    void copyLerp(const ParameterObj& rObjA, const ParameterObj& rObjB, f32 rate);

    ParameterBase* getRootParam() const { return mRootParam; }

    ParameterObj* getNext() const { return mNext; }

    ParameterArray* getParamArray() const { return mParamArray; }

    void setNext(ParameterObj* pObj) { mNext = pObj; }

    void setKey(const sead::SafeString& rKey) { mKey = rKey; }

private:
    ParameterBase* mRootParam = nullptr;
    ParameterBase* mTailParam = nullptr;
    ParameterObj* mNext = nullptr;
    ParameterArray* mParamArray = nullptr;
    sead::FixedSafeString<0x40> mKey;
};

class ParameterArray {
public:
    ParameterArray();

    void tryGetParam(const ByamlIter& rIter);
    bool isEqual(const ParameterArray& rArray) const;
    void copy(const ParameterArray& rArray);
    void copyLerp(const ParameterArray& rArrayA, const ParameterArray& rArrayB, f32 rate);
    void addObj(ParameterObj* pObj);
    void clearObj();
    void removeObj(ParameterObj* pObj);
    bool isExistObj(ParameterObj* pObj);

    ParameterObj* getRootObjNode() const { return mRootObjNode; }

    ParameterArray* getNext() const { return mNext; }

    void setNext(ParameterArray* pArray) { mNext = pArray; }

    void setKey(const sead::SafeString& rKey) { mKey = rKey; }

    const sead::SafeString& getKey() const { return mKey; }

    s32 getSize() const { return mSize; }

private:
    ParameterObj* mRootObjNode = nullptr;
    ParameterArray* mNext = nullptr;
    sead::FixedSafeString<0x40> mKey;
    s32 mSize = 0;
};

class ParameterList {
public:
    ParameterList();

    void addList(ParameterList* pList, const sead::SafeString& rKey);
    void addObj(ParameterObj* pObj, const sead::SafeString& rKey);
    void addArray(ParameterArray* pArray, const sead::SafeString& rKey);
    void clearList();
    void clearObj();
    void removeList(ParameterList* pList);
    void removeObj(ParameterObj* pObj);
    bool isExistObj(ParameterObj* pObj);
    void tryGetParam(const ByamlIter& rIter);

    ParameterList* getNext() const { return mNext; }

    void setNext(ParameterList* pList) { mNext = pList; }

    void setKey(const sead::SafeString& rKey) { mKey = rKey; }

private:
    ParameterObj* mRootObjNode = nullptr;
    ParameterList* mRootListNode = nullptr;
    ParameterArray* mRootArrayNode = nullptr;
    ParameterList* mNext = nullptr;
    sead::FixedSafeString<0x40> mKey;
};
}  // namespace al
