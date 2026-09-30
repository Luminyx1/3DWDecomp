#pragma once
#include <nn/ui2d/ui2d_AnimButton.h>
namespace nn::ui2d {
class NormalButton : public AnimButton {
public:
    NormalButton();
    NN_RUNTIME_TYPEINFO(AnimButton);
    void FinishDown() override;
};
class NormalButtonEx : public AnimButton {
public:
    NormalButtonEx();
    NormalButtonEx(nn::gfx::Device* device, const NormalButtonEx& source, Layout* layout);
    NN_RUNTIME_TYPEINFO(AnimButton);
    void FinishDown() override;
    Layout* GetLayout() override { return mLayout; }
    // layout is the owner retained for state-change notifications.
    void SetLayout(Layout* layout) override { mLayout = layout; }
    Layout* mLayout;
};
static_assert(sizeof(NormalButtonEx) == 0x98, "NormalButtonEx size");
}
