#pragma once
#include <nn/ui2d/ui2d_AnimTransform.h>

namespace nn::ui2d { class AnimResource; class GroupContainer; }
namespace eui {
class LayoutEx;
class Animator : public nn::ui2d::AnimTransformBasic {
public:
    enum PlayType { cPlayType_OneTime, cPlayType_Loop, cPlayType_RoundTrip };
    Animator();
    ~Animator() override;
    NN_RUNTIME_TYPEINFO(nn::ui2d::AnimTransformBasic);
    void UpdateFrame(float step) override;
    void SetEnabled(bool enabled) override;
    virtual void Play(PlayType type, float step);
    virtual void PlayAuto(float step);
    virtual void PlayFromCurrent(PlayType type, float step);
    virtual void Stop(float frame);
    virtual void StopCurrent();
    virtual void StopAtMin();
    virtual void StopAtMax();
    void SetupBasic(const nn::ui2d::AnimResource&, LayoutEx*, bool);
    void SetupWithPane(const nn::ui2d::AnimResource&, LayoutEx*, nn::ui2d::Pane*, bool);
    void SetupWithGroup(const nn::ui2d::AnimResource&, LayoutEx*, nn::ui2d::Group*, bool);
    void SetupWithGroupIndex(const nn::ui2d::AnimResource&, LayoutEx*, nn::ui2d::GroupContainer*, u32, bool);
    void SetupWithGroupAll(const nn::ui2d::AnimResource&, LayoutEx*, nn::ui2d::GroupContainer*, bool);
    void PlayFromFrame(float frame, PlayType type, float step);
    void PlayRandom(PlayType type, float step);
    void Synchronize(const Animator& rOther);
    void DisableAndEraseFromActiveList();
    nn::util::IntrusiveListNode mActiveLink;
    float mStep;
    u16 mLoopCount;
    u8 mPlayType;
    u8 mFlags;
    LayoutEx* mLayout;
    const char* mName;
};
static_assert(sizeof(Animator) == 0x68, "Animator size");
}
