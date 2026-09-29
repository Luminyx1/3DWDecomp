#include "Library/Yaml/MacroUtil.hpp"

#include "Library/Yaml/ByamlUtil.hpp"
#include "Project/Base/StringUtil.hpp"

namespace alYamlMacroUtil {
/**
 * @brief Constructs a named yaml parameter and adds it to the current parameter group.
 * @param pName The key of the parameter in the byaml data.
 */
IUseYamlParam::IUseYamlParam(const char* pName) : mName(pName) {
    YamlParamGroup::sCurrent->addParam(this);
}

/**
 * @brief Appends a parameter to the end of the group.
 * @param pParam The parameter to append.
 */
void YamlParamGroup::addParam(IUseYamlParam* pParam) {
    if (mHeadParam == nullptr) {
        mHeadParam = pParam;
        mTailParam = pParam;
        return;
    }

    mTailParam->mNext = pParam;
    mTailParam = pParam;
}

/**
 * @brief Checks whether the parameter has a given name.
 * @param pName The name to compare against.
 * @return True if the names are equal.
 */
bool IUseYamlParam::isEqualParamName(const char* pName) const {
    return al::isEqualString(pName, mName);
}

/**
 * @brief Clears the value pointer of every parameter in the group.
 */
void YamlParamGroup::readyToSetPtr() {
    for (IUseYamlParam* pParam = mHeadParam; pParam != nullptr; pParam = pParam->mNext) {
        pParam->clearPtr();
    }
}

/**
 * @brief Reads every parameter of the group from byaml data into its value pointer.
 * @param rIter The byaml iterator to read the parameters from.
 */
void YamlParamGroup::readParam(const al::ByamlIter& rIter) {
    for (IUseYamlParam* pParam = mHeadParam; pParam != nullptr; pParam = pParam->mNext) {
        switch (pParam->getClassId()) {
        case YamlClassId::U8:
            al::tryGetByamlU8(static_cast<YamlParamBase<u8>*>(pParam)->mValue, rIter,
                              pParam->mName);
            break;
        case YamlClassId::U16:
            al::tryGetByamlU16(static_cast<YamlParamBase<u16>*>(pParam)->mValue, rIter,
                               pParam->mName);
            break;
        case YamlClassId::S16:
            al::tryGetByamlS16(static_cast<YamlParamBase<s16>*>(pParam)->mValue, rIter,
                               pParam->mName);
            break;
        case YamlClassId::V2f:
            al::tryGetByamlV2f(static_cast<YamlParamBase<sead::Vector2f>*>(pParam)->mValue, rIter,
                               pParam->mName);
            break;
        case YamlClassId::V3f:
            al::tryGetByamlV3f(static_cast<YamlParamBase<sead::Vector3f>*>(pParam)->mValue, rIter,
                               pParam->mName);
            break;
        case YamlClassId::Color:
            al::tryGetByamlColor(static_cast<YamlParamBase<sead::Color4f>*>(pParam)->mValue,
                                 rIter, pParam->mName);
            break;
        case YamlClassId::F32:
            al::tryGetByamlF32(static_cast<YamlParamBase<f32>*>(pParam)->mValue, rIter,
                               pParam->mName);
            break;
        case YamlClassId::S32: {
            s32 value = al::tryGetByamlKeyIntOrZero(rIter, pParam->mName);
            *static_cast<YamlParamBase<s32>*>(pParam)->mValue = value;
            break;
        }
        case YamlClassId::Bool: {
            *static_cast<YamlParamBase<bool>*>(pParam)->mValue =
                al::tryGetByamlKeyBoolOrFalse(rIter, pParam->mName);
            break;
        }
        case YamlClassId::String: {
            const char* value = al::tryGetByamlKeyStringOrNULL(rIter, pParam->mName);
            *static_cast<YamlParamBase<const char*>*>(pParam)->mValue = value;
            break;
        }
        default:
            break;
        }
    }
}
}  // namespace alYamlMacroUtil
