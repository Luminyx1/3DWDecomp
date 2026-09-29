#pragma once

#include <nn/ui2d/ui2d_Pane.h>

namespace nn::ui2d {

struct BuildArgSet {
    unsigned char _00[0x28];
    Layout* m_pLayout;
};

}  // namespace nn::ui2d
