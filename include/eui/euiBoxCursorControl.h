#pragma once
#include <eui/euiControlBase.h>
#include <math/seadVector.h>
namespace eui {
class Animator;
class BoxCursorNode;
class Screen;
class BoxCursorControl : public ControlBase {
public:
    BoxCursorControl();
    ~BoxCursorControl() override;
    const char* getClassName() const override;
    NN_RUNTIME_TYPEINFO(ControlBase);
    void Update(float step) override;
    void clearActiveAndReservedActiveNode(const BoxCursorNode* pNode);
    Screen* getActiveNodeScreen();
    Animator* _28;
    Animator* _30;
    Animator* _38;
    Animator* _40;
    const BoxCursorNode* mActiveNode;
    const BoxCursorNode* mReservedActiveNode;
    sead::Vector2f mPosition;
};
static_assert(sizeof(BoxCursorControl) == 0x60, "BoxCursorControl size");
}
