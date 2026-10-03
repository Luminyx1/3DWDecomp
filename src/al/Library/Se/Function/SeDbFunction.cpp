#include "Library/Se/Function/SeDbFunction.hpp"

#include <cstdio>

#include "Library/Math/InOutParam.hpp"
#include "Library/Se/Info/SeAudioInfo.hpp"
#include "Project/Base/StringUtil.hpp"

namespace {
const char* const cInputFunctionNames[] = {"None", "Minus", "Abs", "Square"};
}

namespace alSeDbFunction {
using PlayInfoList = al::AudioInfoList<al::SePlayInfoInAction>;

/**
 * @brief Counts the one time play information in all actions of SE user information.
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
        const al::SeActionInfo* pActionInfo =
            pUserInfo->mActionInfoList != nullptr ? pUserInfo->mActionInfoList->getInfo(i) : nullptr;
        const PlayInfoList* pPlayInfoList = pActionInfo->mPlayInfoList;

        if (pPlayInfoList == nullptr) {
            continue;
        }

        s32 playNum = pPlayInfoList->getInfoNum();

        for (s32 j = 0; j < playNum; j++) {
            count += pActionInfo->mPlayInfoList->getInfo(j)->mIsOneTime;
        }
    }

    return count;
}

/**
 * @brief Converts an input function name to its id.
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
 * @brief Converts an input function id to its name.
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
 * @brief Applies an input function to a parameter.
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
 * @brief Calculates the linearly interpolated value of an input and output parameter.
 * @param pParam Input and output parameter.
 * @param value Input value.
 * @return Output value.
 */
f32 calcLeapValue(al::InOutParam* pParam, f32 value) { return pParam->calcLeapValue(value); }

/**
 * @brief Creates the default sound source information.
 * @return Always nullptr.
 */
al::SeSoundSourceInfo* createDefaultSoundSourceInfo() { return nullptr; }

/**
 * @brief Creates the default emitter information list.
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
 * @brief Creates a heap copy of a name.
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
} // namespace alSeDbFunction

namespace al {
/** @brief Initializes both input and output endpoints to zero. */
InOutParam::InOutParam() = default;

/**
 * @brief Copies the input and output endpoints.
 * @param rOther Parameter range whose four endpoints are copied.
 */
InOutParam::InOutParam(const InOutParam& rOther)
    : mInMin(rOther.mInMin), mInMax(rOther.mInMax), mOutMin(rOther.mOutMin), mOutMax(rOther.mOutMax) {}

/**
 * @brief Constructs an input-to-output range mapping without validating endpoint order.
 * @param inMin Lower input endpoint.
 * @param inMax Upper input endpoint.
 * @param outMin Output value corresponding to inMin.
 * @param outMax Output value corresponding to inMax.
 */
InOutParam::InOutParam(f32 inMin, f32 inMax, f32 outMin, f32 outMax)
    : mInMin(inMin), mInMax(inMax), mOutMin(outMin), mOutMax(outMax) {}

} // namespace al
