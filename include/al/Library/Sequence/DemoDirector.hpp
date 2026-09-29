#pragma once

namespace al {
class LiveActor;

class DemoDirector {
public:
    bool isActiveDemo() const;
    bool tryRequestStartDemo(const LiveActor* pActor, const char* pDemoName);
    void requestEndDemo(const LiveActor* pActor, const char* pDemoName);
    void addDemoActor(LiveActor* pActor);
};
}  // namespace al
