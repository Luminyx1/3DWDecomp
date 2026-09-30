#pragma once

#include <basis/seadTypes.h>

namespace eui {
class Animator;
}

namespace nn::ui2d {
class Layout;
class Pane;
}

namespace al {
class LayoutPaneGroup {
public:
    LayoutPaneGroup(const char* pGroupName);

    void startAnim(const char* pAnimName);
    eui::Animator* getAnimator(const char* pAnimName) const;
    void setAnimFrame(f32 frame);
    void setAnimFrameRate(f32 frameRate);
    f32 getAnimFrame() const;
    f32 getAnimFrameMax() const;
    f32 getAnimFrameMax(const char* pAnimName) const;
    f32 getAnimFrameRate() const;
    bool isAnimExist(const char* pAnimName) const;
    eui::Animator* tryGetAnimator(const char* pAnimName) const;
    bool isAnimEnd() const;
    bool isAnimOneTime() const;
    bool isAnimOneTime(const char* pAnimName) const;
    bool isAnimPlaying() const;
    const char* getPlayingAnimName() const;
    void pushAnimName(const char* pAnimName);
    void createAnimator(nn::ui2d::Layout* pLayout);
    void animate(bool isRecursive);

    const char* getGroupName() const { return mGroupName; }

private:
    const char* mGroupName;
    u8 _8[0x20];
};

void requestCaptureRecursive(nn::ui2d::Pane* pPane);
}  // namespace al
