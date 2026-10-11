#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class GhostPlayer : public al::LiveActor {
public:
    void tryStartFromObj(const char* pId);
    void stopAndHide(bool hide);
    void restart();

private:
    // Playback data and model state are implemented in GhostPlayer's own TU.
    void* mPlaybackData;
    unsigned char mPlaybackState[0xb0];
};
static_assert(sizeof(GhostPlayer) == 0x200);
