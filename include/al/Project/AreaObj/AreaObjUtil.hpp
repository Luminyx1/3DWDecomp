#pragma once

#include <math/seadVector.h>

namespace al {
class ActorInitInfo;
class AreaObj;
class AreaObjGroup;
class IUseAreaObj;
class LiveActor;

bool tryGetAreaObjStringArg(const char**, const AreaObj*, const char*);

bool tryGetAreaObjArg(bool*, const AreaObj*, const char*);

void registerAreaHostMtx(const LiveActor*, const ActorInitInfo&);

bool isInAreaObj(const IUseAreaObj*, const char*, const sead::Vector3f&);

AreaObjGroup* tryFindAreaObjGroup(const IUseAreaObj*, const char*);

AreaObj* tryGetAreaObj(AreaObjGroup*, const sead::Vector3f&);

bool isInAreaPos(const AreaObj*, const sead::Vector3f&);
}  // namespace al
