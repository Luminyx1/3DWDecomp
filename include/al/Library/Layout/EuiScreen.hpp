#pragma once

#include <eui/euiScreen.h>

namespace sead {
class Heap;
}

namespace al {
class EuiScreen : public eui::Screen {
    SEAD_RTTI_OVERRIDE(EuiScreen, eui::Screen);

public:
    EuiScreen();
    ~EuiScreen() override;

    void doSetupDrawInfo_() override;

private:
    sead::Heap* mHeap = nullptr;
};

static_assert(sizeof(EuiScreen) == 0xf8);
}  // namespace al
