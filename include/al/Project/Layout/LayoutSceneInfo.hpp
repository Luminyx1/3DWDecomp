#pragma once

namespace eui {
class FontMgr;
}

namespace al {
class CameraDirector;
class PadRumbleDirector;
class SceneObjHolder;
class MessageSystem;
class GamePadSystem;

class LayoutSceneInfo {
public:
    LayoutSceneInfo();

    eui::FontMgr* mFontMgr = nullptr;                // _0
    CameraDirector* mCameraDirector = nullptr;       // _8
    PadRumbleDirector* mPadRumbleDirector = nullptr;  // _10
    SceneObjHolder* mSceneObjHolder = nullptr;       // _18
    const MessageSystem* mMessageSystem = nullptr;   // _20
    const GamePadSystem* mGamePadSystem = nullptr;   // _28
    void* _30 = nullptr;
};
}  // namespace al
