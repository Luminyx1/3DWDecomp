#pragma once

#include "NPC/IUseNekoModeActor.hpp"

class Neko;
class NekoParent;

/**
 * @brief Mode actor of a regular cat that wanders around and can be carried to its parent.
 * @note Only what reconstructed code needs is declared so far.
 */
class NekoNormal : public IUseNekoModeActor {
public:
    NekoNormal(Neko* pHost);

    bool isEnableGoal() const;
    bool isAtGoal() const;
    bool canCollect() const;
    void setNekoParent(const NekoParent* pParent);

    void init(const al::ActorInitInfo& rInfo, neko::ColorType colorType,
              NpcTargetFinder* pTargetFinder) override;
    void startAttach(const NekoAttachReason& rReason) override;
    void startKill(bool isDeleteParticle) override;
    void startDisasterAnticipation(bool isEmitEffect) override;
    bool startDisasterDemo() override;
    bool startSeekTarget(const neko::Target* pTarget, bool isForce) override;
    void startAppearLinks() override;
    bool isHold() const override;
    bool isRide() const override;
    s32 getUID() const override;
    neko::ColorType getNekoType() const override;
    f32 getChaseRange() const override;
    const neko::Param* getParam() const override;
    void onStartHide() override;
    bool acceptTarget(const al::LiveActor* pTarget,
                      const npc::NpcFindTargetType& rType) const override;

    Neko* getHost() const { return mHost; }

private:
    Neko* mHost;  // 0x158
    u8 _160[0x2a8 - 0x160];
};

static_assert(sizeof(NekoNormal) == 0x2a8);
