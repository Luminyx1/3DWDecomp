#pragma once

namespace al {
class LiveActor;

void stopScene(const LiveActor* pActor, int stopFrames, int delayFrames, bool flag8, bool flag9);
bool isStopScene(const LiveActor* pActor);
void requestCaptureScreenCover(const LiveActor* pActor, int coverFrames);
void requestCaptureScreenSceneCover(const LiveActor* pActor);
void resetRequestCaptureScreenSceneCover(const LiveActor* pActor);
bool requestStartDemo(const LiveActor* pActor, const char* pDemoName);
void requestEndDemo(const LiveActor* pActor, const char* pDemoName);
void addDemoActor(LiveActor* pActor);
void setDisasterMode(LiveActor* pActor, bool isDisaster);
bool isDisasterMode(LiveActor* pActor);
void stopAllPadRumble(LiveActor* pActor);
bool isSingleMode(const LiveActor*);
}  // namespace al
