#pragma once

namespace al {
class ActorInitInfo;
}

/** @brief Holds the items Tentack's tentacles can carry. */
class TentackAttachItemHolder {
public:
    TentackAttachItemHolder();
    void init(const al::ActorInitInfo& rInfo);

private:
    unsigned char mUnknown[0x10];
};
static_assert(sizeof(TentackAttachItemHolder) == 0x10);
