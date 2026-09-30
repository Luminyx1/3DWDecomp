#pragma once

#include <eui/euiAnimButton.h>

namespace eui {

class NormalButton : public AnimButton {
public:
    NormalButton() = default;
    NormalButton(const NormalButton& rOther, LayoutEx* pLayout, sead::Heap* pHeap);
    ~NormalButton() override = default;
    const char* getClassName() const override;
    NN_RUNTIME_TYPEINFO(AnimButton);
};

}  // namespace eui
