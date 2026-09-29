#pragma once

#include "Project/AreaObj/AreaShape.hpp"

namespace al {
class AreaShapeCube : public AreaShape {
public:
    enum OriginType { Center = 0, Base = 1 };

    AreaShapeCube(OriginType);

    virtual bool isInVolume(const sead::Vector3f&) const;
    virtual void calcNearPoint(sead::Vector3f*, const sead::Vector3f&) const;
    virtual bool calcNearestEdgePoint(sead::Vector3f*, const sead::Vector3f&) const;
    virtual bool checkArrowCollision(sead::Vector3f*, sead::Vector3f*, const sead::Vector3f&,
                                     const sead::Vector3f&) const;
    virtual bool calcLocalBoundingBox(sead::BoundBox3f*) const;
    virtual bool calcWorldBoundingBox(sead::BoundBox3f*) const;

    bool isInLocalVolume(const sead::Vector3f&) const;

    f32 calcBottom() const { return mOriginType == Base ? 0.0f : -500.0f; }

    f32 calcTop() const { return mOriginType == Base ? 1000.0f : 500.0f; }

    OriginType mOriginType;  // _1C
};
}  // namespace al
