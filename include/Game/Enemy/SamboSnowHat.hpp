#pragma once

#include "Library/LiveActor/LiveActor.hpp"

class SamboSnowHat : public al::LiveActor {
public:
    SamboSnowHat(const char* pName);
    /** @brief Releases the snow Pokey's hat actor. */
    ~SamboSnowHat() override = default;
    void init(const al::ActorInitInfo& rInfo) override;
    void startBlow();
    void exeWait();
    void exeBlow();
};
