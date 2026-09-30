#pragma once
#include <eui/euiControlBase.h>
#include <math/seadVector.h>
#include <prim/seadSafeString.h>
namespace nn::ui2d { class Pane; class ControlSrc; }
namespace eui {
class Animator;
class BoxCursorNode;
class Screen;
class DrawTarget;
class LayoutEx;
class BoxCursorControl : public ControlBase {
public:
    BoxCursorControl();
    ~BoxCursorControl() override;
    const char* getClassName() const override;
    NN_RUNTIME_TYPEINFO(ControlBase);
    void Update(float step) override;
    void clearActiveAndReservedActiveNode(const BoxCursorNode* pNode);
    Screen* getActiveNodeScreen();
    void initialize(const nn::ui2d::ControlSrc& rSource, LayoutEx* pLayout);
    void updateActiveNode(DrawTarget target);
    void selectActiveNode_();
    void setActiveNode_(const BoxCursorNode* pNode);
    void decideNode(bool repeat);
    bool isEqualActiveNodePartsLayoutName(const sead::SafeString& rName) const;
    nn::ui2d::Pane* mTopLeft;
    nn::ui2d::Pane* mTopRight;
    nn::ui2d::Pane* mBottomLeft;
    nn::ui2d::Pane* mBottomRight;
    const BoxCursorNode* mActiveNode;
    const BoxCursorNode* mReservedActiveNode;
    sead::Vector2f mPosition;
};
static_assert(sizeof(BoxCursorControl) == 0x60, "BoxCursorControl size");
}
