#include <nn/ui2d/ui2d_AnimatorEx.h>
namespace nn::ui2d {
AnimatorEx::AnimatorEx() : mLayout(nullptr), mTag(nullptr), mExFlags(0) {}
AnimatorEx::~AnimatorEx() = default;
// resource supplies the animation tag; layout owns this animator; enabled controls activation.
void AnimatorEx::SetupBasic(const AnimResource& resource, LayoutEx* layout, bool enabled) {
    mLayout = layout; mTag = resource.mTag; SetEnabled(enabled);
}

// resource and layout identify the animation; pane is the target; enabled controls activation.
void AnimatorEx::SetupWithPane(const AnimResource& resource, LayoutEx* layout, Pane* pane, bool enabled) {
    BindPane(pane, false); SetupBasic(resource, layout, enabled);
}

// resource and layout identify the animation; group supplies targets; enabled controls activation.
void AnimatorEx::SetupWithGroup(const AnimResource& resource, LayoutEx* layout, Group* group, bool enabled) {
    BindGroup(group); SetupBasic(resource, layout, enabled);
}

// frame is the starting frame, type selects playback behavior, and speed is the frame step.
void AnimatorEx::PlayFromFrame(float frame, PlayType type, float speed) { mFrame = frame; PlayFromCurrent(type, speed); }
// other supplies the frame, playback type, and speed to synchronize with.
void AnimatorEx::Synchronize(const AnimatorEx& other) {
    if (other.mSpeed != 0) PlayFromFrame(other.mFrame, other.mPlayType, other.mSpeed);
    else StopAt(other.mFrame);
}

const char* AnimatorEx::GetTagName() const { return mTag ? reinterpret_cast<const char*>(mTag) + mTag->nameOffset : nullptr; }
void AnimatorEx::Unbind() {}
// resource supplies the tag metadata associated with this animator.
void AnimatorEx::SetupAnimationResource(const AnimResource& resource) { mTag = resource.mTag; }
}
