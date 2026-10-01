#include <eui/euiAnimatorSet.h>
#include <eui/euiAnimator.h>
#include <eui/euiLayoutEx.h>

namespace eui {
AnimatorSet::AnimatorSet() : mSelected(nullptr) {}
AnimatorSet::~AnimatorSet() = default;

// count is the number of animation slots; pHeap supplies their backing storage.
void AnimatorSet::allocBuffer(u32 count, sead::Heap* pHeap) {
    mAnimators.tryAllocBuffer(count, pHeap);
    mAnimators.fill(nullptr);
}

// rOther supplies animator names and selection; pLayout owns the clones and pHeap stores their slots.
AnimatorSet::AnimatorSet(const AnimatorSet& rOther, LayoutEx* pLayout, sead::Heap* pHeap) : mSelected(nullptr) {
    if (!rOther.mAnimators.size()) return;
    allocBuffer(rOther.mAnimators.size(), pHeap);
    auto* const* source = rOther.mAnimators.getBufferPtr();
    const u32 count = rOther.mAnimators.size();

    for (size_t i = 0; i != count; ++i) {
        Animator* animator = source[i];

        if (animator != nullptr) {
            Animator* selected = rOther.mSelected;
            Animator* clone = pLayout->createAnimatorAuto(animator->mName, animator == selected);
            mAnimators.getBufferPtr()[i < u32(mAnimators.size()) ? i : 0] = clone;

            if (animator == selected) {
                const u32 capacity = mAnimators.size();
                auto** slots = mAnimators.getBufferPtr();
                mSelected = *(i < capacity ? slots + i : slots);
            }
        }
    }
}

// ppBuffer supplies count externally owned slots, which are cleared on attachment.
void AnimatorSet::setBuffer(u32 count, Animator** ppBuffer) {
    mAnimators.setBuffer(count, ppBuffer);
    mAnimators.fill(nullptr);
}

// index identifies the slot; pAnimator becomes the selection if none exists yet.
void AnimatorSet::setAnimator(u32 index, Animator* pAnimator) {
    mAnimators[index] = pAnimator;

    if (mSelected == nullptr) mSelected = pAnimator;
}

// index selects a stored animator, disabling the previously selected animation.
Animator* AnimatorSet::select(u32 index) {
    Animator* pNext = mAnimators[index];
    Animator* pPrevious = mSelected;

    if (pPrevious != pNext) {
        pPrevious->nn::ui2d::AnimTransform::SetEnabled(false);
        pPrevious->mStep = 0;
        mSelected = pNext;
    }

    return pNext;
}

u32 AnimatorSet::findSelectedIndex() const {
    auto* const* pAnimators = mAnimators.getBufferPtr();

    for (size_t i = 0, count = static_cast<u32>(mAnimators.size()); i != count; ++i)
        if (pAnimators[i] == mSelected) return i;
    return 0;
}

// skip controls whether each non-null animator skips its initial playback frame.
void AnimatorSet::SetSkipFirstFrameAll(bool skip) {
    auto** pAnimators = mAnimators.getBufferPtr();

    for (u32 i = 0, count = mAnimators.size(); i < count; ++i) {
        if (Animator* pAnimator = pAnimators[i]) {
            if (skip) pAnimator->mFlags |= 0x10;
            else pAnimator->mFlags &= ~0x10;
        }
    }
}

// enabled controls sound-linked playback for every non-null animator.
void AnimatorSet::SetSoundLinkAll(bool enabled) {
    auto** pAnimators = mAnimators.getBufferPtr();

    for (u32 i = 0, count = mAnimators.size(); i < count; ++i) {
        if (Animator* pAnimator = pAnimators[i]) {
            if (enabled) pAnimator->mFlags |= 0x20;
            else pAnimator->mFlags &= ~0x20;
        }
    }
}

// rName identifies an animator by its resource name; an unknown name returns null.
Animator* AnimatorSet::select(const sead::SafeString& rName) {
    auto** pAnimators = mAnimators.getBufferPtr();

    for (size_t i = 0, count = static_cast<u32>(mAnimators.size()); i != count; ++i) {
        if (pAnimators[i] != nullptr && rName == sead::SafeString(pAnimators[i]->mName)) {
            Animator* pPrevious = mSelected;

            if (pPrevious != pAnimators[i]) {
                pPrevious->nn::ui2d::AnimTransform::SetEnabled(false);
                pPrevious->mStep = 0;
                mSelected = pAnimators[i];
            }

            return mSelected;
        }
    }

    return nullptr;
}
}
