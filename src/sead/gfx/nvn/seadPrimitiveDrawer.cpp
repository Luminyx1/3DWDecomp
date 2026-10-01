#include "gfx/seadPrimitiveRenderer.h"

#include <cmath>
#include "gfx/nvn/seadPrimitiveDrawMgrNvn.h"
#include "gfx/seadCamera.h"
#include "gfx/seadPrimitiveRendererUtil.h"
#include "gfx/seadProjection.h"
#include "math/seadMathNumbers.h"
#include "math/seadMatrixCalcCommon.h"
#include "math/seadQuat.h"
#include "prim/seadMemUtil.h"

namespace sead
{
/**
 * Constructs a drawer using identity matrices.
 * @param pDrawContext draw context
 */
PrimitiveDrawer::PrimitiveDrawer(DrawContext* pDrawContext)
    : mModelMatrix(&Matrix34f::ident), mViewMatrix(&Matrix34f::ident),
      mProjectionMatrix(&Matrix44f::ident), mDrawContext(pDrawContext)
{
}

/**
 * Destroys the drawer.
 */
PrimitiveDrawer::~PrimitiveDrawer() = default;

/**
 * Creates the draw manager and prepares it from a shader binary.
 * @param pHeap heap to allocate from
 * @param pBinary shader binary
 * @param binarySize size of the shader binary
 */
void PrimitiveDrawer::prepareMgrFromBinary(Heap* pHeap, const void* pBinary, u32 binarySize)
{
    createDrawMgrInstance_(pHeap)->prepareFromBinaryImpl(pHeap, pBinary, binarySize);
}

/**
 * Creates the draw manager singleton.
 * @param pHeap heap to allocate from
 * @return the draw manager
 */
PrimitiveDrawMgrNvn* PrimitiveDrawer::createDrawMgrInstance_(Heap* pHeap)
{
    return PrimitiveDrawMgrNvn::createInstance(pHeap);
}

/**
 * Creates the draw manager and prepares it from a shader file.
 * @param pHeap heap to allocate from
 * @param rPath path of the shader binary
 */
void PrimitiveDrawer::prepareMgr(Heap* pHeap, const SafeString& rPath)
{
    createDrawMgrInstance_(pHeap)->prepareImpl(pHeap, rPath);
}

/**
 * Creates the draw manager with custom settings and prepares it from a shader binary.
 * @param pHeap heap to allocate from
 * @param pBinary shader binary
 * @param binarySize size of the shader binary
 * @param uniformBufferSize size of the uniform buffer
 * @param isTextureEnable whether textured drawing is supported
 */
void PrimitiveDrawer::prepareMgrFromBinary(Heap* pHeap, const void* pBinary, u32 binarySize,
                                           u32 uniformBufferSize, bool isTextureEnable)
{
    PrimitiveDrawMgrNvn* mgr = createDrawMgrInstance_(pHeap);
    mgr->setTextureEnable(isTextureEnable);
    mgr->setUniformBufferSize(uniformBufferSize);
    mgr->prepareFromBinaryImpl(pHeap, pBinary, binarySize);
}

/**
 * Creates the draw manager with custom settings and prepares it from a shader file.
 * @param pHeap heap to allocate from
 * @param rPath path of the shader binary
 * @param uniformBufferSize size of the uniform buffer
 * @param isTextureEnable whether textured drawing is supported
 */
void PrimitiveDrawer::prepareMgr(Heap* pHeap, const SafeString& rPath, u32 uniformBufferSize,
                                 bool isTextureEnable)
{
    PrimitiveDrawMgrNvn* mgr = createDrawMgrInstance_(pHeap);
    mgr->setTextureEnable(isTextureEnable);
    mgr->setUniformBufferSize(uniformBufferSize);
    mgr->prepareImpl(pHeap, rPath);
}

/**
 * Uses the view matrix of a camera.
 * @param pCamera camera
 */
void PrimitiveDrawer::setCamera(const Camera* pCamera)
{
    mViewMatrix = &pCamera->getMatrix();
}

/**
 * Sets the view matrix.
 * @param pViewMatrix view matrix
 */
void PrimitiveDrawer::setCameraViewMatrix(const Matrix34f* pViewMatrix)
{
    mViewMatrix = pViewMatrix;
}

/**
 * Uses the device projection matrix of a projection.
 * @param pProjection projection
 */
void PrimitiveDrawer::setProjection(const Projection* pProjection)
{
    mProjectionMatrix = &pProjection->getDeviceProjectionMatrix();
}

/**
 * Sets the projection matrix.
 * @param pProjectionMatrix projection matrix
 */
void PrimitiveDrawer::setProjectionMatrix(const Matrix44f* pProjectionMatrix)
{
    mProjectionMatrix = pProjectionMatrix;
}

/**
 * Sets the model matrix.
 * @param pModelMatrix model matrix
 */
void PrimitiveDrawer::setModelMatrix(const Matrix34f* pModelMatrix)
{
    mModelMatrix = pModelMatrix;
}

/**
 * Sets the draw context.
 * @param pDrawContext draw context
 */
void PrimitiveDrawer::setDrawContext(DrawContext* pDrawContext)
{
    mDrawContext = pDrawContext;
}

/**
 * Begins drawing with the current view and projection matrices.
 */
void PrimitiveDrawer::begin()
{
    getDrawMgr_()->beginImpl(mDrawContext, *mViewMatrix, *mProjectionMatrix);
}

/**
 * Gets the draw manager.
 * @return the draw manager
 */
PrimitiveDrawMgrNvn* PrimitiveDrawer::getDrawMgr_()
{
    return PrimitiveDrawMgrNvn::instance();
}

/**
 * Ends drawing.
 */
void PrimitiveDrawer::end()
{
    getDrawMgr_()->endImpl(mDrawContext);
}

/**
 * Draws a unit quad with the model matrix.
 * @param rColor0 first color
 * @param rColor1 second color
 */
void PrimitiveDrawer::drawQuad(const Color4f& rColor0, const Color4f& rColor1)
{
    getDrawMgr_()->drawQuadImpl(mDrawContext, *mModelMatrix, rColor0, rColor1);
}

/**
 * Draws a textured unit quad with the model matrix.
 * @param rTexture texture
 * @param rColor0 first color
 * @param rColor1 second color
 * @param rUVSrc texture coordinate origin
 * @param rUVSize texture coordinate size
 */
void PrimitiveDrawer::drawQuad(const Texture& rTexture, const Color4f& rColor0,
                               const Color4f& rColor1, const Vector2f& rUVSrc,
                               const Vector2f& rUVSize)
{
    getDrawMgr_()->drawQuadImpl(mDrawContext, *mModelMatrix, rTexture, rColor0, rColor1, rUVSrc,
                                rUVSize);
}

/**
 * Draws a unit quad outline with the model matrix.
 * @param rColor0 first color
 * @param rColor1 second color
 */
void PrimitiveDrawer::drawBox(const Color4f& rColor0, const Color4f& rColor1)
{
    getDrawMgr_()->drawBoxImpl(mDrawContext, *mModelMatrix, rColor0, rColor1);
}

/**
 * Draws a unit cube with the model matrix.
 * @param rColor0 first color
 * @param rColor1 second color
 */
void PrimitiveDrawer::drawCube(const Color4f& rColor0, const Color4f& rColor1)
{
    getDrawMgr_()->drawCubeImpl(mDrawContext, *mModelMatrix, rColor0, rColor1);
}

/**
 * Draws a unit wireframe cube with the model matrix.
 * @param rColor0 first color
 * @param rColor1 second color
 */
void PrimitiveDrawer::drawWireCube(const Color4f& rColor0, const Color4f& rColor1)
{
    getDrawMgr_()->drawWireCubeImpl(mDrawContext, *mModelMatrix, rColor0, rColor1);
}

/**
 * Draws a unit line with the model matrix.
 * @param rColor0 start color
 * @param rColor1 end color
 */
void PrimitiveDrawer::drawLine(const Color4f& rColor0, const Color4f& rColor1)
{
    getDrawMgr_()->drawLineImpl(mDrawContext, *mModelMatrix, rColor0, rColor1);
}

/**
 * Draws a unit sphere with 4x8 divisions with the model matrix.
 * @param rNorth north pole color
 * @param rSouth south pole color
 */
void PrimitiveDrawer::drawSphere4x8(const Color4f& rNorth, const Color4f& rSouth)
{
    getDrawMgr_()->drawSphere4x8Impl(mDrawContext, *mModelMatrix, rNorth, rSouth);
}

/**
 * Draws a unit sphere with 8x16 divisions with the model matrix.
 * @param rNorth north pole color
 * @param rSouth south pole color
 */
void PrimitiveDrawer::drawSphere8x16(const Color4f& rNorth, const Color4f& rSouth)
{
    getDrawMgr_()->drawSphere8x16Impl(mDrawContext, *mModelMatrix, rNorth, rSouth);
}

/**
 * Draws a unit disk with 16 divisions with the model matrix.
 * @param rCenter center color
 * @param rEdge edge color
 */
void PrimitiveDrawer::drawDisk16(const Color4f& rCenter, const Color4f& rEdge)
{
    getDrawMgr_()->drawDisk16Impl(mDrawContext, *mModelMatrix, rCenter, rEdge);
}

/**
 * Draws a unit disk with 32 divisions with the model matrix.
 * @param rCenter center color
 * @param rEdge edge color
 */
void PrimitiveDrawer::drawDisk32(const Color4f& rCenter, const Color4f& rEdge)
{
    getDrawMgr_()->drawDisk32Impl(mDrawContext, *mModelMatrix, rCenter, rEdge);
}

/**
 * Draws a unit circle with 16 divisions with the model matrix.
 * @param rEdge edge color
 */
void PrimitiveDrawer::drawCircle16(const Color4f& rEdge)
{
    getDrawMgr_()->drawCircle16Impl(mDrawContext, *mModelMatrix, rEdge);
}

/**
 * Draws a unit circle with 32 divisions with the model matrix.
 * @param rEdge edge color
 */
void PrimitiveDrawer::drawCircle32(const Color4f& rEdge)
{
    getDrawMgr_()->drawCircle32Impl(mDrawContext, *mModelMatrix, rEdge);
}

/**
 * Draws a unit cylinder with 16 divisions with the model matrix.
 * @param rTop top color
 * @param rBottom bottom color
 */
void PrimitiveDrawer::drawCylinder16(const Color4f& rTop, const Color4f& rBottom)
{
    getDrawMgr_()->drawCylinder16Impl(mDrawContext, *mModelMatrix, rTop, rBottom);
}

/**
 * Draws a unit cylinder with 32 divisions with the model matrix.
 * @param rTop top color
 * @param rBottom bottom color
 */
void PrimitiveDrawer::drawCylinder32(const Color4f& rTop, const Color4f& rBottom)
{
    getDrawMgr_()->drawCylinder32Impl(mDrawContext, *mModelMatrix, rTop, rBottom);
}

/**
 * Sets the quad from its corner and size.
 * @param rCorner corner position
 * @param rSize size
 */
void PrimitiveDrawer::QuadArg::setCornerAndSize(const Vector3f& rCorner, const Vector2f& rSize)
{
    mCenter.set(rSize.x * 0.5f + rCorner.x, rSize.y * 0.5f + rCorner.y, rCorner.z);
    mSize = rSize;
}

/**
 * Sets the quad from a bounding box.
 * @param rBox bounding box
 * @param z depth of the quad
 * @return this argument
 */
PrimitiveDrawer::QuadArg& PrimitiveDrawer::QuadArg::setBoundBox(const BoundBox2f& rBox, f32 z)
{
    Vector2f center = rBox.getCenter();
    mCenter.set(center.x, center.y, z);
    mSize.set(rBox.getSizeX(), rBox.getSizeY());
    return *this;
}

/**
 * Sets a vertical gradient.
 * @param rColor0 top color
 * @param rColor1 bottom color
 * @return this argument
 */
PrimitiveDrawer::QuadArg& PrimitiveDrawer::QuadArg::setColor(const Color4f& rColor0,
                                                             const Color4f& rColor1)
{
    mIsHorizontal = false;
    mColor0 = rColor0;
    mColor1 = rColor1;
    return *this;
}

/**
 * Sets a horizontal gradient.
 * @param rColor0 left color
 * @param rColor1 right color
 */
void PrimitiveDrawer::QuadArg::setColorHorizontal(const Color4f& rColor0, const Color4f& rColor1)
{
    mIsHorizontal = true;
    mColor0 = rColor0;
    mColor1 = rColor1;
}

/**
 * Sets the cube from its corner and size.
 * @param rCorner corner position
 * @param rSize size
 */
void PrimitiveDrawer::CubeArg::setCornerAndSize(const Vector3f& rCorner, const Vector3f& rSize)
{
    mCenter.setScaleAdd(0.5f, rSize, rCorner);
    mSize = rSize;
}

/**
 * Sets the cube from a bounding box.
 * @param rBox bounding box
 */
void PrimitiveDrawer::CubeArg::setBoundBox(const BoundBox3f& rBox)
{
    mCenter = rBox.getCenter();
    mSize.set(rBox.getSizeX(), rBox.getSizeY(), rBox.getSizeZ());
}

namespace
{
inline void makeQuadMatrix(Matrix34f* pMatrix, const PrimitiveDrawer::QuadArg& rArg)
{
    if (rArg.isHorizontal())
    {
        pMatrix->makeSRT(Vector3f(rArg.getSize().y, rArg.getSize().x, 1.0f),
                         Vector3f(0.0f, 0.0f, numbers::pi / 2), rArg.getCenter());
    }
    else
    {
        pMatrix->makeST(Vector3f(rArg.getSize().x, rArg.getSize().y, 1.0f), rArg.getCenter());
    }
}
}  // namespace

/**
 * Draws a quad described by a QuadArg.
 * @param rArg quad description
 */
void PrimitiveDrawer::drawQuad(const QuadArg& rArg)
{
    Matrix34f local;
    makeQuadMatrix(&local, rArg);
    Matrix34f mtx;
    mtx.setMul(*mModelMatrix, local);
    getDrawMgr_()->drawQuadImpl(mDrawContext, mtx, rArg.getColor0(), rArg.getColor1());
}

/**
 * Draws a textured quad described by a QuadArg.
 * @param rTexture texture
 * @param rArg quad description
 * @param rUVArg texture coordinates
 */
void PrimitiveDrawer::drawQuad(const Texture& rTexture, const QuadArg& rArg, const UVArg& rUVArg)
{
    Matrix34f local;
    makeQuadMatrix(&local, rArg);
    Matrix34f mtx;
    mtx.setMul(*mModelMatrix, local);
    getDrawMgr_()->drawQuadImpl(mDrawContext, mtx, rTexture, rArg.getColor0(), rArg.getColor1(),
                                rUVArg.getUVSrc(), rUVArg.getUVSize());
}

/**
 * Draws a quad outline described by a QuadArg.
 * @param rArg quad description
 */
void PrimitiveDrawer::drawBox(const QuadArg& rArg)
{
    Matrix34f local;
    makeQuadMatrix(&local, rArg);
    Matrix34f mtx;
    mtx.setMul(*mModelMatrix, local);
    getDrawMgr_()->drawBoxImpl(mDrawContext, mtx, rArg.getColor0(), rArg.getColor1());
}

/**
 * Draws a cube described by a CubeArg.
 * @param rArg cube description
 */
void PrimitiveDrawer::drawCube(const CubeArg& rArg)
{
    Matrix34f local;
    local.makeST(rArg.getSize(), rArg.getCenter());
    Matrix34f mtx;
    mtx.setMul(*mModelMatrix, local);
    getDrawMgr_()->drawCubeImpl(mDrawContext, mtx, rArg.getColor0(), rArg.getColor1());
}

/**
 * Draws a wireframe cube described by a CubeArg.
 * @param rArg cube description
 */
void PrimitiveDrawer::drawWireCube(const CubeArg& rArg)
{
    Matrix34f local;
    local.makeST(rArg.getSize(), rArg.getCenter());
    Matrix34f mtx;
    mtx.setMul(*mModelMatrix, local);
    getDrawMgr_()->drawWireCubeImpl(mDrawContext, mtx, rArg.getColor0(), rArg.getColor1());
}

/**
 * Draws a line between two points.
 * @param rFrom start position
 * @param rTo end position
 * @param rColor0 start color
 * @param rColor1 end color
 */
void PrimitiveDrawer::drawLine(const Vector3f& rFrom, const Vector3f& rTo, const Color4f& rColor0,
                               const Color4f& rColor1)
{
    Vector3f dir = rTo - rFrom;
    Matrix34f scale;
    scale.makeS(dir.length(), 1.0f, 1.0f);
    dir.normalize();

    Quatf rotation;
    rotation.makeVectorRotation(Vector3f::ex, dir);
    Matrix34f local;
    local.fromQuat(rotation);
    local.setMul(local, scale);
    local.setTranslation(rFrom + (rTo - rFrom) * 0.5f);

    Matrix34f mtx;
    mtx.setMul(*mModelMatrix, local);
    getDrawMgr_()->drawLineImpl(mDrawContext, mtx, rColor0, rColor1);
}

/**
 * Draws a single colored line between two points.
 * @param rFrom start position
 * @param rTo end position
 * @param rColor line color
 */
void PrimitiveDrawer::drawLine(const Vector3f& rFrom, const Vector3f& rTo, const Color4f& rColor)
{
    drawLine(rFrom, rTo, rColor, rColor);
}

/**
 * Draws a sphere with 4x8 divisions.
 * @param rPos center position
 * @param radius radius
 * @param rNorth north pole color
 * @param rSouth south pole color
 */
void PrimitiveDrawer::drawSphere4x8(const Vector3f& rPos, f32 radius, const Color4f& rNorth,
                                    const Color4f& rSouth)
{
    Matrix34f local;
    local.makeST(Vector3f(radius * 2, radius * 2, radius * 2), rPos);
    Matrix34f mtx;
    mtx.setMul(*mModelMatrix, local);
    getDrawMgr_()->drawSphere4x8Impl(mDrawContext, mtx, rNorth, rSouth);
}

/**
 * Draws a single colored sphere with 4x8 divisions.
 * @param rPos center position
 * @param radius radius
 * @param rColor sphere color
 */
void PrimitiveDrawer::drawSphere4x8(const Vector3f& rPos, f32 radius, const Color4f& rColor)
{
    drawSphere4x8(rPos, radius, rColor, rColor);
}

/**
 * Draws a sphere with 8x16 divisions.
 * @param rPos center position
 * @param radius radius
 * @param rNorth north pole color
 * @param rSouth south pole color
 */
void PrimitiveDrawer::drawSphere8x16(const Vector3f& rPos, f32 radius, const Color4f& rNorth,
                                     const Color4f& rSouth)
{
    Matrix34f local;
    local.makeST(Vector3f(radius * 2, radius * 2, radius * 2), rPos);
    Matrix34f mtx;
    mtx.setMul(*mModelMatrix, local);
    getDrawMgr_()->drawSphere8x16Impl(mDrawContext, mtx, rNorth, rSouth);
}

/**
 * Draws a single colored sphere with 8x16 divisions.
 * @param rPos center position
 * @param radius radius
 * @param rColor sphere color
 */
void PrimitiveDrawer::drawSphere8x16(const Vector3f& rPos, f32 radius, const Color4f& rColor)
{
    drawSphere8x16(rPos, radius, rColor, rColor);
}

/**
 * Draws a disk with 16 divisions.
 * @param rPos center position
 * @param radius radius
 * @param rCenter center color
 * @param rEdge edge color
 */
void PrimitiveDrawer::drawDisk16(const Vector3f& rPos, f32 radius, const Color4f& rCenter,
                                 const Color4f& rEdge)
{
    Matrix34f local;
    local.makeST(Vector3f(radius * 2, radius * 2, radius * 2), rPos);
    Matrix34f mtx;
    mtx.setMul(*mModelMatrix, local);
    getDrawMgr_()->drawDisk16Impl(mDrawContext, mtx, rCenter, rEdge);
}

/**
 * Draws a single colored disk with 16 divisions.
 * @param rPos center position
 * @param radius radius
 * @param rColor disk color
 */
void PrimitiveDrawer::drawDisk16(const Vector3f& rPos, f32 radius, const Color4f& rColor)
{
    drawDisk16(rPos, radius, rColor, rColor);
}

/**
 * Draws a disk with 32 divisions.
 * @param rPos center position
 * @param radius radius
 * @param rCenter center color
 * @param rEdge edge color
 */
void PrimitiveDrawer::drawDisk32(const Vector3f& rPos, f32 radius, const Color4f& rCenter,
                                 const Color4f& rEdge)
{
    Matrix34f local;
    local.makeST(Vector3f(radius * 2, radius * 2, radius * 2), rPos);
    Matrix34f mtx;
    mtx.setMul(*mModelMatrix, local);
    getDrawMgr_()->drawDisk32Impl(mDrawContext, mtx, rCenter, rEdge);
}

/**
 * Draws a single colored disk with 32 divisions.
 * @param rPos center position
 * @param radius radius
 * @param rColor disk color
 */
void PrimitiveDrawer::drawDisk32(const Vector3f& rPos, f32 radius, const Color4f& rColor)
{
    drawDisk32(rPos, radius, rColor, rColor);
}

/**
 * Draws a circle with 16 divisions.
 * @param rPos center position
 * @param radius radius
 * @param rColor circle color
 */
void PrimitiveDrawer::drawCircle16(const Vector3f& rPos, f32 radius, const Color4f& rColor)
{
    Matrix34f local;
    local.makeST(Vector3f(radius * 2, radius * 2, radius * 2), rPos);
    Matrix34f mtx;
    mtx.setMul(*mModelMatrix, local);
    getDrawMgr_()->drawCircle16Impl(mDrawContext, mtx, rColor);
}

/**
 * Draws a circle with 32 divisions.
 * @param rPos center position
 * @param radius radius
 * @param rColor circle color
 */
void PrimitiveDrawer::drawCircle32(const Vector3f& rPos, f32 radius, const Color4f& rColor)
{
    Matrix34f local;
    local.makeST(Vector3f(radius * 2, radius * 2, radius * 2), rPos);
    Matrix34f mtx;
    mtx.setMul(*mModelMatrix, local);
    getDrawMgr_()->drawCircle32Impl(mDrawContext, mtx, rColor);
}

/**
 * Draws a cylinder with 16 divisions.
 * @param rPos center position
 * @param radius radius
 * @param height height
 * @param rTop top color
 * @param rBottom bottom color
 */
void PrimitiveDrawer::drawCylinder16(const Vector3f& rPos, f32 radius, f32 height,
                                     const Color4f& rTop, const Color4f& rBottom)
{
    Matrix34f local;
    local.makeST(Vector3f(radius * 2, height, radius * 2), rPos);
    Matrix34f mtx;
    mtx.setMul(*mModelMatrix, local);
    getDrawMgr_()->drawCylinder16Impl(mDrawContext, mtx, rTop, rBottom);
}

/**
 * Draws a single colored cylinder with 16 divisions.
 * @param rPos center position
 * @param radius radius
 * @param height height
 * @param rColor cylinder color
 */
void PrimitiveDrawer::drawCylinder16(const Vector3f& rPos, f32 radius, f32 height,
                                     const Color4f& rColor)
{
    drawCylinder16(rPos, radius, height, rColor, rColor);
}

/**
 * Draws a cylinder with 32 divisions.
 * @param rPos center position
 * @param radius radius
 * @param height height
 * @param rTop top color
 * @param rBottom bottom color
 */
void PrimitiveDrawer::drawCylinder32(const Vector3f& rPos, f32 radius, f32 height,
                                     const Color4f& rTop, const Color4f& rBottom)
{
    Matrix34f local;
    local.makeST(Vector3f(radius * 2, height, radius * 2), rPos);
    Matrix34f mtx;
    mtx.setMul(*mModelMatrix, local);
    getDrawMgr_()->drawCylinder32Impl(mDrawContext, mtx, rTop, rBottom);
}

/**
 * Draws a single colored cylinder with 32 divisions.
 * @param rPos center position
 * @param radius radius
 * @param height height
 * @param rColor cylinder color
 */
void PrimitiveDrawer::drawCylinder32(const Vector3f& rPos, f32 radius, f32 height,
                                     const Color4f& rColor)
{
    drawCylinder32(rPos, radius, height, rColor, rColor);
}

/**
 * Draws the X, Y and Z axes as red, green and blue lines.
 * @param rPos origin
 * @param scale axis length
 */
void PrimitiveDrawer::drawAxis(const Vector3f& rPos, f32 scale)
{
    drawLine(rPos, rPos + Vector3f(scale, 0.0f, 0.0f), Color4f::cRed);
    drawLine(rPos, rPos + Vector3f(0.0f, scale, 0.0f), Color4f::cGreen);
    drawLine(rPos, rPos + Vector3f(0.0f, 0.0f, scale), Color4f::cBlue);
}

namespace PrimitiveDrawUtil
{
/**
 * Copies the unit quad vertices and indices.
 * @param pVertex vertex output, or nullptr
 * @param pIndex index output, or nullptr
 */
void setQuadVertex(Vertex* pVertex, u16* pIndex)
{
    static const Vertex cVtx[4] = {
        Vertex(Vector3f(-0.5f, 0.5f, 0.0f), Vector2f(0.0f, 1.0f), Color4f(0.0f, 0.0f, 0.0f, 0.0f)),
        Vertex(Vector3f(0.5f, 0.5f, 0.0f), Vector2f(1.0f, 1.0f), Color4f(0.0f, 0.0f, 0.0f, 0.0f)),
        Vertex(Vector3f(-0.5f, -0.5f, 0.0f), Vector2f(0.0f, 0.0f), Color4f(1.0f, 0.0f, 0.0f, 0.0f)),
        Vertex(Vector3f(0.5f, -0.5f, 0.0f), Vector2f(1.0f, 0.0f), Color4f(1.0f, 0.0f, 0.0f, 0.0f))};

    static const u16 cIdx[6] = {0, 2, 1, 1, 2, 3};

    if (pVertex != nullptr)
    {
        MemUtil::copy(pVertex, cVtx, sizeof(cVtx));
    }

    if (pIndex)
    {
        MemUtil::copy(pIndex, cIdx, sizeof(cIdx));
    }
}

/**
 * Copies the unit line vertices and indices.
 * @param pVertex vertex output, or nullptr
 * @param pIndex index output, or nullptr
 */
void setLineVertex(Vertex* pVertex, u16* pIndex)
{
    static const Vertex cVtx[2] = {
        Vertex(Vector3f(-0.5f, 0.0f, 0.0f), Vector2f(0.0f, 0.5f), Color4f(0.0f, 0.0f, 0.0f, 0.0f)),
        Vertex(Vector3f(0.5f, 0.0f, 0.0f), Vector2f(1.0f, 0.5f), Color4f(1.0f, 0.0f, 0.0f, 0.0f))};

    static const u16 cIdx[2] = {0, 1};

    if (pVertex != nullptr)
    {
        MemUtil::copy(pVertex, cVtx, sizeof(cVtx));
    }

    if (pIndex)
    {
        MemUtil::copy(pIndex, cIdx, sizeof(cIdx));
    }
}

/**
 * Copies the unit cube vertices and indices.
 * @param pVertex vertex output, or nullptr
 * @param pIndex index output, or nullptr
 */
void setCubeVertex(Vertex* pVertex, u16* pIndex)
{
    static const Vertex cVtx[8] = {
        Vertex(Vector3f(-0.5f, -0.5f, -0.5f), Vector2f(0.0f, 0.0f),
               Color4f(1.0f / 3.0f, 0.0f, 0.0f, 0.0f)),
        Vertex(Vector3f(-0.5f, 0.5f, -0.5f), Vector2f(0.0f, 1.0f), Color4f(0.0f, 0.0f, 0.0f, 0.0f)),
        Vertex(Vector3f(-0.5f, 0.5f, 0.5f), Vector2f(1.0f, 1.0f),
               Color4f(1.0f / 3.0f, 0.0f, 0.0f, 0.0f)),
        Vertex(Vector3f(-0.5f, -0.5f, 0.5f), Vector2f(1.0f, 0.0f),
               Color4f(2.0f / 3.0f, 0.0f, 0.0f, 0.0f)),
        Vertex(Vector3f(0.5f, -0.5f, 0.5f), Vector2f(0.0f, 0.0f), Color4f(1.0f, 0.0f, 0.0f, 0.0f)),
        Vertex(Vector3f(0.5f, 0.5f, 0.5f), Vector2f(0.0f, 1.0f),
               Color4f(2.0f / 3.0f, 0.0f, 0.0f, 0.0f)),
        Vertex(Vector3f(0.5f, 0.5f, -0.5f), Vector2f(1.0f, 1.0f),
               Color4f(1.0f / 3.0f, 0.0f, 0.0f, 0.0f)),
        Vertex(Vector3f(0.5f, -0.5f, -0.5f), Vector2f(1.0f, 0.0f),
               Color4f(2.0f / 3.0f, 0.0f, 0.0f, 0.0f))};

    static const u16 cIdx[36] = {2, 1, 0, 3, 2, 0, 5, 2, 3, 4, 5, 3, 6, 5, 7, 7, 5, 4,
                                 1, 6, 0, 6, 7, 0, 0, 7, 3, 3, 7, 4, 1, 2, 6, 2, 5, 6};

    if (pVertex != nullptr)
    {
        MemUtil::copy(pVertex, cVtx, sizeof(cVtx));
    }

    if (pIndex)
    {
        MemUtil::copy(pIndex, cIdx, sizeof(cIdx));
    }
}

/**
 * Copies the unit cube vertices and the wireframe line strip indices.
 * @param pVertex vertex output, or nullptr
 * @param pIndex index output, or nullptr
 */
void setWireCubeVertex(Vertex* pVertex, u16* pIndex)
{
    setCubeVertex(pVertex, nullptr);

    static const u16 cIdx[17] = {0, 1, 2, 3, 0, 7, 6, 1, 2, 5, 6, 7, 4, 5, 4, 3, 0};

    if (pIndex)
    {
        MemUtil::copy(pIndex, cIdx, sizeof(cIdx));
    }
}

/**
 * Generates the vertices and indices of a unit sphere.
 * @param pVertex vertex output, or nullptr
 * @param pIndex index output, or nullptr
 * @param x number of divisions around the sphere
 * @param y number of rings
 */
void setSphereVertex(Vertex* pVertex, u16* pIndex, s32 x, s32 y)
{
    if (pVertex != nullptr)
    {
        for (s32 i = 0; i < y; i++)
        {
            f32 angle_y = ((i + 1) / (y + 1.0f) - 0.5f) * numbers::pi;

            f32 pos_y = std::sin(angle_y) * 0.5f;
            f32 radius = std::cos(angle_y) * 0.5f;

            for (s32 j = 0; j < x; j++)
            {
                s32 pos = i * x + j;
                f32 angle_x = (numbers::pi * 2.0f) * j / x;

                if (i % 2 == 0)
                {
                    angle_x -= (numbers::pi * 2.0f) / x / 2;
                }

                f32 pos_x = std::cos(angle_x) * radius;
                f32 pos_z = std::sin(angle_x) * radius;

                pVertex[pos].pos.x = pos_x;
                pVertex[pos].pos.y = pos_y;
                pVertex[pos].pos.z = pos_z;
                pVertex[pos].uv.x = pos_y + 0.5f;
                pVertex[pos].uv.y = static_cast<f32>(j) / x;
                pVertex[pos].color.r = 0.5f - pos_y;
            }
        }

        {
            s32 pos = x * y;

            pVertex[pos].pos.x = 0.0f;
            pVertex[pos].pos.y = -0.5f;
            pVertex[pos].pos.z = 0.0f;
            pVertex[pos].uv.x = 0.0f;
            pVertex[pos].uv.y = 0.5f;
            pVertex[pos].color.r = 1.0f;
        }

        {
            s32 pos = x * y + 1;

            pVertex[pos].pos.x = 0.0f;
            pVertex[pos].pos.y = 0.5f;
            pVertex[pos].pos.z = 0.0f;
            pVertex[pos].uv.x = 1.0f;
            pVertex[pos].uv.y = 0.5f;
            pVertex[pos].color.r = 0.0f;
        }
    }

    if (pIndex)
    {
        for (s32 i = 0; i < x; i++)
        {
            pIndex[i * 3 + 0] = x * y;
            pIndex[i * 3 + 1] = i;
            pIndex[i * 3 + 2] = (i + 1) % x;
        }

        for (s32 i = 0; i < y - 1; i++)
        {
            for (s32 j = 0; j < x; j++)
            {
                s32 offset = i % 2;
                s32 pos = (i * x * 6) + j * 6 + x * 3;

                pIndex[pos + 0] = i * x + j;
                pIndex[pos + 1] = (i + 1) * x + ((j + offset) % x);
                pIndex[pos + 2] = i * x + ((j + 1) % x);
                pIndex[pos + 3] = (i + 1) * x + ((j + offset) % x);
                pIndex[pos + 4] = (i + 1) * x + ((j + 1 + offset) % x);
                pIndex[pos + 5] = i * x + ((j + 1) % x);
            }
        }

        for (s32 i = 0; i < x; i++)
        {
            s32 posOffs = 3 * x * (y - 1) * 2 + x * 3;

            pIndex[i * 3 + 0 + posOffs] = x * y + 1;
            pIndex[i * 3 + 1 + posOffs] = x * (y - 1) + ((i + 1) % x);
            pIndex[i * 3 + 2 + posOffs] = x * (y - 1) + i;
        }
    }
}

/**
 * Generates the vertices and indices of a unit disk.
 * @param pVertex vertex output, or nullptr
 * @param pIndex index output, or nullptr
 * @param divNum number of divisions
 */
void setDiskVertex(Vertex* pVertex, u16* pIndex, s32 divNum)
{
    if (pVertex != nullptr)
    {
        for (s32 i = 0; i < divNum; i++)
        {
            f32 angle = (numbers::pi * 2.0f) * i / divNum;

            pVertex[i].pos.x = std::cos(angle) * 0.5f;
            pVertex[i].pos.y = std::sin(angle) * 0.5f;
            pVertex[i].pos.z = 0.0f;
            pVertex[i].uv.x = pVertex[i].pos.x;
            pVertex[i].uv.y = 1.0f - pVertex[i].pos.y;
            pVertex[i].color.r = 1.0f;
        }

        {
            s32 i = divNum;

            pVertex[i].pos.x = 0.0f;
            pVertex[i].pos.y = 0.0f;
            pVertex[i].pos.z = 0.0f;
            pVertex[i].uv.x = 0.5f;
            pVertex[i].uv.y = 0.5f;
            pVertex[i].color.r = 0.0f;
        }
    }

    if (pIndex)
    {
        for (s32 i = 0; i < divNum; i++)
        {
            pIndex[i * 3 + 0] = i;
            pIndex[i * 3 + 1] = (i + 1) % divNum;
            pIndex[i * 3 + 2] = divNum;
        }
    }
}

/**
 * Copies the unit line vertices and indices.
 * @param pVertex vertex output, or nullptr
 * @param pIndex index output, or nullptr
 */
void setCylinderVertex(Vertex* pVertex, u16* pIndex, s32 divNum)
{
    if (pVertex != nullptr)
    {
        for (s32 i = 0; i < divNum; i++)
        {
            f32 angle = (numbers::pi * 2.0f) * i / divNum;

            pVertex[i].pos.x = std::cos(angle) * 0.5f;
            pVertex[i].pos.z = -std::sin(angle) * 0.5f;
            pVertex[i].pos.y = 0.5f;
            pVertex[i].uv.x = pVertex[i].pos.x;
            pVertex[i].uv.y = 1.0f - pVertex[i].pos.z;
            pVertex[i].color.r = 0.0f;

            s32 pos = i + divNum + 1;

            pVertex[pos].pos.x = std::cos(angle) * 0.5f;
            pVertex[pos].pos.z = -std::sin(angle) * 0.5f;
            pVertex[pos].pos.y = -0.5f;
            pVertex[pos].uv.x = pVertex[i].pos.x;
            pVertex[pos].uv.y = 1.0f - pVertex[i].pos.z;
            pVertex[pos].color.r = 1.0f;
        }

        {
            s32 pos = divNum;

            pVertex[pos].pos.x = 0.0f;
            pVertex[pos].pos.y = 0.5f;
            pVertex[pos].pos.z = 0.0f;
            pVertex[pos].uv.x = 0.5f;
            pVertex[pos].uv.y = 0.5f;
            pVertex[pos].color.r = 0.0f;
        }

        {
            s32 pos = divNum + divNum + 1;

            pVertex[pos].pos.x = 0.0f;
            pVertex[pos].pos.y = -0.5f;
            pVertex[pos].pos.z = 0.0f;
            pVertex[pos].uv.x = 0.5f;
            pVertex[pos].uv.y = 0.5f;
            pVertex[pos].color.r = 1.0f;
        }
    }

    if (pIndex)
    {
        for (s32 i = 0; i < divNum; i++)
        {
            pIndex[i * 3 + 0] = i;
            pIndex[i * 3 + 1] = (i + 1) - ((i + 1) % divNum);
            pIndex[i * 3 + 2] = divNum;

            s32 posOffs = divNum * 3;
            pIndex[i * 3 + 0 + posOffs] = i + (divNum + 1);
            pIndex[i * 3 + 1 + posOffs] = divNum + (divNum + 1);
            pIndex[i * 3 + 2 + posOffs] = ((i + 1) - ((i + 1) % divNum)) + (divNum + 1);
        }

        for (s32 i = 0; i < divNum; i++)
        {
            s32 posOffs = divNum * 6;

            pIndex[i * 6 + 0 + posOffs] = i;
            pIndex[i * 6 + 1 + posOffs] = i + (divNum + 1);
            pIndex[i * 6 + 2 + posOffs] = (i + 1) - ((i + 1) % divNum);
            pIndex[i * 6 + 3 + posOffs] = (i + 1) - ((i + 1) % divNum);
            pIndex[i * 6 + 4 + posOffs] = i + (divNum + 1);
            pIndex[i * 6 + 5 + posOffs] = ((i + 1) - ((i + 1) % divNum)) + (divNum + 1);
        }
    }
}

}  // namespace PrimitiveDrawUtil

}  // namespace sead
