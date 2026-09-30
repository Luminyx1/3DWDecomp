#pragma once
#include <nn/ui2d/ui2d_Animator.h>
#include <nn/ui2d/ui2d_AnimResource.h>
namespace nn::ui2d {
class LayoutEx;
class AnimatorEx : public Animator {
public:
    AnimatorEx();
    void SetupBasic(const AnimResource& resource, LayoutEx* layout, bool enabled);
    void SetupWithPane(const AnimResource& resource, LayoutEx* layout, Pane* pane, bool enabled);
    void SetupWithGroup(const AnimResource& resource, LayoutEx* layout, Group* group, bool enabled);
    void Synchronize(const AnimatorEx& other);
    NN_RUNTIME_TYPEINFO(Animator);
    ~AnimatorEx() override;
    void UpdateFrame(float step) override;
    void SetEnabled(bool enabled) override;
    void Play(PlayType type, float speed) override;
    void PlayFromCurrent(PlayType type, float speed) override;
    void StopAt(float frame) override;
    void StopAtCurrentFrame() override;
    void StopAtStartFrame() override;
    void StopAtEndFrame() override;
    const char* GetTagName() const override;
    void Unbind() override;
    void SetupAnimationResource(const AnimResource& resource) override;
    virtual void PlayFromFrame(float frame, PlayType type, float speed);
    virtual void PlayRandom(PlayType type, float speed);
    virtual void StopRandom();
    nn::util::IntrusiveListNode mActiveLink;
    LayoutEx* mLayout;
    const ResAnimationTagBlock* mTag;
    u32 mExFlags;
};
static_assert(sizeof(AnimatorEx) == 0x70, "AnimatorEx size");
}
