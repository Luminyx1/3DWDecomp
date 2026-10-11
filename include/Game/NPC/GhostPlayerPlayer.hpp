#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class GhostPlayerLoaderBase;
class GhostPlayerPlayer : public al::LiveActor {
public:
    explicit GhostPlayerPlayer(const char* pName);
    ~GhostPlayerPlayer() override;
    void init(const al::ActorInitInfo& rInfo) override;
    void update();
    bool tryStartPlay();
    bool tryEndPlay();

private:
    GhostPlayerLoaderBase* mLoader;
    // Two fixed pointer arrays, each with capacity three.
    unsigned char mPlaybackArrays[0x50];
};
static_assert(sizeof(GhostPlayerPlayer) == 0x1a0);
