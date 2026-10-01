#pragma once

#include <math/seadVector.h>
#include <nvn/nvn_FuncPtrInline.h>

#include "common/aglDrawContext.h"
#include "common/aglIndexStream.h"
#include "common/aglShaderLocation.h"
#include "common/aglShaderProgram.h"
#include "utility/aglPrimitiveShape.h"
#include "utility/aglVertexAttributeHolder.h"

namespace al {

/**
 * Draws the screen covering quad triangle of the agl primitive shapes.
 */
inline void drawPostProcessingQuad(agl::DrawContext* pContext) {
    agl::utl::VertexAttributeHolder::instance()
        ->getVertexAttribute(agl::utl::VertexAttributeHolder::cAttribute_QuadTriangleTexCoord)
        .activate(pContext);
    const agl::IndexStream& rStream =
        agl::utl::PrimitiveShape::instance()->getQuadTriangleIndexStream();
    u32 count = rStream.getCount();

    if (count != 0) {
        NVNdrawPrimitive primitive = rStream.getPrimitiveType();
        NVNcommandBuffer* pCommandBuffer = pContext->getNvnCommandBuffer();
        NVNbufferAddress address = nvnBufferGetAddress(rStream.getNvnBuffer());
        nvnCommandBufferDrawElements(pCommandBuffer, primitive,
                                     NVNindexType(rStream.getFormat()), count, address);
    }
}

/**
 * Creates a shader location of the given name and searches it in a program.
 */
template <typename T>
inline T searchShaderLocation(const agl::ShaderProgram& rProgram, const char* pName) {
    T location(pName);
    location.search(rProgram);
    return location;
}

/**
 * Searches a uniform by name and sets a four component value to it.
 */
inline void setPostProcessingUniform(agl::DrawContext* pContext,
                                     const agl::ShaderProgram* pProgram, const char* pName,
                                     const sead::Vector4f& rValue) {
    agl::UniformLocation location(pName);
    location.search(*pProgram);
    location.setUniform(pContext, 4, &rValue);
}

/**
 * Searches a uniform by name and sets a scalar value to it.
 */
inline void setPostProcessingUniform(agl::DrawContext* pContext,
                                     const agl::ShaderProgram* pProgram, const char* pName,
                                     f32 value) {
    agl::UniformLocation location(pName);
    location.search(*pProgram);
    location.setUniform(pContext, value);
}

}  // namespace al
