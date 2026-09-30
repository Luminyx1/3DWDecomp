#pragma once
#include <nn/ui2d/ui2d_Pane.h>
namespace nn::ui2d {
class Alignment : public Pane {
public:
    Alignment();
    ~Alignment() override;
    NN_RUNTIME_TYPEINFO(Pane);
    void Calculate(DrawInfo&, CalculateContext&, bool) override;
    void RequestAlignment();
    float GetDefaultMargin() const;
    u32 GetHorizontalAlignment() const;
    u32 GetVerticalAlignment() const;
    bool IsHorizontalAlignment() const;
    bool IsVerticalAlignment() const;
    void MakeAlignment();
    void MakeHorizontalAlignment();
    void MakeVerticalAlignment();
    void MakeForwardHorizontalAlignment();
    void MakeReverseHorizontalAlignment();
    void MakeForwardVerticalAlignment();
    void MakeReverseVerticalAlignment();
    u32 mAlignment;
    float mDefaultMargin;
    u8 mMode, mAlignmentFlags;
};
static_assert(sizeof(Alignment) == 0xe0, "Alignment size");
}
