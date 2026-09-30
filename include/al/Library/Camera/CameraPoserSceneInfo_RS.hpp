#pragma once

#include <basis/seadTypes.h>

namespace al {
class AreaObjDirector;
class AudioDirector;
class CameraInputHolder;
class CameraRailHolder_RS;
class CameraRequestParamHolder;
class CameraTargetCollideInfoHolder;
class CameraTargetHolder;
class CollisionDirector;
struct CameraFlagCtrl;
struct SnapShotCameraSceneInfo;

struct CameraPoserSceneInfo_RS {
    CameraPoserSceneInfo_RS();

    void init(AreaObjDirector* pAreaObjDirector, CollisionDirector* pCollisionDirector,
              const AudioDirector* pAudioDirector);
    void registerCameraRailHolder(CameraRailHolder_RS* pRailHolder);

    f32 sceneFovyDegree = 45.0f;
    AreaObjDirector* areaObjDirector = nullptr;
    CollisionDirector* collisionDirector = nullptr;
    const AudioDirector* audioDirector = nullptr;
    CameraInputHolder* inputHolder = nullptr;
    CameraTargetHolder* targetHolder = nullptr;
    CameraFlagCtrl* flagCtrl = nullptr;
    CameraRequestParamHolder* requestParamHolder = nullptr;
    CameraTargetCollideInfoHolder* targetCollideInfoHolder = nullptr;
    SnapShotCameraSceneInfo* snapShotCameraSceneInfo = nullptr;
    CameraRailHolder_RS** railHolders = nullptr;
    s32 railHolderNum = 0;
};

static_assert(sizeof(CameraPoserSceneInfo_RS) == 0x60);

}  // namespace al
