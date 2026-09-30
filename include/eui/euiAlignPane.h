#pragma once
#include <nn/ui2d/ui2d_Pane.h>
namespace eui {
class LayoutEx;
class AlignPane : public nn::ui2d::Pane {
public:
    enum AlignKind { cAlignKind_0, cAlignKind_1, cAlignKind_2 };
    AlignPane(AlignKind kind, bool extendEdge, float margin, bool vertical);
    ~AlignPane() override;
    NN_RUNTIME_TYPEINFO(nn::ui2d::Pane);
    void Calculate(nn::ui2d::DrawInfo& rDrawInfo, CalculateContext& rContext, bool force) override;
    void setAlignKind(AlignKind kind);
    void setExtendEdge(bool extend);
    void setDefaultMargin(float margin);
    void updateScroll_();
    void updateScrollVertical_();
    void updateAlign_(bool adjustSize);
    bool doAlign_();
    bool doAlignVertical_();
    void adjustPaneSize_();
    float _d4;
    float _d8;
    u8 mAlignKind;
    u8 mDirty;
    bool mExtendEdge;
    u8 mFlags;
    float mDefaultMargin;
    float _e4;
};

static_assert(sizeof(AlignPane) == 0xe8, "AlignPane size");
}
