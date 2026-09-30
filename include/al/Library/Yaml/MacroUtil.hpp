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
    IUseYamlParam(const char* pName);

    virtual void clearPtr() = 0;
    virtual YamlClassId getClassId() const = 0;
    virtual bool isValidPtr() const = 0;
    virtual void setPtr_u8(u8* pValue) {}
    virtual void setPtr_u16(u16* pValue) {}
    virtual void setPtr_s16(s16* pValue) {}
    virtual void setPtr_V2f(sead::Vector2f* pValue) {}
    virtual void setPtr_V3f(sead::Vector3f* pValue) {}
    virtual void setPtr_YamlColor(sead::Color4f* pValue) {}
    virtual void setPtr_s32(s32* pValue) {}
    virtual void setPtr_f32(f32* pValue) {}
    virtual void setPtr_bool(bool* pValue) {}
    virtual void setPtr_YamlString(const char** pValue) {}

    bool isEqualParamName(const char* pName) const;

    const char* getName() const { return mName; }
    IUseYamlParam* getNext() const { return mNext; }
    void setNext(IUseYamlParam* pParam) { mNext = pParam; }

private:
    const char* mName;
    IUseYamlParam* mNext = nullptr;
};

class YamlParamGroup {
public:
    YamlParamGroup() = default;

    template <typename T>
    void setParamPtr(const char* pName, T* pValue) {
        for (IUseYamlParam* param = mHeadParam; param; param = param->getNext()) {
            if (!param->isEqualParamName(pName)) {
                continue;
            }

            switch (param->getClassId()) {
            case YamlClassId::U8:
                param->setPtr_u8(reinterpret_cast<u8*>(pValue));
                break;
            case YamlClassId::U16:
                param->setPtr_u16(reinterpret_cast<u16*>(pValue));
                break;
            case YamlClassId::S16:
                param->setPtr_s16(reinterpret_cast<s16*>(pValue));
                break;
            case YamlClassId::V2f:
                param->setPtr_V2f(reinterpret_cast<sead::Vector2f*>(pValue));
                break;
            case YamlClassId::V3f:
                param->setPtr_V3f(reinterpret_cast<sead::Vector3f*>(pValue));
                break;
            case YamlClassId::Color:
                param->setPtr_YamlColor(reinterpret_cast<sead::Color4f*>(pValue));
                break;
            case YamlClassId::F32:
                param->setPtr_f32(reinterpret_cast<f32*>(pValue));
                break;
            case YamlClassId::S32:
                param->setPtr_s32(reinterpret_cast<s32*>(pValue));
                break;
            case YamlClassId::Bool:
                param->setPtr_bool(reinterpret_cast<bool*>(pValue));
                break;
            case YamlClassId::String:
                param->setPtr_YamlString(reinterpret_cast<const char**>(pValue));
                break;
            }
        }
    }

    void addParam(IUseYamlParam* pParam);
    void readyToSetPtr();
    void readParam(const al::ByamlIter& rIter);

    static YamlParamGroup* sCurrent;

private:
    IUseYamlParam* mHeadParam = nullptr;
    IUseYamlParam* mTailParam = nullptr;
};

template <typename T>
class YamlParamBase : public IUseYamlParam {
public:
    YamlParamBase(const char* pName) : IUseYamlParam(pName) {}

    T* getParamPtr() const { return mValue; }
    void setParamPtr(T* pValue) { mValue = pValue; }
    void setParam(T value) { *mValue = value; }

    void clearPtr() override { mValue = nullptr; }
    bool isValidPtr() const override { return mValue != nullptr; }

private:
    T* mValue = nullptr;
};

#define AL_YAML_PARAM_CLASS(NAME, TYPE, ID, SETTER)                                                \
    class NAME : public YamlParamBase<TYPE> {                                                      \
    public:                                                                                        \
        NAME(const char* pName) : YamlParamBase(pName) {}                                          \
                                                                                                   \
        YamlClassId getClassId() const override { return YamlClassId::ID; }                        \
                                                                                                   \
        void SETTER(TYPE* pValue) override { setParamPtr(pValue); }                                \
    };

AL_YAML_PARAM_CLASS(YamlParam_u8, u8, U8, setPtr_u8)
AL_YAML_PARAM_CLASS(YamlParam_V2f, sead::Vector2f, V2f, setPtr_V2f)
AL_YAML_PARAM_CLASS(YamlParam_V3f, sead::Vector3f, V3f, setPtr_V3f)
AL_YAML_PARAM_CLASS(YamlParam_YamlColor, sead::Color4f, Color, setPtr_YamlColor)
AL_YAML_PARAM_CLASS(YamlParam_f32, f32, F32, setPtr_f32)
AL_YAML_PARAM_CLASS(YamlParam_s32, s32, S32, setPtr_s32)
AL_YAML_PARAM_CLASS(YamlParam_bool, bool, Bool, setPtr_bool)
AL_YAML_PARAM_CLASS(YamlParam_YamlString, const char*, String, setPtr_YamlString)

#undef AL_YAML_PARAM_CLASS
}  // namespace alYamlMacroUtil
