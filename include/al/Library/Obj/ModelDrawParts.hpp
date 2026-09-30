#pragma once

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class ActorInitInfo;

class ModelDrawParts : public LiveActor {
public:
    ModelDrawParts(const char* pName, const LiveActor* pParent, const ActorInitInfo& rInfo,
                   const char* pExecutorDrawName);

    const LiveActor* mParent;
};

static_assert(sizeof(ModelDrawParts) == 0x150);
}  // namespace al
