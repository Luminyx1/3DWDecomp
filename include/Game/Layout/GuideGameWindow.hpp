#pragma once

namespace al {
class IUseSceneObjHolder;
}  // namespace al

namespace rc {
void appearGuideGameWindowWithConfirm(const al::IUseSceneObjHolder* pHolder, const char* pMessage,
                                      bool isConfirm);
void unHideGuideGameWindow(const al::IUseSceneObjHolder* pHolder);
bool isGuideGameWindowWaitConfirm(const al::IUseSceneObjHolder* pHolder);
}  // namespace rc
