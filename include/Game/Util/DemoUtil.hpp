#pragma once

#include <basis/seadTypes.h>

namespace al {
class LiveActor;
}  // namespace al

namespace alSeFunction {
enum DemoType : s32;
}  // namespace alSeFunction

namespace rc {
void setUpdateItemsInDemo(const al::LiveActor* pActor);
bool requestStartDemoPlayerCutscene(const al::LiveActor* pActor);
void requestEndDemoPlayerCutscene(const al::LiveActor* pActor);
bool isAnyActiveDemo(const al::LiveActor* pActor);
void addDemoActor(al::LiveActor* pActor);
void removeDemoActor(al::LiveActor* pActor);
void setDemoAudioType(const al::LiveActor* pActor, alSeFunction::DemoType type);
void setDemoFullEffectUpdate(const al::LiveActor* pActor, bool isFull);
}  // namespace rc
