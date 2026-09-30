#pragma once
#include <nn/ui2d/ui2d_AnimButton.h>
#include <nn/ui2d/ui2d_Types.h>
namespace nn::ui2d {
struct Size;
class DrawInfo;
class ControlCreator;
class ResourceAccessor;
class LayoutEx;
class AnimatorEx;
class ScreenBase {
public:
    struct InputDeviceState;
    NN_RUNTIME_TYPEINFO_BASE();
    virtual void DrawCaptureTexture(nn::gfx::Device*, nn::gfx::CommandBuffer&) = 0;
    virtual void DrawLayout(nn::gfx::CommandBuffer&) = 0;
    virtual DrawInfo* CreateDrawInfo_() = 0;
    virtual ControlCreator* CreateControlCreator_() = 0;
};
class Screen : public ScreenBase {
public:
    enum AnimatorOperationType : int;
    NN_RUNTIME_TYPEINFO(ScreenBase);
    void DrawCaptureTexture(nn::gfx::Device*, nn::gfx::CommandBuffer&) override;
    void DrawLayout(nn::gfx::CommandBuffer&) override;
    DrawInfo* CreateDrawInfo_() override;
    ControlCreator* CreateControlCreator_() override;
    virtual ~Screen();
    virtual const char* GetLayoutName() const;
    virtual Size GetViewportSize() const;
    virtual ControlCreator* GetControlCreator() const;
    virtual void Open();
    virtual void OpenDirect();
    virtual void Close();
    virtual void CloseDirect();
    virtual void HandleEventOnButtonStateChanged(AnimButton* button, ButtonBase::State previous, ButtonBase::State next);
    virtual void HandleEventOnAnimatorOperation(AnimatorOperationType operation, AnimatorEx* animator);
    virtual void UnloadForReplaceViewerCallback();
    virtual void DoOpenStart_();
    virtual void DoOpenEnd_();
    virtual void DoCloseStart_();
    virtual void DoCloseEnd_();
    virtual bool IsPlayPartsInOut_() const;
    virtual LayoutEx* DoAllocateLayout_(nn::gfx::Device*);
    virtual void DoBuildLayout_(nn::gfx::Device*, const char*, ResourceAccessor*);
    virtual void DeleteLayout_(nn::gfx::Device*);
    virtual void DoBuildAnimatons_(nn::gfx::Device*, const void*);
    virtual void DoDestroyAnimatons_(nn::gfx::Device*);
    virtual void CreateRestAnimators_(nn::gfx::Device*, LayoutEx*);
    virtual void DoInitialize_(nn::gfx::Device*);
    virtual void DoFinalize_(nn::gfx::Device*);
    virtual void SetupPaneAfterBuild_(nn::gfx::Device*, Pane*, Layout*);
    virtual void SetupPaneAfterBuildRecursively_(nn::gfx::Device*, Pane*, Layout*);
    virtual void UpdateScreenOpening_();
    virtual void UpdateScreenClosing_();
    virtual void DoUpdate_(nn::gfx::Device*);
    virtual void UpdateUserInput_(const nn::util::Float2*, bool, bool);
    virtual void UpdateButtons_(nn::gfx::Device*, const InputDeviceState&);
    virtual void UpdateControl_(nn::gfx::Device*);
    virtual void UpdateAnimator_();
    virtual ResourceAccessor* DoCreateResourceAccessor_();
    virtual void OnPostCalculate(Layout*);
    void SetAnimatorActive(AnimatorEx* animator);
    void EraseAnimatorFromActiveList(AnimatorEx* animator);
    u8 _08[0x28];
    nn::util::IntrusiveListNode mActiveAnimators;
    u8 _40[0xd0];
    ControlCreator* mControlCreator;
    u8 _118[0x48];
    int mScreenId;
    // Remaining instance storage is not yet reconstructed; use through pointers only.
};
}
