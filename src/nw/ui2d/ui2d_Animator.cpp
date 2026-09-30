#include <nn/ui2d/ui2d_Animator.h>
#include <nn/ui2d/ui2d_AnimResource.h>
#include <nn/ui2d/ui2d_Group.h>
namespace nn::ui2d {
Animator::~Animator() = default;
Animator::Animator() : mSpeed(0), mPlayType(PlayType_Once), mFlags(0) {}

// type selects once, looping, or round-trip playback; speed is frames per step.
void Animator::Play(PlayType type, float speed) {
    if (!(speed >= -float(GetFrameSize()) && speed <= float(GetFrameSize()))) {
        if (!IsWaitData() || type != PlayType_Once) { IsWaitData(); return; }
    }

    mSpeed = speed;
    mPlayType = type;
    mFlags &= ~0xfu;

    if (speed >= 0.0f) mFrame = 0.0f;
    else mFrame = float(GetFrameSize());
}

// speed is the playback rate; the animation resource determines looping.
void Animator::PlayAuto(float speed) { Play(IsLoopData() ? PlayType_Loop : PlayType_Once, speed); }
// type and speed select playback without resetting the current frame.
void Animator::PlayFromCurrent(PlayType type, float speed) {
    if (!(speed >= -float(GetFrameSize()) && speed <= float(GetFrameSize()))) {
        if (!IsWaitData() || type != PlayType_Once) { IsWaitData(); return; }
    }

    mSpeed = speed;
    mPlayType = type;
    mFlags &= ~0xfu;
}

// frame is the position to retain after stopping playback.
void Animator::StopAt(float frame) { mSpeed = 0; mFrame = frame; mFlags &= ~0xfu; }
void Animator::StopAtCurrentFrame() { mSpeed = 0; mFlags &= ~0xfu; }
void Animator::StopAtStartFrame() { mSpeed = 0; mFrame = 0; mFlags &= ~0xfu; }
void Animator::StopAtEndFrame() { mSpeed = 0; mFrame = float(GetFrameSize()); mFlags &= ~0xfu; }
// step scales the playback speed; endpoint crossings update completion flags.
void Animator::UpdateFrame(float step) {
    mFlags &= ~0xfu;

    if (mSpeed == 0.0f) return;
    float frame = mFrame + mSpeed * step;

    if (mSpeed > 0.0f) {
        if (frame >= float(GetFrameSize())) {
            switch (mPlayType) {
            case PlayType_Once: frame = float(GetFrameSize()); mSpeed = 0; mFlags |= 1; break;
            case PlayType_Loop: frame -= float(GetFrameSize()); mFlags |= 2; break;
            case PlayType_RoundTrip:
                frame = float(GetFrameSize()) + (float(GetFrameSize()) - frame);
                mSpeed = -mSpeed; mFlags |= 2; break;
            }
        }
    } else {
        if (frame <= 0.0f) {
            switch (mPlayType) {
            case PlayType_Once: frame = 0; mSpeed = 0; mFlags |= 1; break;
            case PlayType_Loop: frame += float(GetFrameSize()); mFlags |= 4; break;
            case PlayType_RoundTrip: frame = -frame; mSpeed = -mSpeed; mFlags |= 4; break;
            }
        }
    }

    mFrame = frame;
}

// resource is unused by the base animator; derived animators may retain its tag.
void Animator::SetupAnimationResource(const AnimResource& resource) {}
const char* Animator::GetTagName() const { return nullptr; }
// pane is the animation target; enabled selects whether its animation is applied.
void PaneAnimator::Setup(Pane* pane, bool enabled) {
    BindPane(pane, false);
    mPane = pane;
    SetEnabled(enabled);
}

void PaneAnimator::Unbind() { UnbindPane(mPane); }
// group is the animation target; enabled selects whether its animation is applied.
void GroupAnimator::Setup(Group* group, bool enabled) {
    BindGroup(group);
    mGroup = group;
    SetEnabled(enabled);
}

void GroupAnimator::Unbind() { UnbindGroup(mGroup); }
// resource supplies group names, groups resolves them, index selects one entry,
// and enabled sets the animator's initial application state.
void GroupAnimator::Setup(const AnimResource& resource, GroupContainer* groups, int index, bool enabled) {
    SetupAnimationResource(resource);

    if (index < resource.GetGroupCount()) Setup(groups->FindGroupByName(resource.GetGroupArray()[index].name), enabled);
}

// resource supplies the group list, groups resolves its names, storage holds
// the resulting pointers, and enabled selects the initial application state.
void GroupArrayAnimator::Setup(const AnimResource& resource, GroupContainer* groups, Group** storage, bool enabled) {
    SetupAnimationResource(resource);
    mGroupCount = resource.GetGroupCount();
    mGroups = storage;

    for (int i = 0; i < mGroupCount; ++i) {
        mGroups[i] = groups->FindGroupByName(resource.GetGroupArray()[i].name);
        BindGroup(mGroups[i]);
    }

    SetEnabled(enabled);
}

void GroupArrayAnimator::Unbind() {
    for (int i = 0; i < mGroupCount; ++i) UnbindGroup(mGroups[i]);
}
}
