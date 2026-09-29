#pragma once

#include <basis/seadTypes.h>
#include <gfx/seadColor.h>
#include <math/seadVector.h>

namespace al {
class ByamlIter;
}  // namespace al

namespace alYamlMacroUtil {
enum class YamlClassId : s32 {
    U8 = 0x54,
    U16 = 0x55,
    S16 = 0x56,
    V2f = 0x57,
    V3f = 0x58,
    Color = 0x59,
    F32 = 0x5a,
    S32 = 0x5b,
    Bool = 0x5c,
    String = 0x5d,
};

class IUseYamlParam {
public:
    IUseYamlParam(const char*);

    virtual void clearPtr() = 0;
    virtual YamlClassId getClassId() const = 0;
    virtual bool isValidPtr() const = 0;

    bool isEqualParamName(const char*) const;

    const char* mName;               // _8
    IUseYamlParam* mNext = nullptr;  // _10
};

class YamlParamGroup {
public:
    void addParam(IUseYamlParam*);
    void readyToSetPtr();
    void readParam(const al::ByamlIter&);

    static YamlParamGroup* sCurrent;

    IUseYamlParam* mHeadParam = nullptr;  // _0
    IUseYamlParam* mTailParam = nullptr;  // _8
};

template <typename T>
class YamlParamBase : public IUseYamlParam {
public:
    YamlParamBase(const char* pName) : IUseYamlParam(pName) {}

    void clearPtr() override { mValue = nullptr; }
    bool isValidPtr() const override { return mValue != nullptr; }

    T* mValue = nullptr;  // _18
};
}  // namespace alYamlMacroUtil
