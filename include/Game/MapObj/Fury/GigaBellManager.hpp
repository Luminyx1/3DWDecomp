#pragma once
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/Scene/ISceneObj.hpp"
class GigaBell;
class GigaBellManager : public al::LiveActor, public al::ISceneObj {
public:
    explicit GigaBellManager(const char*);
    GigaBell* getGigaBellClosestTo(sead::Vector3f);
private:
    u8 mUnreconstructed[0x208];
};
static_assert(sizeof(GigaBellManager) == 0x358);
