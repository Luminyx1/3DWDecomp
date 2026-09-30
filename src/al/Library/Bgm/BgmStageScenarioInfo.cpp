#include "Library/Bgm/BgmDataBase.hpp"

#include "Library/Resource/Resource.hpp"
#include "Library/Resource/ResourceFunction.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Bgm/BgmInfo.hpp"

namespace al {
/**
 * Creates BGM stage play information from BYAML data.
 * @param rIter BYAML data.
 * @return Created information.
 */
BgmStagePlayInfo* BgmStagePlayInfo::createInfo(const ByamlIter& rIter) {
    BgmStagePlayInfo* info = new BgmStagePlayInfo();
    rIter.tryGetStringByKey(&info->mPlayInfoName, "PlayInfoName");
    rIter.tryGetStringByKey(&info->mResourceName, "ResourceName");
    if (!rIter.tryGetStringByKey(&info->mRegionInfoListName, "RegionInfoListName")) {
        info->mRegionInfoListName = nullptr;
    }

    if (!rIter.tryGetIntByKey(&info->mStartDelayFrameNum, "StartDelayFrameNum")) {
        info->mStartDelayFrameNum = 0;
    }

    if (!rIter.tryGetIntByKey(&info->mFadeInFrameNum, "FadeInFrameNum")) {
        info->mFadeInFrameNum = 0;
    }

    return info;
}

/**
 * Creates BGM stage information from BYAML data.
 * @param rIter BYAML data.
 * @return Created information.
 */
BgmStageInfo* BgmStageInfo::createInfo(const ByamlIter& rIter) {
    BgmStageInfo* info = new BgmStageInfo();
    rIter.tryGetStringByKey(&info->mName, "Name");
    ByamlIter playIter;
    rIter.tryGetIterByKey(&playIter, "StagePlayInfo");
    info->mStagePlayInfoList = createInfoList<BgmStagePlayInfo>(playIter);
    return info;
}

/**
 * Compares two BGM stage play information by play information name.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 BgmStagePlayInfo::compareInfo(const BgmStagePlayInfo* pA, const BgmStagePlayInfo* pB) {
    return strcmp(pA->mPlayInfoName, pB->mPlayInfoName);
}

/**
 * Compares two BGM stage information by name.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 BgmStageInfo::compareInfo(const BgmStageInfo* pA, const BgmStageInfo* pB) {
    return strcmp(pA->mName, pB->mName);
}

/**
 * Creates BGM suffix process information from BYAML data.
 * @param rIter BYAML data.
 * @param pProcInfoName Process name.
 * @return Created information.
 */
BgmSuffixProcInfo* BgmSuffixProcInfo::createInfo(const ByamlIter& rIter, const char* pProcInfoName) {
    BgmSuffixProcInfo* info = new BgmSuffixProcInfo();
    info->mProcInfoName = pProcInfoName;
    if (isEqualString(pProcInfoName, "AttachSuffix")) {
        rIter.tryGetStringByKey(&info->mSuffixName, "SuffixName");
    } else {
        info->mSuffixName = nullptr;
    }

    if (!rIter.tryGetBoolByKey(&info->mIsStartCurPosition, "IsStartCurPosition")) {
        info->mIsStartCurPosition = false;
    }

    return info;
}

/**
 * Creates BGM volume process information from BYAML data.
 * @param rIter BYAML data.
 * @param pProcInfoName Process name.
 * @return Created information.
 */
BgmVolumeProcInfo* BgmVolumeProcInfo::createInfo(const ByamlIter& rIter, const char* pProcInfoName) {
    BgmVolumeProcInfo* info = new BgmVolumeProcInfo();
    info->mProcInfoName = pProcInfoName;
    if (!rIter.tryGetFloatByKey(&info->mTargetVolume, "TargetVolume")) {
        info->mTargetVolume = 1.0f;
    }

    if (!rIter.tryGetFloatByKey(&info->mVolumeDiff, "VolumeDiff")) {
        info->mVolumeDiff = 0.0f;
    }

    return info;
}

/**
 * Creates BGM track process information from BYAML data.
 * @param rIter BYAML data.
 * @param pProcInfoName Process name.
 * @return Created information.
 */
BgmTrackProcInfo* BgmTrackProcInfo::createInfo(const ByamlIter& rIter, const char* pProcInfoName) {
    BgmTrackProcInfo* info = new BgmTrackProcInfo();
    info->mProcInfoName = pProcInfoName;
    ByamlIter listIter;
    if (rIter.tryGetIterByKey(&listIter, "ChangeTrackInfoList")) {
        s32 size = listIter.getSize();
        AudioInfoList<BgmTrackChangeInfo>* list = new AudioInfoList<BgmTrackChangeInfo>;
        list->mNext = nullptr;
        list->mInfos = new sead::PtrArray<BgmTrackChangeInfo>;
        list->mInfos->allocBuffer(size + 1, nullptr);
        for (s32 i = 0; i < size; i++) {
            BgmTrackChangeInfo* changeInfo = new BgmTrackChangeInfo();
            ByamlIter iter;
            listIter.tryGetIterByIndex(&iter, i);
            iter.tryGetIntByKey(&changeInfo->mTrackNo, "TrackNo");
            iter.tryGetFloatByKey(&changeInfo->mVolume, "Volume");
            if (!iter.tryGetIntByKey(&changeInfo->mFadeFrameNum, "FadeFrameNum")) {
                changeInfo->mFadeFrameNum = -1;
            }

            list->mInfos->pushBack(changeInfo);
        }

        info->mChangeTrackInfoList = list;
    }

    return info;
}

/**
 * Creates BGM region process information from BYAML data.
 * @param rIter BYAML data.
 * @param pProcInfoName Process name.
 * @return Created information.
 */
BgmRegionProcInfo* BgmRegionProcInfo::createInfo(const ByamlIter& rIter, const char* pProcInfoName) {
    BgmRegionProcInfo* info = new BgmRegionProcInfo();
    info->mProcInfoName = pProcInfoName;
    if (!rIter.tryGetIntByKey(&info->mHeadNo, "HeadNo")) {
        info->mHeadNo = 0;
    }

    if (!rIter.tryGetIntByKey(&info->mLoopStartNo, "LoopStartNo")) {
        info->mLoopStartNo = 0;
    }

    if (!rIter.tryGetIntByKey(&info->mLoopEndNo, "LoopEndNo")) {
        info->mLoopEndNo = 0;
    }

    if (!rIter.tryGetBoolByKey(&info->mIsLoop, "IsLoop")) {
        info->mIsLoop = true;
    }

    if (!rIter.tryGetBoolByKey(&info->mIsPlayHeadOneTime, "IsPlayHeadOneTime")) {
        info->mIsPlayHeadOneTime = false;
    }

    if (!rIter.tryGetStringByKey(&info->mNextSituationName, "NextSituationName")) {
        info->mNextSituationName = nullptr;
    }

    return info;
}

/**
 * Creates BGM pitch process information from BYAML data.
 * @param rIter BYAML data.
 * @param pProcInfoName Process name.
 * @return Created information.
 */
BgmPitchProcInfo* BgmPitchProcInfo::createInfo(const ByamlIter& rIter, const char* pProcInfoName) {
    BgmPitchProcInfo* info = new BgmPitchProcInfo();
    info->mProcInfoName = pProcInfoName;
    if (!rIter.tryGetFloatByKey(&info->mTargetPitch, "TargetPitch")) {
        info->mTargetPitch = 1.0f;
    }

    if (!rIter.tryGetFloatByKey(&info->mPitchDiff, "PitchDiff")) {
        info->mPitchDiff = 0.0f;
    }

    return info;
}

/**
 * Creates BGM pitch modulation process information from BYAML data.
 * @param rIter BYAML data.
 * @param pProcInfoName Process name.
 * @return Created information.
 */
BgmPitchModulationProcInfo* BgmPitchModulationProcInfo::createInfo(const ByamlIter& rIter,
                                                                   const char* pProcInfoName) {
    BgmPitchModulationProcInfo* info = new BgmPitchModulationProcInfo();
    info->mProcInfoName = pProcInfoName;
    if (!rIter.tryGetBoolByKey(&info->mIsEnable, "IsEnable")) {
        info->mIsEnable = false;
    }

    if (!rIter.tryGetFloatByKey(&info->mModDepth, "ModDepth")) {
        info->mModDepth = 0.01f;
    }

    if (!rIter.tryGetFloatByKey(&info->mModDepthDiff, "ModDepthDiff")) {
        info->mModDepthDiff = 1.0f;
    }

    if (!rIter.tryGetFloatByKey(&info->mModFreq, "ModFreq")) {
        info->mModFreq = 1.0f;
    }

    return info;
}

/**
 * Creates BGM low pass filter process information from BYAML data.
 * @param rIter BYAML data.
 * @param pProcInfoName Process name.
 * @return Created information.
 */
BgmLpfProcInfo* BgmLpfProcInfo::createInfo(const ByamlIter& rIter, const char* pProcInfoName) {
    BgmLpfProcInfo* info = new BgmLpfProcInfo();
    info->mProcInfoName = pProcInfoName;
    if (!rIter.tryGetFloatByKey(&info->mCutOffFreq, "CutOffFreq")) {
        info->mCutOffFreq = 0.0f;
    }

    if (!rIter.tryGetFloatByKey(&info->mCutOffFreqDiff, "CutOffFreqDiff")) {
        info->mCutOffFreqDiff = 0.0f;
    }

    return info;
}

/**
 * Creates BGM loop start move process information.
 * @param rIter BYAML data.
 * @param pProcInfoName Process name.
 * @return Created information.
 */
BgmMoveLoopStartProcInfo* BgmMoveLoopStartProcInfo::createInfo(const ByamlIter& rIter, const char* pProcInfoName) {
    BgmMoveLoopStartProcInfo* info = new BgmMoveLoopStartProcInfo();
    info->mProcInfoName = pProcInfoName;
    return info;
}

/**
 * Creates BGM process information of the type given in the BYAML data.
 * @param rIter BYAML data.
 * @return Created information, or nullptr for unknown types.
 */
BgmProcInfo* BgmProcInfo::createInfo(const ByamlIter& rIter) {
    const char* procInfoName;
    rIter.tryGetStringByKey(&procInfoName, "ProcInfoName");
    if (isEqualString(procInfoName, "AttachSuffix") || isEqualString(procInfoName, "DetachSuffix")) {
        return BgmSuffixProcInfo::createInfo(rIter, procInfoName);
    }

    if (isEqualString(procInfoName, "ChangeVolume")) {
        return BgmVolumeProcInfo::createInfo(rIter, procInfoName);
    }

    if (isEqualString(procInfoName, "ChangeTrack")) {
        return BgmTrackProcInfo::createInfo(rIter, procInfoName);
    }

    if (isEqualString(procInfoName, "ChangeRegion")) {
        return BgmRegionProcInfo::createInfo(rIter, procInfoName);
    }

    if (isEqualString(procInfoName, "ChangePitch")) {
        return BgmPitchProcInfo::createInfo(rIter, procInfoName);
    }

    if (isEqualString(procInfoName, "ModulatePitch")) {
        return BgmPitchModulationProcInfo::createInfo(rIter, procInfoName);
    }

    if (isEqualString(procInfoName, "Lpf")) {
        return BgmLpfProcInfo::createInfo(rIter, procInfoName);
    }

    if (isEqualString(procInfoName, "MoveLoopStart")) {
        return BgmMoveLoopStartProcInfo::createInfo(rIter, procInfoName);
    }

    return nullptr;
}

/**
 * Creates BGM sub situation information from BYAML data.
 * @param rIter BYAML data.
 * @return Created information.
 */
BgmSubSituationInfo* BgmSubSituationInfo::createInfo(const ByamlIter& rIter) {
    BgmSubSituationInfo* info = new BgmSubSituationInfo();
    rIter.tryGetStringByKey(&info->mName, "Name");
    ByamlIter procIter;
    rIter.tryGetIterByKey(&procIter, "ProcInfoList");
    info->mProcInfoList = createInfoList<BgmProcInfo>(procIter);
    return info;
}

/**
 * Creates BGM situation information from BYAML data.
 * @param rIter BYAML data.
 * @return Created information.
 */
BgmSituationInfo* BgmSituationInfo::createInfo(const ByamlIter& rIter) {
    BgmSituationInfo* info = new BgmSituationInfo();
    rIter.tryGetStringByKey(&info->mName, "Name");
    ByamlIter subIter;
    rIter.tryGetIterByKey(&subIter, "SubSituationInfoList");
    info->mSubSituationInfoList = createInfoList<BgmSubSituationInfo>(subIter);
    return info;
}

/**
 * Compares two BGM process information by process name.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 BgmProcInfo::compareInfo(const BgmProcInfo* pA, const BgmProcInfo* pB) {
    return strcmp(pA->mProcInfoName, pB->mProcInfoName);
}

/**
 * Compares two BGM sub situation information by name.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 BgmSubSituationInfo::compareInfo(const BgmSubSituationInfo* pA, const BgmSubSituationInfo* pB) {
    return strcmp(pA->mName, pB->mName);
}

/**
 * Compares two BGM situation information by name.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 BgmSituationInfo::compareInfo(const BgmSituationInfo* pA, const BgmSituationInfo* pB) {
    return strcmp(pA->mName, pB->mName);
}

/**
 * Compares two BGM track change information by track number.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 BgmTrackChangeInfo::compareInfo(const BgmTrackChangeInfo* pA, const BgmTrackChangeInfo* pB) {
    if (pA->mTrackNo < pB->mTrackNo) {
        return -1;
    }

    return pA->mTrackNo > pB->mTrackNo;
}

/**
 * Loads all BGM information from the BGM database resource.
 */
BgmDataBase::BgmDataBase() {
    Resource* resource = findOrCreateResource("SoundData/BgmDataBase", nullptr);

    {
        ByamlIter rootIter(resource->getByml("BgmLineInfoList"));
        ByamlIter iter;
        mCombinedLineInfoList =
            rootIter.tryGetIterByKey(&iter, "CombinedLineInfoList") ? createInfoList<BgmCombinedLineInfo>(iter) : nullptr;
    }

    {
        ByamlIter rootIter(resource->getByml("BgmPlayInfoList"));
        ByamlIter iter;
        if (rootIter.tryGetIterByKey(&iter, "PlayInfoList")) {
            mPlayInfoList = createInfoList<BgmPlayInfo>(iter);
        } else {
            mPlayInfoList = nullptr;
        }
    }

    {
        ByamlIter rootIter(resource->getByml("BgmResourceInfoList"));
        ByamlIter iter;
        if (rootIter.tryGetIterByKey(&iter, "ResourceInfoList")) {
            mResourceInfoList = createInfoList<BgmResourceInfo>(iter);
        } else {
            mResourceInfoList = nullptr;
        }
    }

    {
        ByamlIter rootIter(resource->getByml("BgmStageInfoList"));
        ByamlIter iter;
        if (rootIter.tryGetIterByKey(&iter, "StageInfoList")) {
            mStageInfoList = createInfoList<BgmStageInfo>(iter);
        } else {
            mStageInfoList = nullptr;
        }
    }

    {
        ByamlIter rootIter(resource->getByml("BgmSituationInfoList"));
        ByamlIter iter;
        if (rootIter.tryGetIterByKey(&iter, "SituationInfoList")) {
            mSituationInfoList = createInfoList<BgmSituationInfo>(iter);
        } else {
            mSituationInfoList = nullptr;
        }
    }

    sead::PtrArray<BgmUserInfo>* userInfoList = new sead::PtrArray<BgmUserInfo>;
    s32 entryNum = resource->getEntryNum("/");
    if (entryNum >= 1) {
        userInfoList->allocBuffer(entryNum, nullptr);
        for (s32 i = 0; i < entryNum; i++) {
            StringTmp<128> fileName;
            resource->getEntryName(&fileName, "/", i);
            const char* name = fileName.cstr();
            if (isEqualString(name, "BgmLineInfoList.byml") || isEqualString(name, "BgmPlayInfoList.byml") ||
                isEqualString(name, "BgmResourceInfoList.byml") || isEqualString(name, "BgmSituationInfoList.byml") ||
                isEqualString(name, "BgmStageInfoList.byml")) {
                continue;
            }

            ByamlIter userIter(static_cast<const u8*>(resource->getOtherFile(fileName, nullptr)));
            fileName.removeSuffix(".byml");
            ByamlIter listIter;
            if (userIter.tryGetIterByKey(&listIter, "UserInfoList")) {
                mUserInfoList = BgmUserInfo::create(listIter);
                return;
            }

            if (!userInfoList->pushBack(BgmUserInfo::createInfo(userIter, fileName))) {
                break;
            }
        }

        shakerSortInfoArray<BgmUserInfo>(userInfoList, BgmUserInfo::compareInfo);
    }

    mUserInfoList = userInfoList;
}
}  // namespace al
