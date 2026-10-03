#include <nn/ui2d/ui2d_AnimatorEx.h>
#include <nn/ui2d/ui2d_LayoutEx.h>
#include <nn/ui2d/ui2d_Screen.h>
#include <nn/ui2d/ui2d_Group.h>
#include <nn/ui2d/ui2d_Pane.h>
namespace nn::ui2d {
const ResExtUserData* GetExtUserData(const ResExtUserDataList* list, const char* name);
/** @brief Construct an unbound animator with no owning layout, tag or extended playback flags. */
AnimatorEx::AnimatorEx() : mLayout(nullptr), mTag(nullptr), mExFlags(0) {}
/** @brief Destroy the animator using the base animation cleanup. */
AnimatorEx::~AnimatorEx() = default;
/**
 * @brief Associate a tag and owning layout with this animator, then set its activation state.
 * @param resource Resource supplying the animation tag.
 * @param layout Owning extended layout, required for activation callbacks.
 * @param enabled Whether the animation should initially be active.
 */
void AnimatorEx::SetupBasic(const AnimResource& resource, LayoutEx* layout, bool enabled) {
    mTag = resource.mTag;
    mLayout = layout;
    SetEnabled(enabled);
}

/**
 * @brief Bind a single pane without recursively binding its children.
 * @param resource Resource supplying the animation tag.
 * @param layout Owning extended layout used for activation callbacks.
 * @param pane Target pane to bind.
 * @param enabled Whether the bound animation should initially be active.
 */
void AnimatorEx::SetupWithPane(const AnimResource& resource, LayoutEx* layout, Pane* pane, bool enabled) {
    BindPane(pane, false);
    SetupBasic(resource, layout, enabled);
}

/**
 * @brief Bind the panes in a group and configure the animator's layout and tag.
 * @param resource Resource supplying the animation tag.
 * @param layout Owning extended layout used for activation callbacks.
 * @param group Group containing the target panes.
 * @param enabled Whether the bound animation should initially be active.
 */
void AnimatorEx::SetupWithGroup(const AnimResource& resource, LayoutEx* layout, Group* group, bool enabled) {
    BindGroup(group);
    SetupBasic(resource, layout, enabled);
}

/**
 * @brief Set the initial frame and begin playback using the current-frame path.
 * @param frame Initial frame; endpoint adjustment may occur in PlayFromCurrent.
 * @param type Once, looping or round-trip playback mode.
 * @param speed Signed playback rate passed to PlayFromCurrent.
 */
void AnimatorEx::PlayFromFrame(float frame, PlayType type, float speed) {
    mFrame = frame;
    PlayFromCurrent(type, speed);
}
/**
 * @brief Synchronize this animator's playback with another animator.
 * @param other Animator supplying frame, playback mode and speed; a stopped animator stops this one.
 */
void AnimatorEx::Synchronize(const AnimatorEx& other) {
    if (other.mSpeed != 0) {
        PlayFromFrame(other.mFrame, other.mPlayType, other.mSpeed);
    } else {
        StopAt(other.mFrame);
    }
}

/**
 * @brief Retrieve the animation tag's name.
 * @return Tag name stored in the resource, or nullptr when no tag is assigned.
 */
const char* AnimatorEx::GetTagName() const {
    return (mTag != nullptr) ? reinterpret_cast<const char*>(mTag) + mTag->nameOffset : nullptr;
}
/** @brief Leave externally managed pane bindings unchanged. */
void AnimatorEx::Unbind() {}
/**
 * @brief Associate the animator with the supplied animation tag.
 * @param resource Resource whose tag pointer is retained without taking ownership.
 */
void AnimatorEx::SetupAnimationResource(const AnimResource& resource) { mTag = resource.mTag; }

namespace {
/**
 * @brief Notify the owning screen that an animator operation occurred.
 * @param animator Initialized animator whose layout supplies the screen.
 * @param operation Operation or boundary-crossing event to report.
 */
inline void NotifyOperation(AnimatorEx* animator, Screen::AnimatorOperationType operation) {
    animator->mLayout->GetScreen()->HandleEventOnAnimatorOperation(operation, animator);
}
/**
 * @brief Choose an integer animation frame using the layout's shared random state.
 * @param frameCount Exclusive upper frame bound; must be positive.
 * @return Random frame in [0, frameCount).
 */
inline unsigned int RandomFrame(unsigned int frameCount) {
    u32* state = Layout::g_Random;
    u32 value = state[0] ^ (state[0] << 11);
    state[0] = state[1];
    state[1] = state[2];
    state[2] = state[3];
    state[3] = value ^ (value >> 8) ^ state[3] ^ (state[3] >> 19);
    return (static_cast<u64>(frameCount) * state[3]) >> 32;
}
} // namespace
/**
 * @brief Bind one resource group and configure its owning layout and activation state.
 * @param resource Resource containing the group-name array and animation tag.
 * @param layout Owning extended layout retained after a valid group is selected.
 * @param groups Container resolving the resource's group name.
 * @param index Zero-based group index; out-of-range indices leave the animator unchanged.
 * @param enabled Whether the bound animation initially participates in updates.
 */
void AnimatorEx::SetupWithGroupIndex(const AnimResource& resource, LayoutEx* layout, GroupContainer* groups,
                                     unsigned int index, bool enabled) {
    if (index < resource.GetGroupCount()) {
        SetupWithGroup(resource, layout, groups->FindGroupByName(resource.GetGroupArray()[index].name),
                       enabled);
    }
}
/**
 * @brief Bind all groups named by the resource and configure the animator.
 * @param resource Resource containing group names and the animation tag.
 * @param layout Owning extended layout retained by this animator.
 * @param groups Container resolving every referenced group name.
 * @param enabled Whether the animation initially participates in updates.
 */
void AnimatorEx::SetupWithGroupAll(const AnimResource& resource, LayoutEx* layout, GroupContainer* groups,
                                   bool enabled) {
    const int count = resource.GetGroupCount();
    for (int i = 0; i < count; ++i) {
        BindGroup(groups->FindGroupByName(resource.GetGroupArray()[i].name));
    }
    SetupBasic(resource, layout, enabled);
}
/**
 * @brief Enable the animation and register it as active, or disable it and stop playback.
 * @param enabled True to activate; false to stop playback without unlinking the active-list node.
 */
void AnimatorEx::SetEnabled(bool enabled) {
    AnimTransform::SetEnabled(enabled);
    if (enabled) {
        if (!mActiveLink.IsLinked()) {
            mLayout->GetScreen()->SetAnimatorActive(this);
        }
    } else {
        mSpeed = 0;
    }
}
/** @brief Disable playback and unlink this animator from its screen's active list. */
void AnimatorEx::DisableAndEraseFromActiveList() {
    AnimTransform::SetEnabled(false);
    mSpeed = 0;
    if (mActiveLink.IsLinked()) {
        mLayout->GetScreen()->EraseAnimatorFromActiveList(this);
    }
}
/**
 * @brief Begin playback at the direction-dependent initial frame and notify the screen.
 * @param type Once, looping or round-trip playback mode.
 * @param speed Signed frames per update, normally bounded by the animation frame count.
 */
void AnimatorEx::Play(PlayType type, float speed) {
    if (!(speed >= -static_cast<float>(GetFrameSize()) && speed <= static_cast<float>(GetFrameSize()))) {
        if (!IsWaitData() || type != PlayType_Once) {
            IsWaitData();
            return;
        }
    }
    mSpeed = speed;
    mPlayType = type;
    mFlags &= ~0xfu;
    float frame = speed;
    if (speed >= 0.0f) {
        if (!(mExFlags & 8) || GetFrameSize() == 0) {
            frame = 0.0f;
        }
    } else {
        if ((mExFlags & 8) && GetFrameSize() != 0) {
            frame = static_cast<float>(GetFrameSize()) + speed;
        } else {
            frame = static_cast<float>(GetFrameSize());
        }
    }
    mFrame = frame;
    SetEnabled(true);
    NotifyOperation(this, Screen::AnimatorOperationType_Play);
}
/**
 * @brief Resume playback, optionally advancing the current frame away from an endpoint.
 * @param type Once, looping or round-trip playback mode.
 * @param speed Signed frames per update, normally bounded by the animation frame count.
 */
void AnimatorEx::PlayFromCurrent(PlayType type, float speed) {
    if (!(speed >= -static_cast<float>(GetFrameSize()) && speed <= static_cast<float>(GetFrameSize()))) {
        if (!IsWaitData() || type != PlayType_Once) {
            IsWaitData();
            return;
        }
    }
    mSpeed = speed;
    mPlayType = type;
    mFlags &= ~0xfu;
    if ((mExFlags & 8) && GetFrameSize() != 0) {
        if (speed >= 0.0f) {
            if (mFrame < speed) {
                mFrame = speed;
            }
        } else {
            float lastFrame = static_cast<float>(GetFrameSize()) + speed;
            if (mFrame > lastFrame) {
                mFrame = lastFrame;
            }
        }
    }
    SetEnabled(true);
    NotifyOperation(this, Screen::AnimatorOperationType_PlayFromCurrent);
}
/**
 * @brief Start playback from a randomly selected integer frame.
 * @param type Once, looping or round-trip playback mode.
 * @param speed Signed playback rate passed to PlayFromCurrent.
 */
void AnimatorEx::PlayRandom(PlayType type, float speed) {
    mFrame = static_cast<float>(RandomFrame(GetFrameSize() + 1));
    PlayFromCurrent(type, speed);
}
/**
 * @brief Stop at the requested frame, keep the animation enabled and notify the screen.
 * @param frame Animation frame to retain without range validation.
 */
void AnimatorEx::StopAt(float frame) {
    Animator::StopAt(frame);
    SetEnabled(true);
    NotifyOperation(this, Screen::AnimatorOperationType_StopAt);
}
/** @brief Stop at the current frame and notify the screen while remaining enabled. */
void AnimatorEx::StopAtCurrentFrame() {
    Animator::StopAtCurrentFrame();
    SetEnabled(true);
    NotifyOperation(this, Screen::AnimatorOperationType_StopAtCurrentFrame);
}
/** @brief Stop at frame zero and notify the screen while remaining enabled. */
void AnimatorEx::StopAtStartFrame() {
    Animator::StopAtStartFrame();
    SetEnabled(true);
    NotifyOperation(this, Screen::AnimatorOperationType_StopAtStartFrame);
}
/** @brief Stop at the animation's final frame and notify the screen while remaining enabled. */
void AnimatorEx::StopAtEndFrame() {
    Animator::StopAtEndFrame();
    SetEnabled(true);
    NotifyOperation(this, Screen::AnimatorOperationType_StopAtEndFrame);
}
/** @brief Select a random frame, clear boundary flags and notify the screen without changing speed. */
void AnimatorEx::StopRandom() {
    mFrame = static_cast<float>(RandomFrame(GetFrameSize() + 1));
    mFlags &= ~0xfu;
    SetEnabled(true);
    NotifyOperation(this, Screen::AnimatorOperationType_StopAt);
}
/**
 * @brief Find named extended user data attached to this animator's tag.
 * @param name Null-terminated user-data entry name.
 * @return Matching entry, or nullptr when the tag has no user-data list or the name is absent.
 */
const ResExtUserData* AnimatorEx::FindExtUserData(const char* name) const {
    if (mTag->userDataOffset == 0) {
        return nullptr;
    }
    const auto* data = reinterpret_cast<const ResExtUserDataList*>(reinterpret_cast<const char*>(mTag) +
                                                                   mTag->userDataOffset);
    return GetExtUserData(data, name);
}
/**
 * @brief Advance playback and notify the screen when looping or round-trip playback crosses a boundary.
 * @param step Elapsed animation step multiplying the signed playback speed.
 */
void AnimatorEx::UpdateFrame(float step) {
    const float speed = mSpeed;
    mFlags &= ~0xfu;

    if (speed == 0.0f) {
        return;
    }
    float frame = mFrame + speed * step;

    if (speed > 0.0f) {
        if (frame >= float(GetFrameSize())) {
            switch (mPlayType) {
            case PlayType_Once:
                frame = float(GetFrameSize());
                mSpeed = 0;
                mFlags |= 1;
                break;
            case PlayType_Loop:
                frame -= float(GetFrameSize());
                mFlags |= 2;
                NotifyOperation(this, Screen::AnimatorOperationType_CrossEnd);
                break;
            case PlayType_RoundTrip:
                frame = float(GetFrameSize()) + (float(GetFrameSize()) - frame);
                mSpeed = -speed;
                mFlags |= 2;
                NotifyOperation(this, Screen::AnimatorOperationType_CrossEnd);
                break;
            }
        }
    } else {
        if (frame <= 0.0f) {
            switch (mPlayType) {
            case PlayType_Once:
                frame = 0;
                mSpeed = 0;
                mFlags |= 1;
                break;
            case PlayType_Loop:
                frame += float(GetFrameSize());
                mFlags |= 2;
                NotifyOperation(this, Screen::AnimatorOperationType_CrossStart);
                break;
            case PlayType_RoundTrip:
                frame = -frame;
                mSpeed = -speed;
                mFlags |= 2;
                NotifyOperation(this, Screen::AnimatorOperationType_CrossStart);
                break;
            }
        }
    }

    mFrame = frame;
}
} // namespace nn::ui2d
