#pragma once

#include <heap/seadDisposer.h>
#include "common/aglVertexAttribute.h"

namespace agl::utl {

class VertexAttributeHolder {
    SEAD_SINGLETON_DISPOSER(VertexAttributeHolder)

    VertexAttributeHolder();
    virtual ~VertexAttributeHolder();

public:
    enum AttributeType
    {
        cAttribute_Cube,
        cAttribute_CubeNormal,
        cAttribute_ConeNormal,
        cAttribute_CylinderNormal,
        cAttribute_CapsuleNormal,
        cAttribute_Sphere,
        cAttribute_SphereNormal,
        cAttribute_Quad,
        cAttribute_QuadTriangleTexCoord,
        cAttribute_CircleTexCoord,
        cAttribute_Pyramid,
        cAttribute_PyramidNormal,
        cAttribute_Num
    };

    void initialize(sead::Heap* pHeap);

    const VertexAttribute& getVertexAttribute(AttributeType type) const
    {
        return mAttributes[type];
    }

private:
    VertexAttribute mAttributes[cAttribute_Num];
};
static_assert(sizeof(VertexAttributeHolder) == 0x16a8);

}  // namespace agl::utl
