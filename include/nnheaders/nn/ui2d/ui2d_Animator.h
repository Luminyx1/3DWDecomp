#pragma once
#include <nn/ui2d/ui2d_AnimTransform.h>
namespace nn::ui2d {
class AnimResource;
class GroupContainer;
class Animator : public AnimTransformBasic {
public:
    enum PlayType { PlayType_Once, PlayType_Loop, PlayType_RoundTrip };
    Animator();
    ~Animator() override;
    NN_RUNTIME_TYPEINFO(AnimTransformBasic);
    void UpdateFrame(float step) override;
    virtual void Play(PlayType type, float speed);
    virtual void PlayAuto(float speed);
    virtual void PlayFromCurrent(PlayType type, float speed);
    virtual void StopAt(float frame);
    virtual void StopAtCurrentFrame();
    virtual void StopAtStartFrame();
    virtual void StopAtEndFrame();
    virtual const char* GetTagName() const;
    virtual void Unbind() = 0;
    virtual void SetupAnimationResource(const AnimResource& resource);
    float mSpeed;
    PlayType mPlayType;
    u32 mFlags;
};
static_assert(sizeof(Animator) == 0x48, "Animator size");
class PaneAnimator : public Animator {
public:
    NN_RUNTIME_TYPEINFO(Animator);
    void Setup(Pane* pane, bool enabled);
    void Unbind() override;
    Pane* mPane;
};
class GroupAnimator : public Animator {
public:
    NN_RUNTIME_TYPEINFO(Animator);
    void Unbind() override;
    void Setup(Group* group, bool enabled);
    void Setup(const AnimResource& resource, GroupContainer* groups, int index, bool enabled);
    Group* mGroup;
};
class GroupArrayAnimator : public Animator {
public:
    NN_RUNTIME_TYPEINFO(Animator);
    void Setup(const AnimResource& resource, GroupContainer* groups, Group** storage, bool enabled);
    void Unbind() override;
    Group** mGroups;
    int mGroupCount;
};
}
