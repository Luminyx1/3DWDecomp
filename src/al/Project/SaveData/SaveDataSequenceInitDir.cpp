#include "Project/SaveData/SaveDataSequenceInitDir.hpp"

#include <cstring>
#include <nn/account.h>

#include "Project/Account/AccountUtil.hpp"

namespace al {
/**
 * Constructs the init dir sequence.
 * @param unk Unknown flag.
 */
SaveDataSequenceInitDir::SaveDataSequenceInitDir(u8 unk) : _18(unk) {}

/**
 * Sets up the buffer and clears it.
 * @param pBuffer Save data buffer.
 * @param bufferSize Buffer size.
 * @param version Save data version.
 */
void SaveDataSequenceInitDir::start(u8* pBuffer, u32 bufferSize, u32 version) {
    mBuffer = pBuffer;
    mBufferSize = bufferSize;
    mVersion = version;
    memset(pBuffer, 0, bufferSize);
}

/**
 * Makes sure a user is selected for the save data.
 * @param pFileName Save file name.
 * @return Always 0.
 */
s32 SaveDataSequenceInitDir::threadFunc(const char* pFileName) {
    tryInitAccount();
    nn::account::Uid uid = getUid();

    if (uid.IsValid()) {
        return 0;
    }

    nn::account::UserHandle handle;

    if (nn::account::TryOpenPreselectedUser(&handle)) {
        nn::Result result = nn::account::GetUserId(&uid, handle);
        nn::account::CloseUser(handle);

        if (result.IsSuccess()) {
            return 0;
        }
    }

    s32 count;
    nn::account::ListAllUsers(&count, &uid, 1);
    return 0;
}
}  // namespace al
