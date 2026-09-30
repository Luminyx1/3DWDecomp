#include "Library/Bgm/BgmKeeper.hpp"

#include "Library/Audio/System/AudioSystemInfo.hpp"
#include "Library/Bgm/BgmDataBase.hpp"
#include "Project/Bgm/BgmInfo.hpp"

namespace {
/**
 * Searches the index of a BGM user by name.
 * @param pList Sorted BGM user list.
 * @param pKey BGM user name.
 * @return Index, or -1 if not found.
 */
s32 searchUserInfoIndex(const sead::PtrArray<al::BgmUserInfo>* pList, const char* pKey) {
    if (pList->size() == 0) {
        return -1;
    }

    s32 lo = 0;
    s32 hi = pList->size() - 1;

    while (lo < hi) {
        s32 mid = (lo + hi) / 2;
        s32 result = al::BgmUserInfo::compareInfoByKey(pList->unsafeAt(mid), pKey);

        if (result == 0) {
            return mid;
        }

        if (result < 0) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }

    if (al::BgmUserInfo::compareInfoByKey(pList->unsafeAt(lo), pKey) == 0) {
        return lo;
    }

    return -1;
}
}  // namespace

namespace al {
/**
 * Constructs a BGM keeper for a BGM user.
 * @param pInfo Audio system information.
 * @param pDirector BGM director.
 * @param pUserName BGM user name, or nullptr.
 */
BgmKeeper::BgmKeeper(AudioSystemInfo* pInfo, BgmDirector* pDirector, const char* pUserName)
    : mBgmDirector(pDirector), mUserInfo(nullptr) {
    if (pUserName == nullptr) {
        return;
    }

    const sead::PtrArray<BgmUserInfo>* userInfoList = pInfo->mBgmDataBase->mUserInfoList;
    s32 index = searchUserInfoIndex(userInfoList, pUserName);
    mUserInfo = index >= 0 ? userInfoList->unsafeAt(index) : nullptr;
}

/**
 * Does nothing.
 */
void BgmKeeper::update() {}

/**
 * Gets the BGM user name.
 * @return BGM user name, or nullptr.
 */
const char* BgmKeeper::getUserName() const {
    if (mUserInfo == nullptr) {
        return nullptr;
    }

    return mUserInfo->mName;
}
}  // namespace al
