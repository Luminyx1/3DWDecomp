#pragma once

#include "Library/LiveActor/LiveActor.hpp"

class TeresaConveyorTeresaBig : public al::LiveActor {
public:
    TeresaConveyorTeresaBig(const char* pName);
    /** @brief Releases the conveyor's large Boo actor. */
    ~TeresaConveyorTeresaBig() override = default;
    void init(const al::ActorInitInfo& rInfo) override;
    void exeWait();
};
