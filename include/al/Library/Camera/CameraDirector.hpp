#pragma once

#include <basis/seadTypes.h>

namespace al {
class AreaObjDirector;
class CollisionDirector;
class PlayerHolder;
class SceneCameraInfo;

class CameraDirector {
public:
    CameraDirector(s32 viewNum, AreaObjDirector* pAreaObjDirector,
                   CollisionDirector* pCollisionDirector);

    void init(const PlayerHolder* pPlayerHolder);

    SceneCameraInfo* getSceneCameraInfo() const { return mSceneCameraInfo; }

    u8 _0[0x28];
    SceneCameraInfo* mSceneCameraInfo;
    u8 _30[0x1f0 - 0x30];
};
}  // namespace al
