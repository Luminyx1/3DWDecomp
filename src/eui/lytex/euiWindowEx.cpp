#include <eui/euiWindowEx.h>

#include <gfx/nin/seadGraphicsNvn.h>

namespace eui {

/**
 * @brief Creates a window with the requested frame and texture counts.
 * @param[in] frameCount Number of window frames to allocate.
 * @param[in] textureCount Number of texture slots to allocate.
 */
WindowEx::WindowEx(u8 frameCount, u8 textureCount) : Window(frameCount, textureCount) {}

/**
 * @brief Builds a window using the active graphics device.
 * @param[in] pResource Base pane resource to construct from.
 * @param[in] pOverride Override pane resource passed to the NintendoWare constructor.
 * @param[in] rArgs Layout construction arguments and resource context.
 */
WindowEx::WindowEx(const nn::ui2d::ResWindow* pResource,
                   const nn::ui2d::ResWindow* pOverride, const nn::ui2d::BuildArgSet& rArgs)
    : Window(nullptr, reinterpret_cast<nn::gfx::Device*>(sead::GraphicsNvn::instance()->getGfxDevice()),
             pResource, pOverride, rArgs) {}

/**
 * @brief Copies a window and its graphics resources.
 * @param[in] rOther Source object to copy.
 */
WindowEx::WindowEx(const WindowEx& rOther)
    : Window(rOther, reinterpret_cast<nn::gfx::Device*>(sead::GraphicsNvn::instance()->getGfxDevice())) {}

static_assert(sizeof(WindowEx) == 0x130, "WindowEx size");

}  // namespace eui
