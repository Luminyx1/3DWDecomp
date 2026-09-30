#include <nn/ui2d/ui2d_Window.h>
namespace nn::ui2d {
// index selects one of the four content corners.
nn::util::Unorm8x4 Window::GetVertexColor(int index) const { return mVertexColors[index]; }
// index selects a corner; color supplies its RGBA channels.
void Window::SetVertexColor(int index, const nn::util::Unorm8x4& color) { mVertexColors[index] = color; }
// index selects a channel across the four content-corner colors.
u8 Window::GetVertexColorElement(int index) const { return reinterpret_cast<const u8*>(&mVertexColors[index / 4])[index % 4]; }
// index selects a corner channel; value supplies its new intensity.
void Window::SetVertexColorElement(int index, u8 value) { reinterpret_cast<u8*>(&mVertexColors[index / 4])[index % 4] = value; }
u32 Window::GetMaterialCount() const { return mFrameCount + 1; }
}
