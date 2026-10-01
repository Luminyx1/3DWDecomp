#pragma once

namespace agl {
class DrawContext;
class TextureData;
}  // namespace agl

namespace al {
class FullScreenTriangle;
class ShaderHolder;
}  // namespace al

namespace alViewRendererFunction {
void createLinearDepthFromDepthBuffer(agl::DrawContext* pDrawContext,
                                      const al::ShaderHolder* pShaderHolder,
                                      const agl::TextureData* pLinearDepth,
                                      const agl::TextureData* pDepthBuffer,
                                      const al::FullScreenTriangle* pTriangle);
}  // namespace alViewRendererFunction
