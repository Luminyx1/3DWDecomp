#pragma once

#include <basis/seadTypes.h>

#include "Library/Scene/ISceneObj.hpp"

namespace al {
class IUseSceneObjHolder;
class LayoutInitInfo;
}  // namespace al

class WindowMessage;

/** @brief Priority of a guide message; a higher one replaces a lower one already shown. */
enum GuideMessagePriority : s32 {};

/** @brief Scene object (SceneObjID_GuideGameWindow) showing the in-game guide messages. */
class GuideGameWindow : public al::ISceneObj {
public:
    GuideGameWindow(const al::LayoutInitInfo& rInfo, bool isSingleMode);

    bool isWaitConfirm() const;
    void startHide(const void* pUser);
    void endHide(const void* pUser);
    bool isWindowMessageActive() const;

    /** @brief Sets the lowest priority a guide message needs to be shown. */
    void setPriorityLimit(GuideMessagePriority priority) { mPriorityLimit = priority; }

    /** @brief Gets the message layout. @return The active message layout. */
    WindowMessage* getWindowMessage() const { return mWindowMessage; }

private:
    u8 _8[8];
    WindowMessage* mWindowMessage;
    u8 _18[0x24 - 0x18];
    GuideMessagePriority mPriorityLimit;  // 0x24
};

static_assert(sizeof(GuideGameWindow) == 0x28);

namespace rc {
GuideGameWindow* getGuideGameWindow(const al::IUseSceneObjHolder* pHolder);
void forceEndGuideGameWindow(const al::IUseSceneObjHolder* pHolder);
void appearGuideGameWindow(const al::IUseSceneObjHolder* pHolder, const char* pCategory,
                           const char* pLabel, s32 frame, f32 delay);
void appearGuideGameWindowWithConfirm(const al::IUseSceneObjHolder* pHolder, const char* pMessage,
                                      bool isConfirm);
bool appearGuideGameWindowWithPriority(const al::IUseSceneObjHolder* pHolder,
                                       const char* pCategory, const char* pMessage,
                                       GuideMessagePriority priority, s32 frame, f32 delay);
void forceDisappearGuideGameWindow(const al::IUseSceneObjHolder* pHolder);
void disappearGuideGameWindow(const al::IUseSceneObjHolder* pHolder);
bool isCurrentGuideGameWindowUser(const al::IUseSceneObjHolder* pHolder);
void unHideGuideGameWindow(const al::IUseSceneObjHolder* pHolder);
void hideGuideGameWindow(const al::IUseSceneObjHolder* pHolder);
bool isGuideGameWindowWaitConfirm(const al::IUseSceneObjHolder* pHolder);
void appearCutsceneGuideGameWindow(const al::IUseSceneObjHolder* pHolder, const char* pLabel);
bool isGuideGameWindowActive(const al::IUseSceneObjHolder* pHolder);
void disableGuideGameWindowPriority(const al::IUseSceneObjHolder* pHolder);
void enableGuideGameWindowPriority(const al::IUseSceneObjHolder* pHolder);
}  // namespace rc
