#include <nn/ui2d/ui2d_Picture.h>
namespace nn::ui2d {
// index selects one of the four picture corners.
nn::util::Unorm8x4 Picture::GetVertexColor(int index) const { return m_VertexColors[index]; }
// index selects the corner; color supplies its RGBA channels.
void Picture::SetVertexColor(int index, const nn::util::Unorm8x4& color) { m_VertexColors[index] = color; }
// index selects a channel across the four corner colors.
u8 Picture::GetVertexColorElement(int index) const { return reinterpret_cast<const u8*>(&m_VertexColors[index / 4])[index % 4]; }
// index selects a corner channel; value supplies its new intensity.
void Picture::SetVertexColorElement(int index, u8 value) { reinterpret_cast<u8*>(&m_VertexColors[index / 4])[index % 4] = value; }
u32 Picture::GetMaterialCount() const { return m_pMaterial != nullptr; }
// index zero selects the picture's material; other indices have no material.
Material* Picture::GetMaterial(int index) const { GetMaterialCount(); return index == 0 ? m_pMaterial : nullptr; }
}
