#pragma once
#include <nn/ui2d/ui2d_Animator.h>
namespace nn::ui2d {
class AnimatorEx : public Animator {
public:
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
    // Instance storage is not yet reconstructed; use through pointers only.
};
}
