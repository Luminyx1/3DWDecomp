#pragma once

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class ActorInitInfo;

/**
 * Base of actors whose model is drawn as a mirror; feeds the mirror view-projection matrix of the
 * actor's mirror camera to its materials.
 */
class MirrorActorBase : public LiveActor {
public:
    MirrorActorBase(const char* pName);

    void initByArg(const ActorInitInfo& rInfo);
    void initAfterPlacement() override;
    void control() override;

    bool isHideAtSelfMirror() const { return mIsHideAtSelfMirror; }

private:
    bool mIsHideAtSelfMirror = true;
};

static_assert(sizeof(MirrorActorBase) == 0x148);

}  // namespace al
