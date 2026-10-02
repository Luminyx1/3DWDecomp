#pragma once

#include <basis/seadTypes.h>

namespace al {
class PadRumbleKeeper {
public:
    PadRumbleKeeper(s32 port);

    s32 getPort() const { return mPort; }

private:
    s32 mPort;
};
}  // namespace al
