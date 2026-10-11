#pragma once

#include <container/seadPtrArray.h>
#include <math/seadVector.h>

namespace al { struct PlacementInfo; }
class RabbitRoutePoint;

class RabbitRoute {
public:
    explicit RabbitRoute(const al::PlacementInfo& rInfo);
    void calcNearestPosOnRoute(sead::Vector3f* pPosition, const sead::Vector3f& rPosition) const;
    void calcFarthestPosOnRoute(sead::Vector3f* pPosition, const sead::Vector3f& rPosition) const;

private:
    sead::PtrArray<RabbitRoutePoint> mPoints;
    RabbitRoutePoint* mStartPoint;
};
