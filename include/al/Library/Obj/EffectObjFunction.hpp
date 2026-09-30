#pragma once

namespace al {
class ActorInitInfo;
class LiveActor;

namespace EffectObjFunction {
void initActorEffectObj(LiveActor* pActor, const ActorInitInfo& rInfo);
void initActorEffectObj(LiveActor* pActor, const ActorInitInfo& rInfo, const char* pArchiveName);
}  // namespace EffectObjFunction
}  // namespace al
