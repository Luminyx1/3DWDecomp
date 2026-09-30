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

    BgmDirector* getBgmDirector() const { return mBgmDirector; }

private:
    BgmDirector* mBgmDirector;
    const BgmUserInfo* mUserInfo;
};
static_assert(sizeof(BgmKeeper) == 0x10);
}  // namespace al
