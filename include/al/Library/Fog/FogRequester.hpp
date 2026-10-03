#pragma once

#include <basis/seadTypes.h>

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class ActorInitInfo;
class FogParam;
class MtxConnector;
class YFogParam;

/**
 * @brief Placed actor that requests a distance fog and / or a height fog while it is alive.
 */
class FogRequester : public LiveActor {
public:
    FogRequester(const char* pName);

    void init(const ActorInitInfo& rInfo) override;
    void appearBySwitch();
    void initAfterPlacement() override;
    void control() override;

private:
    FogParam* mFogParam;
    s32 mFogPriority = -1;
    s32 mFogInterpStep = 1;
    YFogParam* mYFogParam;
    s32 mYFogPriority = -1;
    s32 mYFogInterpStep = 1;
    bool mIsYFogPlacementRelative = false;
    bool mIsYFogConnectAndMove = false;
    MtxConnector* mMtxConnector = nullptr;
};

static_assert(sizeof(FogRequester) == 0x178);

}  // namespace al
