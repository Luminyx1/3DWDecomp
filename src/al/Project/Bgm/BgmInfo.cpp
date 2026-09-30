#include "Project/Bgm/BgmInfo.hpp"

#include "Library/Yaml/ByamlIter.hpp"
#include "Project/Base/StringOpUtil.hpp"

namespace al {
/**
 * Creates BGM play information of an action from BYAML data.
 * @param rIter BYAML data.
 * @return Created information, or nullptr if it has no name.
 */
BgmPlayInfoInAction* BgmPlayInfoInAction::createInfo(const ByamlIter& rIter) {
    BgmPlayInfoInAction* info = new BgmPlayInfoInAction();

    if (!rIter.tryGetStringByKey(&info->mName, "Name")) {
        return nullptr;
    }

    if (!rIter.tryGetStringByKey(&info->mPlayTypeName, "PlayTypeName")) {
        info->mPlayTypeName = "REQ_UNKNOWN";
    }

    if (!rIter.tryGetFloatByKey(&info->mTriggerFrame, "TriggerFrame")) {
        info->mTriggerFrame = 0.0f;
    }

    if (!rIter.tryGetIntByKey(&info->mFadeFrameNum, "FadeFrameNum")) {
        info->mFadeFrameNum = -1;
    }

    if (!rIter.tryGetIntByKey(&info->mStartDelayFrameNum, "StartDelayFrameNum")) {
        info->mStartDelayFrameNum = 0;
    }

    if (!rIter.tryGetIntByKey(&info->mFadeOutFrameNumForCurBgm, "FadeOutFrameNumForCurBgm")) {
        info->mFadeOutFrameNumForCurBgm = -1;
    }

    if (!rIter.tryGetBoolByKey(&info->mIsTriggerActionEnd, "IsTriggerActionEnd")) {
        info->mIsTriggerActionEnd = false;
    }

    if (!rIter.tryGetBoolByKey(&info->mIsStopActionEnd, "IsStopActionEnd")) {
        info->mIsStopActionEnd = false;
    }

    if (!rIter.tryGetBoolByKey(&info->mIsPlayBySequenceBgm, "IsPlayBySequenceBgm")) {
        info->mIsPlayBySequenceBgm = false;
    }

    if (!rIter.tryGetBoolByKey(&info->mIsDisableLineAutoStop, "DisableLineAutoStop")) {
        info->mIsDisableLineAutoStop = false;
    }

    return info;
}

/**
 * Creates BGM action information from BYAML data.
 * @param rIter BYAML data.
 * @return Created information.
 */
BgmActionInfo* BgmActionInfo::createInfo(const ByamlIter& rIter) {
    BgmActionInfo* info = new BgmActionInfo();
    rIter.tryGetStringByKey(&info->mName, "Name");
    ByamlIter playIter;
    rIter.tryGetIterByKey(&playIter, "PlayInfoInActionList");
    info->mPlayInfoList = createInfoList<BgmPlayInfoInAction>(playIter);
    return info;
}

/**
 * Compares two BGM play information of an action by trigger frame.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 BgmPlayInfoInAction::compareInfo(const BgmPlayInfoInAction* pA, const BgmPlayInfoInAction* pB) {
    if (pA->mTriggerFrame < pB->mTriggerFrame) {
        return -1;
    }

    return pA->mTriggerFrame > pB->mTriggerFrame;
}

/**
 * Constructs empty BGM play information of an action.
 */
BgmPlayInfoInAction::BgmPlayInfoInAction()
    : mName(nullptr), mPlayTypeName(nullptr), mTriggerFrame(0.0f), mFadeFrameNum(0), mStartDelayFrameNum(0),
      mFadeOutFrameNumForCurBgm(0), mIsTriggerActionEnd(false), mIsStopActionEnd(false),
      mIsPlayBySequenceBgm(false), mIsDisableLineAutoStop(false) {}

/**
 * Copies BGM play information of an action except for its trigger frame.
 * @param rOther Information to copy.
 */
BgmPlayInfoInAction::BgmPlayInfoInAction(const BgmPlayInfoInAction& rOther)
    : mName(rOther.mName), mPlayTypeName(rOther.mPlayTypeName), mTriggerFrame(0.0f),
      mFadeFrameNum(rOther.mFadeFrameNum), mStartDelayFrameNum(rOther.mStartDelayFrameNum),
      mFadeOutFrameNumForCurBgm(rOther.mFadeOutFrameNumForCurBgm), mIsTriggerActionEnd(rOther.mIsTriggerActionEnd),
      mIsStopActionEnd(rOther.mIsStopActionEnd), mIsPlayBySequenceBgm(rOther.mIsPlayBySequenceBgm),
      mIsDisableLineAutoStop(rOther.mIsDisableLineAutoStop) {}

/**
 * Copies BGM play information of an action.
 * @param rOther Information to copy.
 * @return This information.
 */
BgmPlayInfoInAction& BgmPlayInfoInAction::operator=(const BgmPlayInfoInAction& rOther) {
    mName = rOther.mName;
    mTriggerFrame = rOther.mTriggerFrame;
    mPlayTypeName = rOther.mPlayTypeName;
    mFadeFrameNum = rOther.mFadeFrameNum;
    mStartDelayFrameNum = rOther.mStartDelayFrameNum;
    mFadeOutFrameNumForCurBgm = rOther.mFadeOutFrameNumForCurBgm;
    mIsTriggerActionEnd = rOther.mIsTriggerActionEnd;
    mIsStopActionEnd = rOther.mIsStopActionEnd;
    mIsPlayBySequenceBgm = rOther.mIsPlayBySequenceBgm;
    mIsDisableLineAutoStop = rOther.mIsDisableLineAutoStop;
    return *this;
}

/**
 * Constructs empty BGM action information.
 */
BgmActionInfo::BgmActionInfo() : mName(nullptr), mPlayInfoList(nullptr) {}

/**
 * Allocates the play information list.
 * @param size Number of play information.
 */
void BgmActionInfo::allockBuffer(s32 size) {
    if (size < 1) {
        return;
    }

    AudioInfoList<BgmPlayInfoInAction>* list = new AudioInfoList<BgmPlayInfoInAction>;
    list->mNext = nullptr;
    list->mInfos = new sead::PtrArray<BgmPlayInfoInAction>;
    list->mInfos->allocBuffer(size + 1, nullptr);
    mPlayInfoList = list;
}

/**
 * Copies BGM action information including its play information.
 * @param rOther Information to copy.
 */
BgmActionInfo::BgmActionInfo(const BgmActionInfo& rOther) : mName(rOther.mName), mPlayInfoList(nullptr) {
    if (rOther.mPlayInfoList == nullptr) {
        mPlayInfoList = nullptr;
        return;
    }

    s32 num = rOther.mPlayInfoList->getInfoNum();

    if (num < 1) {
        return;
    }

    allockBuffer(num);

    for (s32 i = 0; i != num; i++) {
        BgmPlayInfoInAction* info =
            rOther.mPlayInfoList != nullptr ? rOther.mPlayInfoList->getInfo(i) : nullptr;
        if (info == nullptr) {
            break;
        }

        BgmPlayInfoInAction* copy = new BgmPlayInfoInAction(*info);
        mPlayInfoList->mInfos->pushBack(copy);
    }
}

/**
 * Copies BGM action information.
 * @param rOther Information to copy.
 * @return This information.
 */
BgmActionInfo& BgmActionInfo::operator=(const BgmActionInfo& rOther) {
    mName = rOther.mName;

    if (rOther.mPlayInfoList != nullptr && mPlayInfoList != nullptr) {
        *mPlayInfoList = *rOther.mPlayInfoList;
    }

    return *this;
}

/**
 * Compares two BGM action information by name.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 BgmActionInfo::compareInfo(const BgmActionInfo* pA, const BgmActionInfo* pB) {
    return strcmp(pA->mName, pB->mName);
}

/**
 * Compares BGM action information with a name.
 * @param pInfo Information.
 * @param pKey Name.
 * @return Comparison result.
 */
s32 BgmActionInfo::compareInfoByKey(const BgmActionInfo* pInfo, const char* pKey) {
    return strcmp(pInfo->mName, pKey);
}

/**
 * Creates a sorted BGM action information list from BYAML data.
 * @param rIter BYAML data.
 * @return Created list.
 */
sead::PtrArray<BgmActionInfo>* BgmActionInfoList::create(const ByamlIter& rIter) {
    s32 size = rIter.getSize();
    sead::PtrArray<BgmActionInfo>* list = new sead::PtrArray<BgmActionInfo>;

    if (size >= 1) {
        list->allocBuffer(size, nullptr);

        for (s32 i = 0; i < size; i++) {
            ByamlIter iter;
            rIter.tryGetIterByIndex(&iter, i);
            list->pushBack(BgmActionInfo::createInfo(iter));
        }

        shakerSortInfoArray<BgmActionInfo>(list, BgmActionInfo::compareInfo);
    }

    return list;
}

/**
 * Constructs empty BGM user information.
 */
BgmUserInfo::BgmUserInfo() : mName(nullptr), mActionInfoList(nullptr) {}

/**
 * Creates a sorted BGM user information list from BYAML data.
 * @param rIter BYAML data.
 * @return Created list.
 */
sead::PtrArray<BgmUserInfo>* BgmUserInfo::create(const ByamlIter& rIter) {
    s32 size = rIter.getSize();
    sead::PtrArray<BgmUserInfo>* list = new sead::PtrArray<BgmUserInfo>;
    list->allocBuffer(size, nullptr);

    for (s32 i = 0; i < size; i++) {
        ByamlIter iter;

        if (!rIter.tryGetIterByIndex(&iter, i)) {
            continue;
        }

        list->pushBack(BgmUserInfo::createInfo(iter));
    }

    shakerSortInfoArray<BgmUserInfo>(list, BgmUserInfo::compareInfo);
    return list;
}

/**
 * Creates BGM user information from BYAML data.
 * @param rIter BYAML data.
 * @return Created information.
 */
BgmUserInfo* BgmUserInfo::createInfo(const ByamlIter& rIter) {
    BgmUserInfo* info = new BgmUserInfo();

    if (!rIter.tryGetStringByKey(&info->mName, "Name")) {
        return info;
    }

    ByamlIter actionIter;
    rIter.tryGetIterByKey(&actionIter, "ActionInfoList");
    info->mActionInfoList = BgmActionInfoList::create(actionIter);
    return info;
}

/**
 * Compares two BGM user information by name.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 BgmUserInfo::compareInfo(const BgmUserInfo* pA, const BgmUserInfo* pB) {
    return strcmp(pA->mName, pB->mName);
}

/**
 * Creates BGM user information from BYAML data with the given name.
 * @param rIter BYAML data.
 * @param rName User name.
 * @return Created information.
 */
BgmUserInfo* BgmUserInfo::createInfo(const ByamlIter& rIter, const sead::SafeString& rName) {
    BgmUserInfo* info = new BgmUserInfo();
    info->mName = createStringIfInStack(rName.cstr());
    ByamlIter actionIter;

    if (rIter.tryGetIterByKey(&actionIter, "ActionInfoList")) {
        info->mActionInfoList = BgmActionInfoList::create(actionIter);
    }

    return info;
}

/**
 * Compares two BGM user information by name.
 * @param pA First information.
 * @param pB Second information.
 * @return Comparison result.
 */
s32 BgmUserInfoList::compareInfoByName(const BgmUserInfo* pA, const BgmUserInfo* pB) {
    return strcmp(pA->mName, pB->mName);
}

/**
 * Compares BGM user information with a name.
 * @param pInfo Information.
 * @param pKey Name.
 * @return Comparison result.
 */
s32 BgmUserInfo::compareInfoByKey(const BgmUserInfo* pInfo, const char* pKey) {
    return strcmp(pInfo->mName, pKey);
}

}  // namespace al
