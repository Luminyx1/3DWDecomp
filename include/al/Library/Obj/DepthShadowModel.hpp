#pragma once

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class ActorInitInfo;

class DepthShadowModel : public LiveActor {
public:
    DepthShadowModel(LiveActor* pParent, const ActorInitInfo& rInfo, const char* pExecutorDrawName);

    LiveActor* mParent;
};

static_assert(sizeof(DepthShadowModel) == 0x150);
}  // namespace al
