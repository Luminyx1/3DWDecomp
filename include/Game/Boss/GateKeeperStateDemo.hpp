#pragma once

#include <basis/seadTypes.h>

#include "Library/Nerve/NerveStateBase.hpp"

namespace al {
class ActorInitInfo;
class CameraInfo;
class CameraTicket;
class LiveActor;
}  // namespace al

/** @brief Name and settings of a gate keeper boss's opening demo. */
class GateKeeperStateDemoParam {
public:
    GateKeeperStateDemoParam(const char* pActionName, s32 unk);

    const char* mActionName;
    u8 mUnknown08[0x20 - 0x08];
};

static_assert(sizeof(GateKeeperStateDemoParam) == 0x20);

/** @brief Opening demo shared by the gate keeper bosses (Punpun, Bunbun, ...). */
class GateKeeperStateDemo : public al::ActorStateBase {
public:
    GateKeeperStateDemo(al::LiveActor* pHost, const al::ActorInitInfo& rInfo,
                        const GateKeeperStateDemoParam* pParam, al::CameraInfo* pCamera);
    GateKeeperStateDemo(al::LiveActor* pHost, const al::ActorInitInfo& rInfo,
                        const GateKeeperStateDemoParam* pParam, al::CameraTicket* pCamera);

    void appear() override;
    void kill() override;
    bool isDemoFirstStep() const;
    void exeInitOnGround();
    void exeDemoRequest();
    void exeDemo();

    /**
     * @brief Sets whether this demo drives the player (off when another boss owns the demo).
     * @param isControlPlayer Whether the demo controls the player.
     */
    void setIsControlPlayer(bool isControlPlayer) { mIsControlPlayer = isControlPlayer; }

private:
    u8 mUnknown20[0xcd - 0x20];
    bool mIsControlPlayer;  // 0xcd
    u8 mUnknownCE[0xd8 - 0xce];
};

static_assert(sizeof(GateKeeperStateDemo) == 0xd8);
