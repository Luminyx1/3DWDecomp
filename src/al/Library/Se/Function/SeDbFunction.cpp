#include "Library/Se/Function/SeDbFunction.hpp"

#include <cstdio>

#include "Library/Math/InOutParam.hpp"
#include "Library/Se/Info/SeAudioInfo.hpp"
#include "Project/Base/StringUtil.hpp"

namespace {
const char* const cInputFunctionNames[] = {"None", "Minus", "Abs", "Square"};
}

namespace alSeDbFunction {
/**
 * Counts the one time play information in all actions of SE user information.
 * @param pUserInfo SE user information.
 * @return Number of one time play information.
 */
s32 calcIsOneTimeInUserInfo(const al::SeUserInfo* pUserInfo) {
    if (pUserInfo->mActionInfoList == nullptr) {
        return 0;
    }

    s32 actionNum = pUserInfo->mActionInfoList->getInfoNum();
    s32 count = 0;

    for (s32 i = 0; i < actionNum; i++) {
        const al::SeActionInfo* actionInfo =
            pUserInfo->mActionInfoList != nullptr ? pUserInfo->mActionInfoList->getInfo(i) : nullptr;
        const al::AudioInfoList<al::SePlayInfoInAction>* playInfoList = actionInfo->mPlayInfoList;

        if (playInfoList == nullptr) {
            continue;
        }

        s32 playNum = playInfoList->getInfoNum();

        for (s32 j = 0; j < playNum; j++) {
            count += actionInfo->mPlayInfoList->getInfo(j)->mIsOneTime;
        }
    }

    return count;
}

/**
 * Converts an input function name to its id.
 * @param pName Input function name.
 * @return Input function id.
 */
al::SeInputFunctionId convertInputFunctionNameToId(const char* pName) {
    if (al::isEqualString(pName, "Minus")) {
        return static_cast<al::SeInputFunctionId>(1);
    }

    if (al::isEqualString(pName, "Abs")) {
        return static_cast<al::SeInputFunctionId>(2);
    }

    if (al::isEqualString(pName, "Square")) {
        return static_cast<al::SeInputFunctionId>(3);
    }

    return static_cast<al::SeInputFunctionId>(0);
}

/**
 * Converts an input function id to its name.
 * @param id Input function id.
 * @return Input function name.
 */
const char* convertInputFunctionIdToName(al::SeInputFunctionId id) {
    if (static_cast<u32>(id) <= 3) {
        return cInputFunctionNames[id];
    }

    return "InvalidID";
}

/**
 * Applies an input function to a parameter.
 * @param id Input function id.
 * @param param Parameter.
 * @return Converted parameter.
 */
f32 convertSeInputParam(al::SeInputFunctionId id, f32 param) {
    switch (id) {
    case 1:
        return -param;
    case 2:
        return param > 0.0f ? param : -param;
    case 3:
        return param * param;
    default:
        return param;
    }
}

/**
 * Calculates the linearly interpolated value of an input and output parameter.
 * @param pParam Input and output parameter.
 * @param value Input value.
 * @return Output value.
 */
f32 calcLeapValue(al::InOutParam* pParam, f32 value) {
    return pParam->calcLeapValue(value);
}

/**
 * Creates the default sound source information.
 * @return Always nullptr.
 */
al::SeSoundSourceInfo* createDefaultSoundSourceInfo() {
    return nullptr;
}

/**
 * Creates the default emitter information list.
 * @return Created list.
 */
al::AudioInfoList<al::SeEmitterInfo>* createDefaultEmitterInfoList() {
    al::AudioInfoList<al::SeEmitterInfo>* list = new al::AudioInfoList<al::SeEmitterInfo>;
    list->mNext = nullptr;
    list->mInfos = new sead::PtrArray<al::SeEmitterInfo>;
    list->mInfos->allocBuffer(2, nullptr);
    al::SeEmitterInfo* info = new al::SeEmitterInfo;
    info->mName = "Default";
    list->mInfos->pushBack(info);
    return list;
}

/**
 * Creates a heap copy of a name.
 * @param pName Name.
 * @return Created copy, or nullptr.
 */
const char* createNameAreaAndCopy(const char* pName) {
    if (pName == nullptr) {
        return nullptr;
    }

    al::StringTmp<128> name(pName);
    s32 size = name.calcLength() + 1;
    char* buffer = new char[size];
    snprintf(buffer, size, "%s", name.cstr());
    return buffer;
}
}  // namespace alSeDbFunction
