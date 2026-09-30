#include "Library/Se/Info/SeAudioInfo.hpp"

#include "Library/Yaml/ByamlIter.hpp"

namespace al {
/**
 * Creates SE play information of an action from BYAML data.
 * @param rIter BYAML data.
 * @return Created information.
 */
SePlayInfoInAction* SePlayInfoInAction::createInfo(const ByamlIter& rIter) {
    SePlayInfoInAction* info = new SePlayInfoInAction;
    rIter.tryGetStringByKey(&info->mName, "Name");
    if (!rIter.tryGetFloatByKey(&info->mStartFrame, "StartFrame")) {
        info->mStartFrame = 0.0f;
    }
    if (!rIter.tryGetFloatByKey(&info->mEndFrame, "EndFrame")) {
        info->mEndFrame = 0.0f;
    }
    if (!rIter.tryGetBoolByKey(&info->mIsOneTime, "IsOneTime")) {
        info->mIsOneTime = false;
    }
    return info;
}

/**
 * Creates SE action information from BYAML data.
 * @param rIter BYAML data.
 * @return Created information.
 */
SeActionInfo* SeActionInfo::createInfo(const ByamlIter& rIter) {
    SeActionInfo* info = new SeActionInfo;
    rIter.tryGetStringByKey(&info->mName, "Name");
    if (!rIter.tryGetBoolByKey(&info->mIsStopPlayingSe, "IsStopPlayingSe")) {
        info->mIsStopPlayingSe = false;
    }
    ByamlIter playIter;
    rIter.tryGetIterByKey(&playIter, "PlayInfoInActionList");
    info->mPlayInfoList = createInfoList<SePlayInfoInAction>(playIter);
    return info;
}

/**
 * Compares two SE play information of an action by start frame.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 SePlayInfoInAction::compareInfo(const SePlayInfoInAction* pA, const SePlayInfoInAction* pB) {
    if (pA->mStartFrame < pB->mStartFrame) {
        return -1;
    }
    return pA->mStartFrame > pB->mStartFrame;
}

/**
 * Compares two SE action information by name.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 SeActionInfo::compareInfo(const SeActionInfo* pA, const SeActionInfo* pB) {
    return strcmp(pA->mName, pB->mName);
}

/**
 * Constructs empty SE play information of an action.
 */
SePlayInfoInAction::SePlayInfoInAction() = default;

/**
 * Copies SE play information of an action.
 * @param rOther Information to copy.
 */
SePlayInfoInAction::SePlayInfoInAction(const SePlayInfoInAction& rOther)
    : mName(rOther.mName), mStartFrame(rOther.mStartFrame), mEndFrame(rOther.mEndFrame),
      mIsOneTime(rOther.mIsOneTime) {}

/**
 * Copies SE play information of an action.
 * @param rOther Information to copy.
 * @return This information.
 */
SePlayInfoInAction& SePlayInfoInAction::operator=(const SePlayInfoInAction& rOther) {
    mName = rOther.mName;
    mStartFrame = rOther.mStartFrame;
    mEndFrame = rOther.mEndFrame;
    mIsOneTime = rOther.mIsOneTime;
    return *this;
}

/**
 * Constructs empty SE action information.
 */
SeActionInfo::SeActionInfo() : mName(nullptr), mIsStopPlayingSe(false), mPlayInfoList(nullptr) {}

/**
 * Copies SE action information including its play information.
 * @param rOther Information to copy.
 */
SeActionInfo::SeActionInfo(const SeActionInfo& rOther)
    : mName(rOther.mName), mIsStopPlayingSe(rOther.mIsStopPlayingSe) {
    if (rOther.mPlayInfoList == nullptr) {
        mPlayInfoList = nullptr;
        return;
    }
    s32 num = rOther.mPlayInfoList->getInfoNum();
    AudioInfoList<SePlayInfoInAction>* list = new AudioInfoList<SePlayInfoInAction>;
    list->mNext = nullptr;
    list->mInfos = new sead::PtrArray<SePlayInfoInAction>;
    list->mInfos->allocBuffer(num + 1, nullptr);
    mPlayInfoList = list;
    for (s32 i = 0; i < num; i++) {
        SePlayInfoInAction* info = rOther.mPlayInfoList != nullptr ? rOther.mPlayInfoList->getInfo(i) : nullptr;
        if (info == nullptr) {
            break;
        }
        SePlayInfoInAction* copy = new SePlayInfoInAction(*info);
        mPlayInfoList->mInfos->pushBack(copy);
    }
}

/**
 * Copies SE action information.
 * @param rOther Information to copy.
 * @return This information.
 */
SeActionInfo& SeActionInfo::operator=(const SeActionInfo& rOther) {
    mName = rOther.mName;
    mIsStopPlayingSe = rOther.mIsStopPlayingSe;
    if (rOther.mPlayInfoList != nullptr && mPlayInfoList != nullptr) {
        *mPlayInfoList = *rOther.mPlayInfoList;
    }
    return *this;
}

}  // namespace al
