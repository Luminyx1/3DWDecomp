#pragma once

#include <basis/seadTypes.h>

namespace al {
class LiveActor;
}  // namespace al

namespace alSeFunction {
enum DemoType : s32;
}  // namespace alSeFunction

namespace rc {
void setImmediateSwitchFlag(const al::LiveActor*);
void setUpdateFreeze(const al::LiveActor*, bool);
void setOtherActiveDemo(al::LiveActor*, bool);
bool requestStartDemoIntro(al::LiveActor*, const char*, bool);
bool requestStartDemoCamera(al::LiveActor*, const char*);
bool isActiveSpecificDemo(const al::LiveActor*);
bool isActiveDemoCamera(const al::LiveActor*);
void requestEndDemoIntro(const al::LiveActor*);
void requestEndDemoCamera(const al::LiveActor*);
bool requestStartDemoInGameCutscene(const al::LiveActor* pActor);
void requestEndDemoInGameCutscene(const al::LiveActor* pActor);
void setUpdateItemsInDemo(const al::LiveActor* pActor);
bool requestStartDemoPlayerCutscene(const al::LiveActor* pActor);
void requestEndDemoPlayerCutscene(const al::LiveActor* pActor);
bool isAnyActiveDemo(const al::LiveActor* pActor);
void addDemoActor(al::LiveActor* pActor);
void removeDemoActor(al::LiveActor* pActor);
void setDemoAudioType(const al::LiveActor* pActor, alSeFunction::DemoType type);
void setDemoFullEffectUpdate(const al::LiveActor* pActor, bool isFull);
}  // namespace rc
