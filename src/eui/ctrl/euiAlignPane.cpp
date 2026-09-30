#include <eui/euiAlignPane.h>
namespace eui {
// NON_MATCHING: constructor byte stores and boolean conversion still differ.
// kind selects alignment; extendEdge fills edge space; margin separates panes; vertical selects the axis.
AlignPane::AlignPane(AlignKind kind, bool extendEdge, float margin, bool vertical)
    : _d4(0), _d8(0), mAlignKind(kind), mExtendEdge(extendEdge),
      mDefaultMargin(margin), _e4(-1) { mDirty = 1; mFlags = vertical; }
AlignPane::~AlignPane() = default;
// kind selects the alignment to apply during the next calculation.
void AlignPane::setAlignKind(AlignKind kind) { mAlignKind = kind; mDirty = 1; }
// extend controls whether edge panes fill the available alignment extent.
void AlignPane::setExtendEdge(bool extend) { mExtendEdge = extend; mDirty = 1; }
// margin is the default spacing between aligned panes.
void AlignPane::setDefaultMargin(float margin) { mDefaultMargin = margin; mDirty = 1; }
void AlignPane::updateScroll_() {
    for (auto* link = m_Children.GetNext(); link != &m_Children; link = link->GetNext()) {
        auto* pane = reinterpret_cast<nn::ui2d::Pane*>(reinterpret_cast<char*>(link) - offsetof(nn::ui2d::Pane, m_Link));
        const float position = (_d4 - _d8) + pane->mPositionX;
        pane->mFlags |= 0x10;
        pane->mPositionX = position;
    }
}

void AlignPane::updateScrollVertical_() {
    for (auto* link = m_Children.GetNext(); link != &m_Children; link = link->GetNext()) {
        auto* pane = reinterpret_cast<nn::ui2d::Pane*>(reinterpret_cast<char*>(link) - offsetof(nn::ui2d::Pane, m_Link));
        const float position = (_d4 - _d8) + pane->mPositionY;
        pane->mFlags |= 0x10;
        pane->mPositionY = position;
    }
}

// NON_MATCHING: branch relocations await the alignment and sizing helpers.
// adjustSize permits resizing this pane after its children have been aligned successfully.
void AlignPane::updateAlign_(bool adjustSize) {
    if (mDirty && (nn::ui2d::Pane::mFlags & 1) && mAlpha) {
        bool dirty;
        if (!(mFlags & 1)) dirty = !doAlign_();
        else dirty = !doAlignVertical_();
        if (adjustSize && !dirty && (nn::ui2d::Pane::mFlags & 4)) adjustPaneSize_();
        mDirty = dirty;
    }
}

// NON_MATCHING: scroll-delta scheduling and helper relocations still differ.
// rDrawInfo and rContext supply layout state; force requests recalculation of the base pane.
void AlignPane::Calculate(nn::ui2d::DrawInfo& rDrawInfo, CalculateContext& rContext, bool force) {
    updateAlign_(true);
    if (_d4 != _d8) {
        if (mFlags & 1) updateScrollVertical_();
        else updateScroll_();
        _d8 = _d4;
    }

    nn::ui2d::Pane::Calculate(rDrawInfo, rContext, force);
}
}
