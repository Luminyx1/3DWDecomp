#pragma once

namespace al {
class ActorInitInfo;
class LiveActor;

namespace EffectObjFunction {
void initActorEffectObj(LiveActor*, const ActorInitInfo&);
void initActorEffectObj(LiveActor*, const ActorInitInfo&, const char*);
}  // namespace EffectObjFunction
}  // namespace al
