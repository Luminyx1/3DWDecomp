#pragma once
#include "Library/LiveActor/LiveActor.hpp"
namespace neko {
enum class ColorType : int;
class Target;
} // namespace neko
class NekoAttachReason;
class NpcTargetFinder;
// Primary virtual-interface prefix needed by target filtering; no instances are allocated here.
class NekoNormal : public al::LiveActor {
public:
    virtual void init(const al::ActorInitInfo&, neko::ColorType, NpcTargetFinder*);
    virtual void startAttach(const NekoAttachReason&);
    virtual void startKill(bool);
    virtual void startDisasterAnticipation(bool);
    virtual void startDisasterDemo();
    virtual void startSeekTarget(const neko::Target*, bool);
    virtual void startAppearLinks();
    virtual bool isHold() const;
    bool isEnableGoal() const;
};
