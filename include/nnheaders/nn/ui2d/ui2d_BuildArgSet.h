#pragma once

#include <nn/ui2d/ui2d_Pane.h>

namespace nn::ui2d {

struct BuildArgSet {
    unsigned char _00[0x20];
    Layout* m_pPartsLayout;
    Layout* m_pLayout;
    unsigned char _30[0x60];
    int mDynamicTexturePrefixDepth;
    int mAlternateDynamicTexturePrefixDepth;
};

}  // namespace nn::ui2d
