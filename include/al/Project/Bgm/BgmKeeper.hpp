#pragma once

#include <basis/seadTypes.h>

namespace al {
class AudioSystemInfo;
class BgmDirector;
class BgmUserInfo;

class BgmKeeper {
public:
    BgmKeeper(AudioSystemInfo* pInfo, BgmDirector* pDirector, const char* pUserName);

    void update();
    const char* getUserName() const;

    BgmDirector* mBgmDirector;  // _0
    const BgmUserInfo* mUserInfo = nullptr;  // _8
};
}  // namespace al
