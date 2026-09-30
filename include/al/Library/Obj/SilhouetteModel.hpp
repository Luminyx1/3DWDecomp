#pragma once

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class ActorInitInfo;

class SilhouetteModel : public LiveActor {
public:
    SilhouetteModel(LiveActor* pParent, const ActorInitInfo& rInfo, const char* pExecutorDrawName);

    void movement() override;

    LiveActor* mParent;
};

static_assert(sizeof(SilhouetteModel) == 0x150);
}  // namespace al
