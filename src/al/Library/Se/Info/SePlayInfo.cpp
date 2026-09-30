#include "Library/Se/Info/SeAudioInfo.hpp"

#include "Library/Math/InOutParam.hpp"
#include "Library/Se/Function/SeDbFunction.hpp"
#include "Library/Yaml/ByamlIter.hpp"

namespace al {
/**
 * Creates SE resource information from BYAML data.
 * @param rIter BYAML data.
 * @return Created information, or nullptr if the sound does not exist.
 */
SeResourceInfo* SeResourceInfo::createInfo(const ByamlIter& rIter) {
    SeResourceInfo* info = new SeResourceInfo;
    rIter.tryGetStringByKey(&info->mName, "Name");
    u32 soundId = alSoundNameUtil::getSoundId(info->mName, false);
    info->mSoundId = soundId;

    if (AudioConst::SOUND_ID_INVALID == soundId) {
        return nullptr;
    }

    const char* inputFunctionName = nullptr;

    if (rIter.tryGetStringByKey(&inputFunctionName, "InputFunctionName")) {
        info->mInputFunctionId = alSeDbFunction::convertInputFunctionNameToId(inputFunctionName);
    } else {
        info->mInputFunctionId = 0;
    }

    {
        ByamlIter pitchIter;
        InOutParam* pitch;

        if (rIter.tryGetIterByKey(&pitchIter, "Pitch")) {
            pitch = new InOutParam(0.0f, 0.0f, 1.0f, 1.0f);
            pitch->init(pitchIter);
        } else {
            pitch = nullptr;
        }

        info->mPitch = pitch;
    }

    {
        ByamlIter volumeIter;
        InOutParam* volume;

        if (rIter.tryGetIterByKey(&volumeIter, "Volume")) {
            volume = new InOutParam(0.0f, 0.0f, 1.0f, 1.0f);
            volume->init(volumeIter);
        } else {
            volume = nullptr;
        }

        info->mVolume = volume;
    }

    {
        ByamlIter tempoIter;
        InOutParam* tempo;

        if (rIter.tryGetIterByKey(&tempoIter, "Tempo")) {
            tempo = new InOutParam(0.0f, 0.0f, 1.0f, 1.0f);
            tempo->init(tempoIter);
        } else {
            tempo = nullptr;
        }

        info->mTempo = tempo;
    }

    if (!rIter.tryGetStringByKey(&info->mEmitterName, "EmitterName")) {
        info->mEmitterName = nullptr;
    }

    if (rIter.tryGetFloatByKey(&info->mParamMin, "ParamMin")) {
        info->mIsSetParamMin = true;
    }

    if (!rIter.tryGetIntByKey(&info->mLocalVarNo, "LocalVarNo")) {
        info->mLocalVarNo = -1;
    }

    if (!rIter.tryGetFloatByKey(&info->mLfeSend, "LfeSend")) {
        info->mLfeSend = 0.0f;
    }

    return info;
}

/**
 * Creates SE play information from BYAML data.
 * @param rIter BYAML data.
 * @return Created information, or nullptr if there is no resource information.
 */
SePlayInfo* SePlayInfo::createInfo(const ByamlIter& rIter) {
    SePlayInfo* info = new SePlayInfo;
    rIter.tryGetStringByKey(&info->mName, "Name");

    if (!rIter.tryGetBoolByKey(&info->mIsLoop, "IsLevel") && !rIter.tryGetBoolByKey(&info->mIsLoop, "IsLoop")) {
        info->mIsLoop = false;
    }

    if (!rIter.tryGetIntByKey(&info->mFadeOutFrameNum, "FadeOutFrameNum")) {
        info->mFadeOutFrameNum = USE_DEFAULT_FADE_OUT_FRAME_NUM;
    }

    if (!rIter.tryGetStringByKey(&info->mRequestKeeperName, "RequestKeeperName")) {
        info->mRequestKeeperName = nullptr;
    }

    ByamlIter resourceIter;

    if (!rIter.tryGetIterByKey(&resourceIter, "ResourceInfoList")) {
        return nullptr;
    }

    info->mResourceInfoList = createInfoList<SeResourceInfo>(resourceIter);
    return info;
}

/**
 * Compares two SE resource information by name.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 SeResourceInfo::compareInfo(const SeResourceInfo* pA, const SeResourceInfo* pB) {
    return strcmp(pA->mName, pB->mName);
}

/**
 * Compares two SE play information by name.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 SePlayInfo::compareInfo(const SePlayInfo* pA, const SePlayInfo* pB) {
    return strcmp(pA->mName, pB->mName);
}

}  // namespace al
