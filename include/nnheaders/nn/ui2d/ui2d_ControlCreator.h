#pragma once
#include <nn/gfx/gfx_Types.h>
namespace nn::ui2d {
class Layout;
class ControlSrc;
class ControlCreator {
public:
    virtual ~ControlCreator() = default;
    virtual void CreateControl(nn::gfx::Device* pDevice, Layout* pLayout, const ControlSrc& rSource) = 0;
};
}
