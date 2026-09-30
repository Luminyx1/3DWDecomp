#pragma once

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class Sky : public LiveActor {
public:
    Sky(const char* pName);

    void init(const ActorInitInfo& rInfo) override;
    void control() override;
};
}  // namespace al
