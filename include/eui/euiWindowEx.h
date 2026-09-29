#pragma once

#include <nn/ui2d/ui2d_Window.h>

namespace eui {
class WindowEx : public nn::ui2d::Window {
public:
    WindowEx(u8 frameCount, u8 textureCount);
    WindowEx(const nn::ui2d::ResWindow* pResource, const nn::ui2d::ResWindow* pOverride,
             const nn::ui2d::BuildArgSet& rArgs);
    WindowEx(const WindowEx& rOther);
    /** @brief Destroys the extended window pane. */
    ~WindowEx() override = default;
    NN_RUNTIME_TYPEINFO(nn::ui2d::Window);
};
}  // namespace eui
