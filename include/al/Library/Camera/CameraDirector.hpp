#pragma once

#include <basis/seadTypes.h>

namespace sead {
class LookAtCamera;
class PerspectiveProjection;
class Viewport;
}  // namespace sead

namespace al {
class ActorInitInfo;
class Resource;
class AreaObjDirector;
class CollisionDirector;
class PlayerHolder;
class SceneCameraInfo;

class CameraDirector {
public:
    CameraDirector(s32 viewNum, AreaObjDirector* pAreaObjDirector,
                   CollisionDirector* pCollisionDirector);

    void init(const PlayerHolder* pPlayerHolder);
    void initCameraCreator(s32 resourceNum, bool isStageOneResource);
    void setCameraResource(const Resource* pResource, s32 index);
    void initAudioKeeper(ActorInitInfo& rInfo);
    void setCameraAspect(const sead::Viewport* pMainViewport, const sead::Viewport* pSubViewport);
    void setStageName(const char* pStageName);
    void update(bool isPaused);

    SceneCameraInfo* getSceneCameraInfo() const { return mSceneCameraInfo; }
    sead::LookAtCamera* getLookAtCamera() const { return mLookAtCamera; }
    sead::PerspectiveProjection* getProjection() const { return mProjection; }

    u8 _0[0x8];
    sead::LookAtCamera* mLookAtCamera;
    u8 _10[0x8];
    sead::PerspectiveProjection* mProjection;
    u8 _20[0x8];
    SceneCameraInfo* mSceneCameraInfo;
    u8 _30[0x1f0 - 0x30];
};
}  // namespace al
