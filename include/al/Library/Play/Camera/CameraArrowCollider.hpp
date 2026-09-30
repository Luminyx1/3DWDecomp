#pragma once

#include <gfx/seadCamera.h>
#include <math/seadVector.h>

#include "Library/Nerve/NerveExecutor.hpp"
#include "Project/Collision/IUseCollision.hpp"

namespace al {
class CollisionParts;

class CameraArrowCollider : public NerveExecutor, public IUseCollision {
public:
    CameraArrowCollider(CollisionDirector* pDirector);

    CollisionDirector* getCollisionDirector() const override;

    void start();
    void update(const sead::Vector3f& rPos, const sead::Vector3f& rAt, const sead::Vector3f& rUp);
    void pushBackCollisionParts(CollisionParts* pParts);
    void makeLookAtCamera(sead::LookAtCamera* pCamera) const;
    f32 getPushLength() const;
    void exeKeep();
    void exeShrink();
    bool isShrink() const;

    void setIsInvalidThroughPassCollision(bool isInvalid) { _48c = isInvalid; }

private:
    char _18[0x674];
    bool _48c;
};

}  // namespace al
