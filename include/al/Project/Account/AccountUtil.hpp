#pragma once

#include <nn/account.h>

namespace al {
bool tryInitAccount();
nn::account::Uid getUid();
const nn::account::UserHandle& getUserHandle();
}  // namespace al
