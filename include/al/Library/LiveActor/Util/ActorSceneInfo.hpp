#pragma once

#include <basis/seadTypes.h>

namespace al {
class SceneStopCtrl;
class ScreenCoverCtrl;
class DemoDirector;
class CameraDirector_RS;
class PadRumbleDirector;
class SceneObjHolder;
class ClippingDirectorBase;
class PlayerHolder;
class SceneCameraInfo;
class CollisionDirector;
class ItemDirectorBase;
class ShadowDirector;
class AreaObjDirector;
class GraphicsSystemInfo;

struct ActorSceneInfo {
    ActorSceneInfo();

    SceneObjHolder* sceneObjHolder = nullptr;
    ClippingDirectorBase* clippingDirectorBase = nullptr;
    CollisionDirector* collisionDirector = nullptr;
    PlayerHolder* playerHolder = nullptr;
    SceneCameraInfo* sceneCameraInfo = nullptr;
    SceneStopCtrl* sceneStopCtrl = nullptr;
    ScreenCoverCtrl* screenCoverCtrl = nullptr;
    ItemDirectorBase* itemDirectorBase = nullptr;
    DemoDirector* demoDirector = nullptr;
    AreaObjDirector* areaObjDirector = nullptr;
    ShadowDirector* shadowDirector = nullptr;
    PadRumbleDirector* padRumbleDirector = nullptr;
    CameraDirector_RS* cameraDirector = nullptr;
    bool isSingleMode = false;
    void* _70 = nullptr;
    GraphicsSystemInfo* graphicsSystemInfo = nullptr;
};
}  // namespace al
