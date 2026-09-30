#include <eui/euiAnimator.h>
#include <eui/euiLayoutEx.h>
#include <eui/euiScreen.h>
#include <nn/ui2d/ui2d_AnimResource.h>
#include <nn/ui2d/ui2d_Group.h>
#include <random/seadGlobalRandom.h>

namespace eui {
// type selects endpoint behavior; step is signed playback speed, with negative values reversing playback.
void Animator::Play(PlayType type, float step) {
    if (!(-float(GetFrameSize()) <= step && step <= float(GetFrameSize()))) {
        if (!IsWaitData() || type != cPlayType_OneTime) { IsWaitData(); return; }
    }
    mPlayType = type;
    mStep = step;
    mFlags &= ~0xf;
    if (step >= 0) {
        if ((mFlags & 0x10) && GetFrameSize()) mFrame = step;
        else mFrame = 0;
    } else {
        if ((mFlags & 0x10) && GetFrameSize()) mFrame = GetFrameSize() + step;
        else mFrame = GetFrameSize();
    }
    if (type == cPlayType_OneTime && (mFlags & 0x20)) mLayout->mScreen->invokeSoundLink2AnimPlayEvent(this, "_play");
    SetEnabled(true);
    mLayout->mScreen->animatorOperationCallback(Screen::AnimatorOperationType::cPlay, this);
}
// type selects endpoint behavior; step resumes playback from the current pose in either direction.
void Animator::PlayFromCurrent(PlayType type, float step) {
    if (!(-float(GetFrameSize()) <= step && step <= float(GetFrameSize()))) {
        if (!IsWaitData() || type != cPlayType_OneTime) { IsWaitData(); return; }
    }
    mPlayType = type;
    mStep = step;
    mFlags &= ~0xf;
    if ((mFlags & 0x10) && GetFrameSize()) {
        if (step >= 0) {
            if (mFrame < step) mFrame = step;
        } else {
            const float lastFrame = GetFrameSize() + step;
            if (mFrame > lastFrame) mFrame = lastFrame;
        }
    }
    SetEnabled(true);
    mLayout->mScreen->animatorOperationCallback(Screen::AnimatorOperationType::cPlayFromCurrent, this);
}
// type and step control playback; the initial pose is chosen uniformly from integer resource frames.
void Animator::PlayRandom(PlayType type, float step) {
    const u16 lastFrame = GetFrameSize();
    auto* random = sead::GlobalRandom::instance();
    mFrame = random->getU32(u32(lastFrame) + 1);
    PlayFromCurrent(type, step);
}
// rResource supplies the animation name; pLayout owns playback; enabled selects its initial state.
void Animator::SetupBasic(const nn::ui2d::AnimResource& rResource, LayoutEx* pLayout, bool enabled) {
    mName = rResource.GetTagName();
    mLayout = pLayout;
    SetEnabled(enabled);
}
// rResource supplies animation metadata; pLayout owns playback; pPane is bound without recursion;
// enabled selects the initial animation state.
void Animator::SetupWithPane(const nn::ui2d::AnimResource& rResource, LayoutEx* pLayout, nn::ui2d::Pane* pPane, bool enabled) {
    BindPane(pPane, false);
    SetupBasic(rResource, pLayout, enabled);
}
// rResource supplies animation metadata; pLayout owns playback; pGroup selects panes to bind;
// enabled selects the initial animation state.
void Animator::SetupWithGroup(const nn::ui2d::AnimResource& rResource, LayoutEx* pLayout, nn::ui2d::Group* pGroup, bool enabled) {
    BindGroup(pGroup);
    SetupBasic(rResource, pLayout, enabled);
}
// rResource lists groups; pLayout owns playback; pGroups resolves names, index selects a group,
// and enabled selects the initial animation state. Invalid indices leave the animator unchanged.
void Animator::SetupWithGroupIndex(const nn::ui2d::AnimResource& rResource, LayoutEx* pLayout,
    nn::ui2d::GroupContainer* pGroups, u32 index, bool enabled) {
    if (index >= rResource.GetGroupCount()) return;
    BindGroup(pGroups->FindGroupByName(rResource.GetGroupArray()[index].name));
    SetupBasic(rResource, pLayout, enabled);
}
// rResource lists every group to bind; pLayout owns playback, pGroups resolves names,
// and enabled selects the initial animation state.
void Animator::SetupWithGroupAll(const nn::ui2d::AnimResource& rResource, LayoutEx* pLayout,
    nn::ui2d::GroupContainer* pGroups, bool enabled) {
    const int count = rResource.GetGroupCount();
    for (int i = 0; i < count; ++i) BindGroup(pGroups->FindGroupByName(rResource.GetGroupArray()[i].name));
    SetupBasic(rResource, pLayout, enabled);
}
// step supplies the frame increment; the resource selects looping or one-time playback.
void Animator::PlayAuto(float step) { Play(IsLoopData() ? cPlayType_Loop : cPlayType_OneTime, step); }
// frame selects the stopped pose, which remains enabled for evaluation.
void Animator::Stop(float frame) {
    mStep = 0;
    mFrame = frame;
    mFlags &= ~0xf;
    SetEnabled(true);
    mLayout->mScreen->animatorOperationCallback(Screen::AnimatorOperationType::cStop, this);
}
void Animator::StopCurrent() {
    mStep = 0;
    mFlags &= ~0xf;
    SetEnabled(true);
    mLayout->mScreen->animatorOperationCallback(Screen::AnimatorOperationType::cStopCurrent, this);
}
void Animator::StopAtMin() {
    mStep = 0;
    mFrame = 0;
    mFlags &= ~0xf;
    SetEnabled(true);
    mLayout->mScreen->animatorOperationCallback(Screen::AnimatorOperationType::cStopAtMin, this);
}
void Animator::StopAtMax() {
    mStep = 0;
    mFrame = GetFrameSize();
    mFlags &= ~0xf;
    SetEnabled(true);
    mLayout->mScreen->animatorOperationCallback(Screen::AnimatorOperationType::cStopAtMax, this);
}
// enabled controls evaluation; enabling registers the animator with its screen exactly once.
void Animator::SetEnabled(bool enabled) {
    nn::ui2d::AnimTransform::SetEnabled(enabled);
    if (enabled) {
        if (!mActiveLink.IsLinked()) mLayout->mScreen->setAnimatorActive(this);
    } else mStep = 0;
}
void Animator::DisableAndEraseFromActiveList() {
    nn::ui2d::AnimTransform::SetEnabled(false);
    mStep = 0;
    if (mActiveLink.IsLinked()) mLayout->mScreen->eraseAnimatorFromActiveList(this);
}
// step scales this update's playback increment; endpoints stop, wrap, or reflect according to the mode.
void Animator::UpdateFrame(float step) {
    mFlags &= ~0xf;
    if (mStep == 0) return;
    const float previousFrame = mFrame;
    float frame = mStep * step + previousFrame;
    if (mStep > 0) {
        if (frame >= GetFrameSize()) {
            switch (mPlayType) {
            case cPlayType_OneTime: frame = GetFrameSize(); mStep = 0; mFlags |= 1; break;
            case cPlayType_Loop: frame -= GetFrameSize(); mFlags |= 4; break;
            case cPlayType_RoundTrip: frame = GetFrameSize() + (GetFrameSize() - frame); mStep = -mStep; mFlags |= 4; break;
            }
        }
    } else if (frame <= 0) {
        switch (mPlayType) {
        case cPlayType_OneTime: frame = 0; mStep = 0; mFlags |= 1; break;
        case cPlayType_Loop: frame += GetFrameSize(); mFlags |= 8; break;
        case cPlayType_RoundTrip: frame = -frame; mStep = -mStep; mFlags |= 8; break;
        }
    }
    mFrame = frame;
}
Animator::Animator() : mStep(0), mLoopCount(0), mPlayType(0), mFlags(0x20),
                       mLayout(nullptr), mName(nullptr) {}
Animator::~Animator() = default;

// frame is the initial frame, type controls playback, and step is the frame increment.
void Animator::PlayFromFrame(float frame, PlayType type, float step) {
    mFrame = frame;
    PlayFromCurrent(type, step);
}

// rOther supplies the frame, speed, and playback mode to mirror.
void Animator::Synchronize(const Animator& rOther) {
    if (rOther.mStep != 0) {
        PlayFromFrame(rOther.mFrame, static_cast<PlayType>(rOther.mPlayType), rOther.mStep);
    } else {
        Stop(rOther.mFrame);
    }
}
}
