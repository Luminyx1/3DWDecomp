#pragma once

#include <basis/seadTypes.h>

namespace al {
class AudioSystemDebug {
public:
    AudioSystemDebug();

    void update();
    void draw() const;

private:
    f32 mDrawPosX = -191.0f;
    f32 mDrawPosY = 92.0f;
    s32 mState = 0;
    s32 mTimer = 0;
    bool mIsDraw = false;
};
}  // namespace al
