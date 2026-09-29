#include <eui/euiWindowEx.h>

#include <gfx/nin/seadGraphicsNvn.h>

namespace eui {

/** @brief Creates a window with the requested frame and texture counts. */
WindowEx::WindowEx(u8 frameCount, u8 textureCount) : Window(frameCount, textureCount) {}

/** @brief Builds a window using the active graphics device. */
WindowEx::WindowEx(const nn::ui2d::ResWindow* pResource,
                   const nn::ui2d::ResWindow* pOverride, const nn::ui2d::BuildArgSet& rArgs)
    : Window(nullptr, reinterpret_cast<nn::gfx::Device*>(sead::GraphicsNvn::instance()->getGfxDevice()),
             pResource, pOverride, rArgs) {}

/** @brief Copies a window and its graphics resources. */
WindowEx::WindowEx(const WindowEx& rOther)
    : Window(rOther, reinterpret_cast<nn::gfx::Device*>(sead::GraphicsNvn::instance()->getGfxDevice())) {}

static_assert(sizeof(WindowEx) == 0x130, "WindowEx size");

}  // namespace eui
