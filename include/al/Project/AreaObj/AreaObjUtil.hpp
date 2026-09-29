#pragma once

#include <basis/seadTypes.h>

namespace al {
class ActorInitInfo;
class AreaObj;
class AreaObjGroup;
class LiveActor;

bool tryGetAreaObjStringArg(const char**, const AreaObj*, const char*);

bool tryGetAreaObjArg(s32*, const AreaObj*, const char*);
bool tryGetAreaObjArg(f32*, const AreaObj*, const char*);
bool tryGetAreaObjArg(bool*, const AreaObj*, const char*);

AreaObjGroup* createLinkAreaGroup(LiveActor*, const ActorInitInfo&, const char*, const char*, const char*);

void registerAreaHostMtx(const LiveActor*, const ActorInitInfo&);
}  // namespace al
