#include "gfx/seadPrimitiveRenderer.h"

#include "gfx/seadCamera.h"
#include "gfx/seadProjection.h"

namespace sead
{
SEAD_SINGLETON_DISPOSER_IMPL(PrimitiveRenderer)

/**
 * Constructs the primitive renderer with identity matrices.
 */
PrimitiveRenderer::PrimitiveRenderer() = default;

/**
 * Destroys the primitive renderer.
 */
PrimitiveRenderer::~PrimitiveRenderer() = default;

/**
 * Prepares the draw manager from a shader binary.
 * @param pHeap heap to allocate from
 * @param pBinary shader binary
 * @param binarySize size of the shader binary
 */
void PrimitiveRenderer::prepareFromBinary(Heap* pHeap, const void* pBinary, u32 binarySize)
{
    PrimitiveDrawer::prepareMgrFromBinary(pHeap, pBinary, binarySize);
}

/**
 * Prepares the draw manager from a shader file.
 * @param pHeap heap to allocate from
 * @param rPath path of the shader binary
 */
void PrimitiveRenderer::prepare(Heap* pHeap, const SafeString& rPath)
{
    PrimitiveDrawer::prepareMgr(pHeap, rPath);
}

/**
 * Copies the view matrix of a camera.
 * @param rCamera camera
 */
void PrimitiveRenderer::setCamera(const Camera& rCamera)
{
    mViewMatrix = rCamera.getMatrix();
    mDrawer.setCameraViewMatrix(&mViewMatrix);
}

/**
 * Copies the device projection matrix of a projection.
 * @param rProjection projection
 */
void PrimitiveRenderer::setProjection(const Projection& rProjection)
{
    mProjectionMatrix = rProjection.getDeviceProjectionMatrix();
    mDrawer.setProjectionMatrix(&mProjectionMatrix);
}

/**
 * Copies the model matrix.
 * @param rModelMatrix model matrix
 */
void PrimitiveRenderer::setModelMatrix(const Matrix34f& rModelMatrix)
{
    mModelMatrix = rModelMatrix;
    mDrawer.setModelMatrix(&mModelMatrix);
}

/**
 * Begins drawing.
 */
void PrimitiveRenderer::begin()
{
    mDrawer.begin();
}

/**
 * Ends drawing.
 */
void PrimitiveRenderer::end()
{
    mDrawer.end();
}

/**
 * Draws a unit quad.
 * @param rColor0 first color
 * @param rColor1 second color
 */
void PrimitiveRenderer::drawQuad(const Color4f& rColor0, const Color4f& rColor1)
{
    mDrawer.drawQuad(rColor0, rColor1);
}

/**
 * Draws a textured unit quad.
 * @param rTexture texture
 * @param rColor0 first color
 * @param rColor1 second color
 * @param rUVSrc texture coordinate origin
 * @param rUVSize texture coordinate size
 */
void PrimitiveRenderer::drawQuad(const Texture& rTexture, const Color4f& rColor0, const Color4f& rColor1,
                                 const Vector2f& rUVSrc, const Vector2f& rUVSize)
{
    mDrawer.drawQuad(rTexture, rColor0, rColor1, rUVSrc, rUVSize);
}

/**
 * Draws a unit quad outline.
 * @param rColor0 first color
 * @param rColor1 second color
 */
void PrimitiveRenderer::drawBox(const Color4f& rColor0, const Color4f& rColor1)
{
    mDrawer.drawBox(rColor0, rColor1);
}

/**
 * Draws a unit cube.
 * @param rColor0 first color
 * @param rColor1 second color
 */
void PrimitiveRenderer::drawCube(const Color4f& rColor0, const Color4f& rColor1)
{
    mDrawer.drawCube(rColor0, rColor1);
}

/**
 * Draws a unit wireframe cube.
 * @param rColor0 first color
 * @param rColor1 second color
 */
void PrimitiveRenderer::drawWireCube(const Color4f& rColor0, const Color4f& rColor1)
{
    mDrawer.drawWireCube(rColor0, rColor1);
}

/**
 * Draws a unit line.
 * @param rColor0 start color
 * @param rColor1 end color
 */
void PrimitiveRenderer::drawLine(const Color4f& rColor0, const Color4f& rColor1)
{
    mDrawer.drawLine(rColor0, rColor1);
}

/**
 * Draws a unit sphere with 4x8 divisions.
 * @param rNorth north pole color
 * @param rSouth south pole color
 */
void PrimitiveRenderer::drawSphere4x8(const Color4f& rNorth, const Color4f& rSouth)
{
    mDrawer.drawSphere4x8(rNorth, rSouth);
}

/**
 * Draws a unit sphere with 8x16 divisions.
 * @param rNorth north pole color
 * @param rSouth south pole color
 */
void PrimitiveRenderer::drawSphere8x16(const Color4f& rNorth, const Color4f& rSouth)
{
    mDrawer.drawSphere8x16(rNorth, rSouth);
}

/**
 * Draws a unit disk with 16 divisions.
 * @param rCenter center color
 * @param rEdge edge color
 */
void PrimitiveRenderer::drawDisk16(const Color4f& rCenter, const Color4f& rEdge)
{
    mDrawer.drawDisk16(rCenter, rEdge);
}

/**
 * Draws a unit disk with 32 divisions.
 * @param rCenter center color
 * @param rEdge edge color
 */
void PrimitiveRenderer::drawDisk32(const Color4f& rCenter, const Color4f& rEdge)
{
    mDrawer.drawDisk32(rCenter, rEdge);
}

/**
 * Draws a unit circle with 16 divisions.
 * @param rEdge edge color
 */
void PrimitiveRenderer::drawCircle16(const Color4f& rEdge)
{
    mDrawer.drawCircle16(rEdge);
}

/**
 * Draws a unit circle with 32 divisions.
 * @param rEdge edge color
 */
void PrimitiveRenderer::drawCircle32(const Color4f& rEdge)
{
    mDrawer.drawCircle32(rEdge);
}

/**
 * Draws a unit cylinder with 16 divisions.
 * @param rTop top color
 * @param rBottom bottom color
 */
void PrimitiveRenderer::drawCylinder16(const Color4f& rTop, const Color4f& rBottom)
{
    mDrawer.drawCylinder16(rTop, rBottom);
}

/**
 * Draws a unit cylinder with 32 divisions.
 * @param rTop top color
 * @param rBottom bottom color
 */
void PrimitiveRenderer::drawCylinder32(const Color4f& rTop, const Color4f& rBottom)
{
    mDrawer.drawCylinder32(rTop, rBottom);
}

/**
 * Draws a quad described by a QuadArg.
 * @param rArg quad description
 */
void PrimitiveRenderer::drawQuad(const PrimitiveDrawer::QuadArg& rArg)
{
    mDrawer.drawQuad(rArg);
}

/**
 * Draws a textured quad described by a QuadArg.
 * @param rTexture texture
 * @param rArg quad description
 * @param rUVArg texture coordinates
 */
void PrimitiveRenderer::drawQuad(const Texture& rTexture, const PrimitiveDrawer::QuadArg& rArg,
                                 const PrimitiveDrawer::UVArg& rUVArg)
{
    mDrawer.drawQuad(rTexture, rArg, rUVArg);
}

/**
 * Draws a quad outline described by a QuadArg.
 * @param rArg quad description
 */
void PrimitiveRenderer::drawBox(const PrimitiveDrawer::QuadArg& rArg)
{
    mDrawer.drawBox(rArg);
}

/**
 * Draws a cube described by a CubeArg.
 * @param rArg cube description
 */
void PrimitiveRenderer::drawCube(const PrimitiveDrawer::CubeArg& rArg)
{
    mDrawer.drawCube(rArg);
}

/**
 * Draws a wireframe cube described by a CubeArg.
 * @param rArg cube description
 */
void PrimitiveRenderer::drawWireCube(const PrimitiveDrawer::CubeArg& rArg)
{
    mDrawer.drawWireCube(rArg);
}

/**
 * Draws a line between two points.
 * @param rFrom start position
 * @param rTo end position
 * @param rColor0 start color
 * @param rColor1 end color
 */
void PrimitiveRenderer::drawLine(const Vector3f& rFrom, const Vector3f& rTo, const Color4f& rColor0,
                                 const Color4f& rColor1)
{
    mDrawer.drawLine(rFrom, rTo, rColor0, rColor1);
}

/**
 * Draws a single colored line between two points.
 * @param rFrom start position
 * @param rTo end position
 * @param rColor line color
 */
void PrimitiveRenderer::drawLine(const Vector3f& rFrom, const Vector3f& rTo, const Color4f& rColor)
{
    mDrawer.drawLine(rFrom, rTo, rColor);
}

/**
 * Draws a sphere with 4x8 divisions.
 * @param rPos center position
 * @param radius radius
 * @param rColor0 first color
 * @param rColor1 second color
 */
void PrimitiveRenderer::drawSphere4x8(const Vector3f& rPos, f32 radius, const Color4f& rColor0,
                                 const Color4f& rColor1)
{
    mDrawer.drawSphere4x8(rPos, radius, rColor0, rColor1);
}

/**
 * Draws a single colored sphere with 4x8 divisions.
 * @param rPos center position
 * @param radius radius
 * @param rColor color
 */
void PrimitiveRenderer::drawSphere4x8(const Vector3f& rPos, f32 radius, const Color4f& rColor)
{
    mDrawer.drawSphere4x8(rPos, radius, rColor);
}

/**
 * Draws a sphere with 8x16 divisions.
 * @param rPos center position
 * @param radius radius
 * @param rColor0 first color
 * @param rColor1 second color
 */
void PrimitiveRenderer::drawSphere8x16(const Vector3f& rPos, f32 radius, const Color4f& rColor0,
                                 const Color4f& rColor1)
{
    mDrawer.drawSphere8x16(rPos, radius, rColor0, rColor1);
}

/**
 * Draws a single colored sphere with 8x16 divisions.
 * @param rPos center position
 * @param radius radius
 * @param rColor color
 */
void PrimitiveRenderer::drawSphere8x16(const Vector3f& rPos, f32 radius, const Color4f& rColor)
{
    mDrawer.drawSphere8x16(rPos, radius, rColor);
}

/**
 * Draws a disk with 16 divisions.
 * @param rPos center position
 * @param radius radius
 * @param rColor0 first color
 * @param rColor1 second color
 */
void PrimitiveRenderer::drawDisk16(const Vector3f& rPos, f32 radius, const Color4f& rColor0,
                                 const Color4f& rColor1)
{
    mDrawer.drawDisk16(rPos, radius, rColor0, rColor1);
}

/**
 * Draws a single colored disk with 16 divisions.
 * @param rPos center position
 * @param radius radius
 * @param rColor color
 */
void PrimitiveRenderer::drawDisk16(const Vector3f& rPos, f32 radius, const Color4f& rColor)
{
    mDrawer.drawDisk16(rPos, radius, rColor);
}

/**
 * Draws a disk with 32 divisions.
 * @param rPos center position
 * @param radius radius
 * @param rColor0 first color
 * @param rColor1 second color
 */
void PrimitiveRenderer::drawDisk32(const Vector3f& rPos, f32 radius, const Color4f& rColor0,
                                 const Color4f& rColor1)
{
    mDrawer.drawDisk32(rPos, radius, rColor0, rColor1);
}

/**
 * Draws a single colored disk with 32 divisions.
 * @param rPos center position
 * @param radius radius
 * @param rColor color
 */
void PrimitiveRenderer::drawDisk32(const Vector3f& rPos, f32 radius, const Color4f& rColor)
{
    mDrawer.drawDisk32(rPos, radius, rColor);
}

/**
 * Draws a circle with 16 divisions.
 * @param rPos center position
 * @param radius radius
 * @param rColor color
 */
void PrimitiveRenderer::drawCircle16(const Vector3f& rPos, f32 radius, const Color4f& rColor)
{
    mDrawer.drawCircle16(rPos, radius, rColor);
}

/**
 * Draws a circle with 32 divisions.
 * @param rPos center position
 * @param radius radius
 * @param rColor color
 */
void PrimitiveRenderer::drawCircle32(const Vector3f& rPos, f32 radius, const Color4f& rColor)
{
    mDrawer.drawCircle32(rPos, radius, rColor);
}

/**
 * Draws a cylinder with 16 divisions.
 * @param rPos center position
 * @param radius radius
 * @param height height
 * @param rTop top color
 * @param rBottom bottom color
 */
void PrimitiveRenderer::drawCylinder16(const Vector3f& rPos, f32 radius, f32 height,
                                 const Color4f& rTop, const Color4f& rBottom)
{
    mDrawer.drawCylinder16(rPos, radius, height, rTop, rBottom);
}

/**
 * Draws a single colored cylinder with 16 divisions.
 * @param rPos center position
 * @param radius radius
 * @param height height
 * @param rColor color
 */
void PrimitiveRenderer::drawCylinder16(const Vector3f& rPos, f32 radius, f32 height,
                                 const Color4f& rColor)
{
    mDrawer.drawCylinder16(rPos, radius, height, rColor);
}

/**
 * Draws a cylinder with 32 divisions.
 * @param rPos center position
 * @param radius radius
 * @param height height
 * @param rTop top color
 * @param rBottom bottom color
 */
void PrimitiveRenderer::drawCylinder32(const Vector3f& rPos, f32 radius, f32 height,
                                 const Color4f& rTop, const Color4f& rBottom)
{
    mDrawer.drawCylinder32(rPos, radius, height, rTop, rBottom);
}

/**
 * Draws a single colored cylinder with 32 divisions.
 * @param rPos center position
 * @param radius radius
 * @param height height
 * @param rColor color
 */
void PrimitiveRenderer::drawCylinder32(const Vector3f& rPos, f32 radius, f32 height,
                                 const Color4f& rColor)
{
    mDrawer.drawCylinder32(rPos, radius, height, rColor);
}

/**
 * Draws the X, Y and Z axes.
 * @param rPos origin
 * @param scale axis length
 */
void PrimitiveRenderer::drawAxis(const Vector3f& rPos, f32 scale)
{
    mDrawer.drawAxis(rPos, scale);
}

}  // namespace sead
