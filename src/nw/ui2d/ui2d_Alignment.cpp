#include <nn/ui2d/ui2d_Alignment.h>
namespace nn::ui2d {
Alignment::Alignment() : mAlignment(0), mDefaultMargin(0), mMode(0), mAlignmentFlags(0) {}
Alignment::~Alignment() = default;
void Alignment::RequestAlignment() { mAlignmentFlags |= 1; }
float Alignment::GetDefaultMargin() const { return mDefaultMargin; }
u32 Alignment::GetHorizontalAlignment() const { return mAlignment; }
u32 Alignment::GetVerticalAlignment() const { return mAlignment; }
bool Alignment::IsHorizontalAlignment() const { return !(mAlignmentFlags & 2); }
bool Alignment::IsVerticalAlignment() const { return (mAlignmentFlags & 2) != 0; }
void Alignment::MakeAlignment() {
    if (IsVerticalAlignment()) MakeVerticalAlignment(); else MakeHorizontalAlignment();
}
void Alignment::MakeHorizontalAlignment() {
    if (mAlignment <= 1) MakeForwardHorizontalAlignment(); else MakeReverseHorizontalAlignment();
}
void Alignment::MakeVerticalAlignment() {
    if (mAlignment <= 1) MakeForwardVerticalAlignment(); else MakeReverseVerticalAlignment();
}
}
