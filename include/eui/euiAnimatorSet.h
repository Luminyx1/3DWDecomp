#pragma once
#include <container/seadBuffer.h>
#include <prim/seadRuntimeTypeInfo.h>
#include <prim/seadSafeString.h>

namespace eui {
class Animator;
class LayoutEx;
class AnimatorSet {
public:
    SEAD_RTTI_BASE(AnimatorSet);
    AnimatorSet();
    AnimatorSet(const AnimatorSet& rOther, LayoutEx* pLayout, sead::Heap* pHeap);
    virtual ~AnimatorSet();
    void allocBuffer(u32 count, sead::Heap* pHeap);
    void setBuffer(u32 count, Animator** ppBuffer);
    void setAnimator(u32 index, Animator* pAnimator);
    Animator* select(u32 index);
    Animator* select(const sead::SafeString& rName);
    u32 findSelectedIndex() const;
    void SetSkipFirstFrameAll(bool skip);
    void SetSoundLinkAll(bool enabled);
    sead::Buffer<Animator*> mAnimators;
    Animator* mSelected;
};

static_assert(sizeof(AnimatorSet) == 0x20, "AnimatorSet size");
}
