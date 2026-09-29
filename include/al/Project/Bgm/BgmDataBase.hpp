#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>

namespace al {
class BgmUserInfo;

class BgmDataBase {
public:
    BgmDataBase();

    void* _0;
    void* _8;
    void* _10;
    void* _18;
    void* _20;
    sead::PtrArray<BgmUserInfo>* mUserInfoList;  // _28
};
}  // namespace al
