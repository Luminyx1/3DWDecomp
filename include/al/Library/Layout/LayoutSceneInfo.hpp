#pragma once

namespace eui {
class FontMgr;
}

namespace al {
class CameraDirector;
class PadRumbleDirector;
class SceneCameraInfo;
class SceneObjHolder;
class MessageSystem;
class GamePadSystem;

class LayoutSceneInfo {
public:
    LayoutSceneInfo();

    eui::FontMgr* getFontMgr() const { return mFontMgr; }
    CameraDirector* getCameraDirector() const { return mCameraDirector; }
    SceneCameraInfo* getSceneCameraInfo() const { return mSceneCameraInfo; }
    PadRumbleDirector* getPadRumbleDirector() const { return mPadRumbleDirector; }
    SceneObjHolder* getSceneObjHolder() const { return mSceneObjHolder; }
    const MessageSystem* getMessageSystem() const { return mMessageSystem; }
    const GamePadSystem* getGamePadSystem() const { return mGamePadSystem; }

protected:
    eui::FontMgr* mFontMgr = nullptr;
    CameraDirector* mCameraDirector = nullptr;
    SceneCameraInfo* mSceneCameraInfo = nullptr;
    PadRumbleDirector* mPadRumbleDirector = nullptr;
    SceneObjHolder* mSceneObjHolder = nullptr;
    const MessageSystem* mMessageSystem = nullptr;
    const GamePadSystem* mGamePadSystem = nullptr;
};
}  // namespace al
