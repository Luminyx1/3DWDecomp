#pragma once

#include <basis/seadTypes.h>

namespace al {
class IUseSceneObjHolder;
}  // namespace al

/** @brief Priority of a guide message; a higher one replaces a lower one already shown. */
enum GuideMessagePriority : s32 {};

namespace rc {
void appearGuideGameWindowWithConfirm(const al::IUseSceneObjHolder* pHolder, const char* pMessage,
                                      bool isConfirm);
bool appearGuideGameWindowWithPriority(const al::IUseSceneObjHolder* pHolder,
                                       const char* pCategory, const char* pMessage,
                                       GuideMessagePriority priority, s32 frame, f32 delay);
void disappearGuideGameWindow(const al::IUseSceneObjHolder* pHolder);
bool isCurrentGuideGameWindowUser(const al::IUseSceneObjHolder* pHolder);
void unHideGuideGameWindow(const al::IUseSceneObjHolder* pHolder);
bool isGuideGameWindowWaitConfirm(const al::IUseSceneObjHolder* pHolder);
bool isGuideGameWindowActive(const al::IUseSceneObjHolder* pHolder);
}  // namespace rc
