#pragma once
#include <eui/euiAnimButton.h>
namespace eui {
class DragScrollButton : public AnimButton {
public:
    DragScrollButton();
    const char* getClassName() const override;
    NN_RUNTIME_TYPEINFO(AnimButton);
    void StartDrag(const sead::Vector2f& rPosition) override;
    void UpdateDrag(const sead::Vector2f* pPosition) override;
    void FinishDrag(const sead::Vector2f* pPosition) override;
};

static_assert(sizeof(DragScrollButton) == 0x68, "DragScrollButton size");
}
