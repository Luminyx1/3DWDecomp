#ifndef SEAD_PRIMITIVE_RENDERER_UTIL_H_
#define SEAD_PRIMITIVE_RENDERER_UTIL_H_

#include <basis/seadTypes.h>
#include <gfx/seadColor.h>
#include <math/seadVector.h>

namespace sead
{
namespace PrimitiveDrawUtil
{
class Vertex
{
public:
    Vertex(const Vector3f& pos, const Vector2f& uv, const Color4f& color)
        : pos(pos), uv(uv), color(color)
    {
    }

    Vector3f pos;
    Vector2f uv;
    Color4f color;
};

void setQuadVertex(Vertex* pVertex, u16* pIndex);
void setLineVertex(Vertex* pVertex, u16* pIndex);
void setCubeVertex(Vertex* pVertex, u16* pIndex);
void setWireCubeVertex(Vertex* pVertex, u16* pIndex);
void setSphereVertex(Vertex* pVertex, u16* pIndex, s32 x, s32 y);
void setDiskVertex(Vertex* pVertex, u16* pIndex, s32 divNum);
void setCylinderVertex(Vertex* pVertex, u16* pIndex, s32 divNum);

}  // namespace PrimitiveDrawUtil

namespace PrimitiveRendererUtil = PrimitiveDrawUtil;
}  // namespace sead

#endif  // SEAD_PRIMITIVE_RENDERER_UTIL_H_
