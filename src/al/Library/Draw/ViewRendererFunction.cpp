#include "Library/Draw/ViewRendererFunction.hpp"

#include <common/aglDrawContext.h>
#include <common/aglRenderBuffer.h>
#include <common/aglRenderTarget.h>
#include <common/aglShaderLocation.h>
#include <common/aglShaderProgram.h>
#include <common/aglTextureSampler.h>
#include <gfx/seadGraphicsContext.h>

#include "Library/Debug/Render/RenderBufferAttacher.hpp"
#include "Library/Projection/Projection.hpp"
#include "Library/Shader/DeferredRendering/FullScreenTriangle.hpp"
#include "Library/Shader/ForwardRendering/ShaderHolder.hpp"

namespace alViewRendererFunction {
/**
 * Converts a depth buffer into a linear depth texture.
 * @param pDrawContext Draw context.
 * @param pShaderHolder Shader holder to take the conversion shader from.
 * @param pLinearDepth Linear depth texture to write.
 * @param pDepthBuffer Depth buffer to read.
 * @param pTriangle Screen covering triangle.
 */
void createLinearDepthFromDepthBuffer(agl::DrawContext* pDrawContext,
                                      const al::ShaderHolder* pShaderHolder,
                                      const agl::TextureData* pLinearDepth,
                                      const agl::TextureData* pDepthBuffer,
                                      const al::FullScreenTriangle* pTriangle) {
    {
        agl::RenderTargetDepth depthTarget;
        depthTarget.applyTextureData(*pDepthBuffer);
        depthTarget.expandHiZBuffer(pDrawContext);
    }

    agl::RenderBuffer renderBuffer;
    al::RenderBufferAttacher attacher(pDrawContext, &renderBuffer, pLinearDepth, nullptr, nullptr,
                                      nullptr, nullptr);

    sead::GraphicsContext context;
    context.setBlendEnable(false);
    context.setDepthEnable(false, false);
    context.setColorMask(0, true, false, false, false);
    context.apply(pDrawContext);

    const agl::ShaderProgram* program = pShaderHolder->getShaderProgram("alMakeLinearDepth");
    const char* macros[] = {"PROJ_TYPE"};
    const char* values[] = {"PROJ_TYPE"};

    if (ShaderSearchImpl::isEqualStr(macros[0], "PROJ_TYPE")) {
        values[0] = al::isMakeLinearDepthProjReverseInfinite() ? "1" : "0";
    }

    program = program->searchVariationShaderProgram(1, macros, values);
    program->activate(pDrawContext, true);

    agl::SamplerLocation location("cDepthBuffer");
    location.search(*program);

    agl::TextureSampler sampler;
    sampler.applyTextureData(*pDepthBuffer);
    sampler.activate(pDrawContext, location, -1, false);
    pTriangle->drawNear(pDrawContext);
}
}  // namespace alViewRendererFunction
