#pragma once

#include <basis/seadTypes.h>

#include "Project/AreaObj/AreaObjGroup.hpp"

namespace al {
class ActorInitInfo;
class AreaObj;
class LiveActor;

class SwitchKeepOnAreaGroup {
public:
    SwitchKeepOnAreaGroup(AreaObjGroup* pGroup);

    void update(const sead::Vector3f* pPositions, s32 num, bool isDisasterMode);
    void update(const sead::Vector3f& rPos);

    AreaObjGroup* mGroup;
    AreaObj** mKeepOnAreas = nullptr;
    s32 mCount = 0;
    s32 mKeepOnCount = 0;
};
}  // namespace al
