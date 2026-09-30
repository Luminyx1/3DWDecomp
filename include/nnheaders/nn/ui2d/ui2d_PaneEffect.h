#pragma once
#include <nn/ui2d/ui2d_TexMap.h>
namespace nn::ui2d {
class Pane;
namespace detail {
class PaneEffect {
public:
    PaneEffect();
    void* GetPrivateTexturePtr(const TexMap& map);
    Pane* mPane;
    struct Parameters {
        u8 _08[8];
        TexMap maskTexture, shadowTexture;
        u8 _30[0x30];
        TexMap effectTexture;
        u8 _70[0x1b9];
    } mParameters;
};
}
}
