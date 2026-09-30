#include <eui/euiAlignPane.h>
namespace eui {
// NON_MATCHING: constructor byte stores and boolean conversion still differ.
// kind selects alignment; extendEdge fills edge space; margin separates panes; vertical selects the axis.
AlignPane::AlignPane(AlignKind kind, bool extendEdge, float margin, bool vertical)
    : _d4(0), _d8(0), mAlignKind(kind), mDirty(1), mExtendEdge(extendEdge), mFlags(vertical),
      mDefaultMargin(margin), _e4(-1) {}
AlignPane::~AlignPane() = default;
// kind selects the alignment to apply during the next calculation.
void AlignPane::setAlignKind(AlignKind kind) { mAlignKind = kind; mDirty = 1; }
// extend controls whether edge panes fill the available alignment extent.
void AlignPane::setExtendEdge(bool extend) { mExtendEdge = extend; mDirty = 1; }
// margin is the default spacing between aligned panes.
void AlignPane::setDefaultMargin(float margin) { mDefaultMargin = margin; mDirty = 1; }
}
