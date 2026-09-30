#include "Project/Account/AccountUtil.hpp"

#include <mc/seadCoreInfo.h>
#include <thread/seadThreadUtil.h>

namespace al {
namespace {
struct AccountThreadParam {
    AccountThreadParam() {
        mCoreMask = sead::CoreIdMask(sead::CoreId::cMain);
        mPriority = sead::ThreadUtil::ConvertPrioritySeadToPlatform(17);
    }

    void* _0 = nullptr;
    u64 _8 = 0x10000;
    sead::CoreIdMask mCoreMask;
    s32 mPriority;
    u32 _18 = 0x80;
    u64 _20 = 0x8000;
    u32 _28 = 0x80;
    bool _2c = true;
};

struct AccountInfo {
    nn::account::UserHandle mUserHandle;
    nn::account::Nickname mNickname;
    AccountThreadParam mThreadParam;
    nn::account::Uid mUid = {};
};

AccountInfo sAccountInfo;
}  // namespace

/**
 * Selects the user account used by the game if none is selected yet.
 * @return True if an account is available.
 */
bool tryInitAccount() {
    if (sAccountInfo.mUid.IsValid()) {
        return true;
    }

    if (!nn::account::TryOpenPreselectedUser(&sAccountInfo.mUserHandle)) {
        s32 count = 0;
        nn::account::ListAllUsers(&count, &sAccountInfo.mUid, 1);
        return nn::account::OpenUser(&sAccountInfo.mUserHandle, sAccountInfo.mUid).IsSuccess();
    }

    nn::account::GetUserId(&sAccountInfo.mUid, sAccountInfo.mUserHandle);

    if (!sAccountInfo.mUid.IsValid()) {
        s32 count = 0;
        nn::account::Uid uids[8];
        nn::account::ListAllUsers(&count, uids, 8);
        sAccountInfo.mUid = uids[0];
        nn::account::GetNickname(&sAccountInfo.mNickname, sAccountInfo.mUid);
    }

    return true;
}

/**
 * Gets the selected user's id.
 * @return The user id.
 */
nn::account::Uid getUid() {
    return sAccountInfo.mUid;
}

/**
 * Gets the selected user's handle.
 * @return The user handle.
 */
nn::account::UserHandle* getUserHandle() {
    return &sAccountInfo.mUserHandle;
}
}  // namespace al
