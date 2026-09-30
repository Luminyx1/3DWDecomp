#pragma once

#include "Project/AreaObj/AreaShape.hpp"

namespace al {
class AreaShapeCube : public AreaShape {
public:
    enum OriginType { Center = 0, Base = 1 };

    AreaShapeCube(OriginType originType);

    bool isInVolume(const sead::Vector3f& rPos) const override;
    void calcNearPoint(sead::Vector3f* pOut, const sead::Vector3f& rPos) const override;
    bool calcNearestEdgePoint(sead::Vector3f* pOut, const sead::Vector3f& rPos) const override;
    bool checkArrowCollision(sead::Vector3f* pHitPos, sead::Vector3f* pNormal,
                             const sead::Vector3f& rStart,
                             const sead::Vector3f& rEnd) const override;
    bool calcLocalBoundingBox(sead::BoundBox3f* pBox) const override;
    bool calcWorldBoundingBox(sead::BoundBox3f* pBox) const override;

    bool isInLocalVolume(const sead::Vector3f& rPos) const;

    f32 calcBottom() const { return mOriginType == Base ? 0.0f : -500.0f; }
    f32 calcTop() const { return mOriginType == Base ? 1000.0f : 500.0f; }

    OriginType mOriginType;
};

class AreaShapeCubeBase : public AreaShapeCube {
public:
    AreaShapeCubeBase() : AreaShapeCube(Base) {}
};

class AreaShapeCubeCenter : public AreaShapeCube {
public:
    AreaShapeCubeCenter() : AreaShapeCube(Center) {}
};
}  // namespace al
