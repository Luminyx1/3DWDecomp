#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
class ActorInitInfo;
class AreaObj;
class AreaObjGroup;
class IUseAreaObj;
class LiveActor;

bool tryGetAreaObjStringArg(const char**, const AreaObj*, const char*);

bool tryGetAreaObjArg(s32*, const AreaObj*, const char*);
bool tryGetAreaObjArg(f32*, const AreaObj*, const char*);
bool tryGetAreaObjArg(bool*, const AreaObj*, const char*);

bool tryIsInAreaPos(const AreaObj*, const sead::Vector3f&);

AreaObjGroup* createLinkAreaGroup(LiveActor*, const ActorInitInfo&, const char*, const char*, const char*);

void registerAreaHostMtx(const LiveActor*, const ActorInitInfo&);

bool isInAreaObj(const IUseAreaObj*, const char*, const sead::Vector3f&);

AreaObjGroup* tryFindAreaObjGroup(const IUseAreaObj*, const char*);

AreaObj* tryGetAreaObj(AreaObjGroup*, const sead::Vector3f&);

bool isInAreaPos(const AreaObj*, const sead::Vector3f&);
}  // namespace al
