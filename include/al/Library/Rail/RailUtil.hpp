#pragma once

#include <math/seadVector.h>

namespace al {
class LiveActor;
class RailKeeper;

void setSyncRailToStart(LiveActor*);
void setSyncRailToNearestPos(LiveActor*, const sead::Vector3f&);
void setSyncRailToNearestPos(LiveActor*);
void setRailClippingInfo(sead::Vector3f*, LiveActor*, f32, f32);
void setRailClippingInfo(sead::Vector3f*, LiveActor*, const RailKeeper*, f32, f32);
}  // namespace al
