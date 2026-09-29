#include "Project/Bgm/BgmKeeper.hpp"

#include "Project/Audio/AudioSystemInfo.hpp"
#include "Project/Bgm/BgmDataBase.hpp"
#include "Project/Bgm/BgmUserInfo.hpp"

namespace al {
/**
 * @brief Constructs a BGM keeper for one audio user.
 * @param pInfo The audio system info holding the BGM database.
 * @param pDirector The BGM director the keeper sends its requests to.
 * @param pUserName The name of the user's entry in the BGM database, or nullptr for none.
 */
BgmKeeper::BgmKeeper(AudioSystemInfo* pInfo, BgmDirector* pDirector, const char* pUserName)
    : mBgmDirector(pDirector) {
    if (pUserName == nullptr) {
        return;
    }

    const sead::PtrArray<BgmUserInfo>* userInfoList = pInfo->mBgmDataBase->mUserInfoList;
    s32 index = userInfoList->binarySearch(reinterpret_cast<const BgmUserInfo*>(pUserName),
                                           reinterpret_cast<s32 (*)(const BgmUserInfo*, const BgmUserInfo*)>(
                                               BgmUserInfo::compareInfoByKey));
    mUserInfo = index < 0 ? nullptr : userInfoList->unsafeAt(index);
}

/**
 * @brief Does nothing.
 */
void BgmKeeper::update() {}

/**
 * @brief Gets the name of the user this keeper belongs to.
 * @return The user name, or nullptr if the keeper has no user info.
 */
const char* BgmKeeper::getUserName() const {
    if (mUserInfo == nullptr) {
        return nullptr;
    }
    return mUserInfo->mName;
}
}  // namespace al
