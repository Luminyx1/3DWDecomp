#include "Library/Yaml/MacroUtil.hpp"

#include "Library/Yaml/ByamlIter.hpp"
#include "Library/Yaml/ByamlUtil.hpp"
#include "Project/Base/StringUtil.hpp"

namespace alYamlMacroUtil {
/**
 * Constructs a parameter and registers it with the current parameter group.
 * @param pName parameter name
 */
IUseYamlParam::IUseYamlParam(const char* pName) : mName(pName) {
    YamlParamGroup::sCurrent->addParam(this);
}

/**
 * Appends a parameter to the group.
 * @param pParam parameter to add
 */
void YamlParamGroup::addParam(IUseYamlParam* pParam) {
    if (mHeadParam == nullptr) {
        mHeadParam = pParam;
        mTailParam = pParam;
        return;
    }

    mTailParam->setNext(pParam);
    mTailParam = pParam;
}

/**
 * Checks whether the parameter has a name.
 * @param pName name to compare with
 * @return true if the names are equal
 */
bool IUseYamlParam::isEqualParamName(const char* pName) const {
    return al::isEqualString(pName, mName);
}

/**
 * Clears the value pointers of all parameters in the group.
 */
void YamlParamGroup::readyToSetPtr() {
    for (IUseYamlParam* param = mHeadParam; param != nullptr; param = param->getNext()) {
        param->clearPtr();
    }
}

/**
 * Reads all parameters of the group from a byaml hash.
 * @param rIter hash iterator
 */
void YamlParamGroup::readParam(const al::ByamlIter& rIter) {
    for (IUseYamlParam* param = mHeadParam; param != nullptr; param = param->getNext()) {
        switch (param->getClassId()) {
        case YamlClassId::U8:
            al::tryGetByamlU8(static_cast<YamlParamBase<u8>*>(param)->getParamPtr(), rIter,
                              param->getName());
            break;
        case YamlClassId::U16:
            al::tryGetByamlU16(static_cast<YamlParamBase<u16>*>(param)->getParamPtr(), rIter,
                               param->getName());
            break;
        case YamlClassId::S16:
            al::tryGetByamlS16(static_cast<YamlParamBase<s16>*>(param)->getParamPtr(), rIter,
                               param->getName());
            break;
        case YamlClassId::V2f:
            al::tryGetByamlV2f(static_cast<YamlParamBase<sead::Vector2f>*>(param)->getParamPtr(),
                               rIter, param->getName());
            break;
        case YamlClassId::V3f:
            al::tryGetByamlV3f(static_cast<YamlParamBase<sead::Vector3f>*>(param)->getParamPtr(),
                               rIter, param->getName());
            break;
        case YamlClassId::Color:
            al::tryGetByamlColor(static_cast<YamlParamBase<sead::Color4f>*>(param)->getParamPtr(),
                                 rIter, param->getName());
            break;
        case YamlClassId::F32:
            al::tryGetByamlF32(static_cast<YamlParamBase<f32>*>(param)->getParamPtr(), rIter,
                               param->getName());
            break;
        case YamlClassId::S32:
            static_cast<YamlParamBase<s32>*>(param)->setParam(
                al::tryGetByamlKeyIntOrZero(rIter, param->getName()));
            break;
        case YamlClassId::Bool:
            *static_cast<YamlParamBase<bool>*>(param)->getParamPtr() =
                al::tryGetByamlKeyBoolOrFalse(rIter, param->getName());
            break;
        case YamlClassId::String:
            static_cast<YamlParamBase<const char*>*>(param)->setParam(
                al::tryGetByamlKeyStringOrNULL(rIter, param->getName()));
            break;
        }
    }
}
}  // namespace alYamlMacroUtil
