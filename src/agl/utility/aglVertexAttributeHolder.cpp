#include "utility/aglVertexAttributeHolder.h"
#include "utility/aglPrimitiveShape.h"

namespace agl::utl
{

SEAD_SINGLETON_DISPOSER_IMPL(VertexAttributeHolder)

/**
 * Constructs the holder with empty vertex attributes.
 */
VertexAttributeHolder::VertexAttributeHolder() = default;

/**
 * Destroys the vertex attributes.
 */
VertexAttributeHolder::~VertexAttributeHolder() = default;

/**
 * Creates the vertex attributes for the primitive shapes.
 * @param pHeap heap used for the vertex attributes
 */
void VertexAttributeHolder::initialize(sead::Heap* pHeap)
{
    const PrimitiveShape* shape = PrimitiveShape::instance();

    mAttributes[cAttribute_Quad].create(1, pHeap);
    mAttributes[cAttribute_Quad].setVertexStream(0, &shape->getQuadVertexBuffer(), 0);
    mAttributes[cAttribute_Quad].setUp();

    mAttributes[cAttribute_CubeNormal].create(1, pHeap);
    mAttributes[cAttribute_CubeNormal].setVertexStream(0, &shape->getCubeVertexBuffer(), 0);
    mAttributes[cAttribute_CubeNormal].setVertexStream(1, &shape->getCubeVertexBuffer(), 1);
    mAttributes[cAttribute_CubeNormal].setUp();

    mAttributes[cAttribute_ConeNormal].create(1, pHeap);
    mAttributes[cAttribute_ConeNormal].setVertexStream(0, &shape->getConeVertexBuffer(), 0);
    mAttributes[cAttribute_ConeNormal].setVertexStream(1, &shape->getConeVertexBuffer(), 1);
    mAttributes[cAttribute_ConeNormal].setUp();

    mAttributes[cAttribute_CylinderNormal].create(1, pHeap);
    mAttributes[cAttribute_CylinderNormal].setVertexStream(0, &shape->getCylinderVertexBuffer(),
                                                           0);
    mAttributes[cAttribute_CylinderNormal].setVertexStream(1, &shape->getCylinderVertexBuffer(),
                                                           1);
    mAttributes[cAttribute_CylinderNormal].setUp();

    mAttributes[cAttribute_CapsuleNormal].create(1, pHeap);
    mAttributes[cAttribute_CapsuleNormal].setVertexStream(0, &shape->getCapsuleVertexBuffer(), 0);
    mAttributes[cAttribute_CapsuleNormal].setVertexStream(1, &shape->getCapsuleVertexBuffer(), 1);
    mAttributes[cAttribute_CapsuleNormal].setUp();

    mAttributes[cAttribute_SphereNormal].create(1, pHeap);
    mAttributes[cAttribute_SphereNormal].setVertexStream(0, &shape->getSphereVertexBuffer(), 0);
    mAttributes[cAttribute_SphereNormal].setVertexStream(1, &shape->getSphereVertexBuffer(), 1);
    mAttributes[cAttribute_SphereNormal].setUp();

    mAttributes[cAttribute_QuadTriangleTexCoord].create(1, pHeap);
    mAttributes[cAttribute_QuadTriangleTexCoord].setVertexStream(
        0, &shape->getQuadTriangleVertexBuffer(), 0);
    mAttributes[cAttribute_QuadTriangleTexCoord].setVertexStream(
        1, &shape->getQuadTriangleVertexBuffer(), 2);
    mAttributes[cAttribute_QuadTriangleTexCoord].setUp();

    mAttributes[cAttribute_CircleTexCoord].create(1, pHeap);
    mAttributes[cAttribute_CircleTexCoord].setVertexStream(0, &shape->getCircleVertexBuffer(), 0);
    mAttributes[cAttribute_CircleTexCoord].setVertexStream(1, &shape->getCircleVertexBuffer(), 2);
    mAttributes[cAttribute_CircleTexCoord].setUp();

    mAttributes[cAttribute_Cube].create(1, pHeap);
    mAttributes[cAttribute_Cube].setVertexStream(0, &shape->getCubeVertexBuffer(), 0);
    mAttributes[cAttribute_Cube].setUp();

    mAttributes[cAttribute_Sphere].create(1, pHeap);
    mAttributes[cAttribute_Sphere].setVertexStream(0, &shape->getSphereVertexBuffer(), 0);
    mAttributes[cAttribute_Sphere].setUp();

    mAttributes[cAttribute_Pyramid].create(1, pHeap);
    mAttributes[cAttribute_Pyramid].setVertexStream(0, &shape->getPyramidVertexBuffer(), 0);
    mAttributes[cAttribute_Pyramid].setUp();

    mAttributes[cAttribute_PyramidNormal].create(1, pHeap);
    mAttributes[cAttribute_PyramidNormal].setVertexStream(0, &shape->getPyramidVertexBuffer(), 0);
    mAttributes[cAttribute_PyramidNormal].setVertexStream(1, &shape->getPyramidVertexBuffer(), 1);
    mAttributes[cAttribute_PyramidNormal].setUp();
}

}  // namespace agl::utl
