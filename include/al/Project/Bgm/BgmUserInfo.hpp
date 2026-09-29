#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>
#include <prim/seadSafeString.h>

namespace al {
class BgmActionInfoList;
class ByamlIter;

class BgmUserInfo {
public:
    static sead::PtrArray<BgmUserInfo>* create(const ByamlIter& rIter);
    static BgmUserInfo* createInfo(const ByamlIter& rIter);
    static BgmUserInfo* createInfo(const ByamlIter& rIter, const sead::SafeString& rName);
    static s32 compareInfo(const BgmUserInfo* pInfoA, const BgmUserInfo* pInfoB);
    static s32 compareInfoByKey(const BgmUserInfo* pInfo, const char* pKey);

    BgmUserInfo();

    const char* mName = nullptr;                          // _0
    BgmActionInfoList* mActionInfoList = nullptr;         // _8
};

class BgmUserInfoList {
public:
    static s32 compareInfoByName(const BgmUserInfo* pInfoA, const BgmUserInfo* pInfoB);
};
}  // namespace al
