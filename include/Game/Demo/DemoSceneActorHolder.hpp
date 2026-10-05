#pragma once

class DemoSceneActorHolder {
public:
    void startAction(int index, bool loop);
    bool isActionEndCamera(int frames) const;
};
