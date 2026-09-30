#pragma once

#include "Project/Collision/CollisionPartsFilterBase.hpp"

namespace al {

class CameraCollisionPartsFilter : public CollisionPartsFilterBase {
public:
    CameraCollisionPartsFilter();

    bool isInvalidParts(const CollisionParts& rParts) const override;
};

}  // namespace al
