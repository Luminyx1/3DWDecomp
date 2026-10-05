#pragma once
#include "Project/Block/BlockRailParts.hpp"
#include <math/seadQuat.h>
class IUseRouteDokan;
class RouteDokanEntrance : public al::BlockRailParts {
public:
    RouteDokanEntrance(const char*, const char*, const char*);
    void setInitQT(const sead::Quatf&, const sead::Vector3f&);
    void setHost(IUseRouteDokan*);
private:
    u8 mUnreconstructed[0x38];
};
static_assert(sizeof(RouteDokanEntrance) == 0x1b8);
