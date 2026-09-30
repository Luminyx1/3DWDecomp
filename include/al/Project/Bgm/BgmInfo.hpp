#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>
#include <prim/seadSafeString.h>

#include "Project/Audio/AudioInfoList.hpp"

namespace al {
class ByamlIter;

class BgmPlayInfoInAction {
public:
    BgmPlayInfoInAction();
    BgmPlayInfoInAction(const BgmPlayInfoInAction& rOther);
    BgmPlayInfoInAction& operator=(const BgmPlayInfoInAction& rOther);

    static BgmPlayInfoInAction* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const BgmPlayInfoInAction* pA, const BgmPlayInfoInAction* pB);

    const char* mName;
    const char* mPlayTypeName;
    f32 mTriggerFrame;
    s32 mFadeFrameNum;
    s32 mStartDelayFrameNum;
    s32 mFadeOutFrameNumForCurBgm;
    bool mIsTriggerActionEnd;
    bool mIsStopActionEnd;
    bool mIsPlayBySequenceBgm;
    bool mIsDisableLineAutoStop;
};

static_assert(sizeof(BgmPlayInfoInAction) == 0x28);

class BgmActionInfo {
public:
    BgmActionInfo();
    BgmActionInfo(const BgmActionInfo& rOther);
    BgmActionInfo& operator=(const BgmActionInfo& rOther);

    static BgmActionInfo* createInfo(const ByamlIter& rIter);
    static s32 compareInfo(const BgmActionInfo* pA, const BgmActionInfo* pB);
    static s32 compareInfoByKey(const BgmActionInfo* pInfo, const char* pKey);

    void allockBuffer(s32 size);

    const char* mName;
    AudioInfoList<BgmPlayInfoInAction>* mPlayInfoList;
};

static_assert(sizeof(BgmActionInfo) == 0x10);

class BgmActionInfoList {
public:
    static sead::PtrArray<BgmActionInfo>* create(const ByamlIter& rIter);
};

class BgmUserInfo {
public:
    BgmUserInfo();

    static sead::PtrArray<BgmUserInfo>* create(const ByamlIter& rIter);
    static BgmUserInfo* createInfo(const ByamlIter& rIter);
    static BgmUserInfo* createInfo(const ByamlIter& rIter, const sead::SafeString& rName);
    static s32 compareInfo(const BgmUserInfo* pA, const BgmUserInfo* pB);
    static s32 compareInfoByKey(const BgmUserInfo* pInfo, const char* pKey);

    const char* mName;
    sead::PtrArray<BgmActionInfo>* mActionInfoList;
};

static_assert(sizeof(BgmUserInfo) == 0x10);

class BgmUserInfoList {
public:
    static s32 compareInfoByName(const BgmUserInfo* pA, const BgmUserInfo* pB);
};
}  // namespace al
