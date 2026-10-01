#include "Project/OceanWave/OceanWaveUserInfo.hpp"

#include "Library/LiveActor/LiveActor.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Library/Yaml/ByamlUtil.hpp"
#include "Project/OceanWave/OceanWaveDirector.hpp"
#include "Project/OceanWave/OceanWaveKeeper.hpp"

namespace al {
/**
 * Creates ocean wave information from BYAML data.
 * @param rIter BYAML data
 * @return created information, or nullptr if the data has no name
 */
OceanWaveInfo* OceanWaveInfo::createInfo(const ByamlIter& rIter) {
    OceanWaveInfo* info = new OceanWaveInfo;

    if (!rIter.tryGetStringByKey(&info->mName, "Name")) {
        return nullptr;
    }

    rIter.tryGetStringByKey(&info->mJointName, "JointName");
    tryGetByamlV3f(&info->mPosOffset, rIter, "PosOffset");
    rIter.tryGetFloatByKey(&info->mSize, "Size");
    rIter.tryGetFloatByKey(&info->mSpeed, "Speed");
    rIter.tryGetFloatByKey(&info->mTime, "Time");
    rIter.tryGetFloatByKey(&info->mAmp, "Amp");
    rIter.tryGetFloatByKey(&info->mLen, "Len");
    return info;
}

/**
 * Compares two ocean wave information by name.
 * @param pA first information
 * @param pB second information
 * @return comparison result
 */
s32 OceanWaveInfo::compareInfo(const OceanWaveInfo* pA, const OceanWaveInfo* pB) {
    return strcmp(pA->mName, pB->mName);
}

/**
 * Creates ocean wave play information of an action from BYAML data.
 * @param rIter BYAML data
 * @return created information
 */
OceanWavePlayInfoInAction* OceanWavePlayInfoInAction::createInfo(const ByamlIter& rIter) {
    OceanWavePlayInfoInAction* info = new OceanWavePlayInfoInAction;
    rIter.tryGetStringByKey(&info->mName, "Name");

    if (!rIter.tryGetFloatByKey(&info->mStartFrame, "StartFrame")) {
        info->mStartFrame = 0.0f;
    }

    if (!rIter.tryGetFloatByKey(&info->mEndFrame, "EndFrame")) {
        info->mEndFrame = 0.0f;
    }

    return info;
}

/**
 * Compares two ocean wave play information of an action by start frame.
 * @param pA first information
 * @param pB second information
 * @return comparison result
 */
s32 OceanWavePlayInfoInAction::compareInfo(const OceanWavePlayInfoInAction* pA,
                                           const OceanWavePlayInfoInAction* pB) {
    if (pA->mStartFrame < pB->mStartFrame) {
        return -1;
    }

    return pA->mStartFrame > pB->mStartFrame;
}

/**
 * Constructs empty ocean wave play information of an action.
 */
OceanWavePlayInfoInAction::OceanWavePlayInfoInAction() = default;

/**
 * Copies ocean wave play information of an action.
 * @param rOther information to copy
 */
OceanWavePlayInfoInAction::OceanWavePlayInfoInAction(const OceanWavePlayInfoInAction& rOther)
    : mName(rOther.mName), mStartFrame(rOther.mStartFrame), mEndFrame(rOther.mEndFrame) {}

/**
 * Copies ocean wave play information of an action.
 * @param rOther information to copy
 * @return this information
 */
OceanWavePlayInfoInAction& OceanWavePlayInfoInAction::operator=(
    const OceanWavePlayInfoInAction& rOther) {
    mName = rOther.mName;
    mStartFrame = rOther.mStartFrame;
    mEndFrame = rOther.mEndFrame;
    return *this;
}

/**
 * Creates ocean wave action information from BYAML data.
 * @param rIter BYAML data
 * @return created information
 */
OceanWaveActionInfo* OceanWaveActionInfo::createInfo(const ByamlIter& rIter) {
    OceanWaveActionInfo* info = new OceanWaveActionInfo;
    rIter.tryGetStringByKey(&info->mName, "Name");
    ByamlIter iter;
    rIter.tryGetIterByKey(&iter, "PlayInfoInActionList");
    info->mPlayInfoList = createInfoList<OceanWavePlayInfoInAction>(iter);
    return info;
}

/**
 * Compares two ocean wave action information by name.
 * @param pA first information
 * @param pB second information
 * @return comparison result
 */
s32 OceanWaveActionInfo::compareInfo(const OceanWaveActionInfo* pA, const OceanWaveActionInfo* pB) {
    return strcmp(pA->mName, pB->mName);
}

/**
 * Constructs empty ocean wave action information.
 */
OceanWaveActionInfo::OceanWaveActionInfo() : mName(nullptr), mPlayInfoList(nullptr) {}

/**
 * Copies ocean wave action information including its play information.
 * @param rOther information to copy
 */
OceanWaveActionInfo::OceanWaveActionInfo(const OceanWaveActionInfo& rOther)
    : mName(rOther.mName), mPlayInfoList(nullptr) {
    if (rOther.mPlayInfoList == nullptr) {
        mPlayInfoList = nullptr;
        return;
    }

    s32 num = rOther.mPlayInfoList->getInfoNum();
    AudioInfoList<OceanWavePlayInfoInAction>* list = new AudioInfoList<OceanWavePlayInfoInAction>;
    list->mNext = nullptr;
    list->mInfos = new sead::PtrArray<OceanWavePlayInfoInAction>;
    list->mInfos->allocBuffer(num + 1, nullptr);
    mPlayInfoList = list;

    for (s32 i = 0; i < num; i++) {
        OceanWavePlayInfoInAction* info =
            rOther.mPlayInfoList != nullptr ? rOther.mPlayInfoList->getInfo(i) : nullptr;

        if (info != nullptr) {
            OceanWavePlayInfoInAction* copy = new OceanWavePlayInfoInAction(*info);
            mPlayInfoList->mInfos->pushBack(copy);
        }
    }
}

/**
 * Copies ocean wave action information.
 * @param rOther information to copy
 * @return this information
 */
OceanWaveActionInfo& OceanWaveActionInfo::operator=(const OceanWaveActionInfo& rOther) {
    mName = rOther.mName;

    if (rOther.mPlayInfoList != nullptr && mPlayInfoList != nullptr) {
        *mPlayInfoList = *rOther.mPlayInfoList;
    }

    return *this;
}

/**
 * Starts the ocean waves of a play information.
 * @param pActor actor that owns the waves
 * @param pName play information name
 */
void startOceanWave(LiveActor* pActor, const char* pName) {
    OceanWaveKeeper* keeper = pActor->mOceanWaveKeeper;

    if (keeper == nullptr || keeper->getUserInfo() == nullptr) {
        return;
    }

    OceanWaveUserInfo* userInfo = keeper->getUserInfo();
    OceanWaveDirector* director = keeper->getDirector();

    if (director == nullptr || pName == nullptr || userInfo->mPlayInfoList == nullptr) {
        return;
    }

    OceanWavePlayInfo* playInfo = userInfo->mPlayInfoList->tryFindInfo(pName);

    if (playInfo == nullptr || playInfo->mOceanWaveInfoList == nullptr) {
        return;
    }

    s32 num = playInfo->mOceanWaveInfoList->getInfoNum();

    for (s32 i = 0; i < num; i++) {
        OceanWaveInfo* info = playInfo->mOceanWaveInfoList != nullptr
                                  ? playInfo->mOceanWaveInfoList->getInfo(i)
                                  : nullptr;
        director->createWave(pActor, info);
    }
}
}  // namespace al
