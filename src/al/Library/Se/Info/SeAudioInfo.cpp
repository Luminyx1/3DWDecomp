#include "Library/Se/Info/SeAudioInfo.hpp"

#include "Library/Yaml/ByamlIter.hpp"

namespace al {
/**
 * Creates SE resource specific information from BYAML data.
 * @param rIter BYAML data.
 * @return Created information, or nullptr if the sound does not exist.
 */
SeResourceSpecificInfo* SeResourceSpecificInfo::createInfo(const ByamlIter& rIter) {
    SeResourceSpecificInfo* info = new SeResourceSpecificInfo;
    rIter.tryGetStringByKey(&info->mName, "Name");
    u32 soundId = alSoundNameUtil::getSoundId(info->mName, false);
    info->mSoundId = soundId;
    if (AudioConst::SOUND_ID_INVALID == soundId) {
        return nullptr;
    }

    if (!rIter.tryGetIntByKey(&info->mLimitPlayingNum, "LimitPlayingNum")) {
        info->mLimitPlayingNum = 0;
    }

    if (!rIter.tryGetIntByKey(&info->mLimitTriggerFrame, "LimitTriggerFrame")) {
        info->mLimitTriggerFrame = 0;
    }

    if (!rIter.tryGetIntByKey(&info->mLimitTriggerNum, "LimitTriggerNum")) {
        info->mLimitTriggerNum = 0;
    }

    if (!rIter.tryGetIntByKey(&info->mDelayFrame, "DelayFrame")) {
        info->mDelayFrame = 0;
    }

    if (!rIter.tryGetIntByKey(&info->mDelayMaxNum, "DelayMaxNum")) {
        info->mDelayMaxNum = 0;
    }

    if (!rIter.tryGetFloatByKey(&info->mDelayVolume, "DelayVolume")) {
        info->mDelayVolume = -1.0f;
    }

    if (!rIter.tryGetFloatByKey(&info->mVolumeAfterGoal, "VolumeAfterGoal")) {
        info->mVolumeAfterGoal = -1.0f;
    }

    if (!rIter.tryGetBoolByKey(&info->mIsValidMatCodeLpf, "IsValidMatCodeLpf")) {
        info->mIsValidMatCodeLpf = false;
    }

    if (!rIter.tryGetBoolByKey(&info->mIsCmNg, "IsCmNg")) {
        info->mIsCmNg = false;
    }

    if (!rIter.tryGetBoolByKey(&info->mIsIgnoreDistPause, "IsIgnoreDistPause")) {
        info->mIsIgnoreDistPause = false;
    }

    if (!rIter.tryGetBoolByKey(&info->mIsIgnoreInTitleScene, "IsIgnoreInTitleScene")) {
        info->mIsIgnoreInTitleScene = false;
    }

    ByamlIter materialIter;
    if (rIter.tryGetIterByKey(&materialIter, "MaterialInfoList")) {
        info->mMaterialInfoList = createInfoList<SeMaterialSettingInfo>(materialIter);
    } else {
        info->mMaterialInfoList = nullptr;
    }

    return info;
}

/**
 * Compares two SE resource specific information by name.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 SeResourceSpecificInfo::compareInfo(const SeResourceSpecificInfo* pA, const SeResourceSpecificInfo* pB) {
    return strcmp(pA->mName, pB->mName);
}

}  // namespace al
