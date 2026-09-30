#pragma once

#include "Project/AreaObj/AreaShape.hpp"

namespace al {
class AreaShapeSphere : public AreaShape {
public:
    AreaShapeSphere();

    bool isInVolume(const sead::Vector3f& rPos) const override;
    void calcNearPoint(sead::Vector3f* pOut, const sead::Vector3f& rPos) const override;
    bool calcNearestEdgePoint(sead::Vector3f* pOut, const sead::Vector3f& rPos) const override;
    bool checkArrowCollision(sead::Vector3f* pHitPos, sead::Vector3f* pNormal,
                             const sead::Vector3f& rStart,
                             const sead::Vector3f& rEnd) const override;
    bool calcLocalBoundingBox(sead::BoundBox3f* pBox) const override;
    bool calcWorldBoundingBox(sead::BoundBox3f* pBox) const override;
};

class AreaShapeCylinder : public AreaShape {
public:
    AreaShapeCylinder();

    bool isInVolume(const sead::Vector3f& rPos) const override;
    void calcNearPoint(sead::Vector3f* pOut, const sead::Vector3f& rPos) const override;
    bool calcNearestEdgePoint(sead::Vector3f* pOut, const sead::Vector3f& rPos) const override;
    bool checkArrowCollision(sead::Vector3f* pHitPos, sead::Vector3f* pNormal,
                             const sead::Vector3f& rStart,
                             const sead::Vector3f& rEnd) const override;
    bool calcLocalBoundingBox(sead::BoundBox3f* pBox) const override;
    bool calcWorldBoundingBox(sead::BoundBox3f* pBox) const override;
};

class AreaShapeOval : public AreaShape {
public:
    AreaShapeOval();

    bool isInVolume(const sead::Vector3f& rPos) const override;
    void calcNearPoint(sead::Vector3f* pOut, const sead::Vector3f& rPos) const override;
    bool calcNearestEdgePoint(sead::Vector3f* pOut, const sead::Vector3f& rPos) const override;
    bool checkArrowCollision(sead::Vector3f* pHitPos, sead::Vector3f* pNormal,
                             const sead::Vector3f& rStart,
                             const sead::Vector3f& rEnd) const override;
    bool calcLocalBoundingBox(sead::BoundBox3f* pBox) const override;
    bool calcWorldBoundingBox(sead::BoundBox3f* pBox) const override;
};

class AreaShapeOvalBase : public AreaShapeOval {
public:
    AreaShapeOvalBase() {}
};

class AreaShapeCylinderBase : public AreaShapeCylinder {
public:
    AreaShapeCylinderBase() {}
};
}  // namespace al
