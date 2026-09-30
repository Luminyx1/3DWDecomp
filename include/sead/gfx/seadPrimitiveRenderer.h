#ifndef SEAD_PRIMITIVE_RENDERER_H_
#define SEAD_PRIMITIVE_RENDERER_H_

#include <gfx/seadColor.h>
#include <heap/seadDisposer.h>
#include <math/seadBoundBox.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>
#include <prim/seadSafeString.h>

namespace sead
{
class Heap;
class Camera;
class Projection;
class Texture;
class DrawContext;
class PrimitiveDrawMgrNvn;

class PrimitiveDrawer
{
public:
    class QuadArg
    {
    public:
        QuadArg() = default;

        void setCornerAndSize(const Vector3f& rCorner, const Vector2f& rSize);
        QuadArg& setBoundBox(const BoundBox2f& rBox, f32 z);
        QuadArg& setColor(const Color4f& rColor0, const Color4f& rColor1);
        QuadArg& setColor(const Color4f& rColor) { return setColor(rColor, rColor); }
        void setColorHorizontal(const Color4f& rColor0, const Color4f& rColor1);
        void setCenter(const Vector3f& rCenter) { mCenter = rCenter; }
        void setSize(const Vector2f& rSize) { mSize = rSize; }

        const Vector3f& getCenter() const { return mCenter; }
        const Vector2f& getSize() const { return mSize; }
        const Color4f& getColor0() const { return mColor0; }
        const Color4f& getColor1() const { return mColor1; }
        bool isHorizontal() const { return mIsHorizontal; }

    private:
        Vector3f mCenter = Vector3f::zero;
        Vector2f mSize = Vector2f(Vector3f::ones.x, Vector3f::ones.y);
        Color4f mColor0 = Color4f::cWhite;
        Color4f mColor1 = Color4f::cWhite;
        bool mIsHorizontal = false;
    };

    class CubeArg
    {
    public:
        CubeArg() = default;

        void setCornerAndSize(const Vector3f& rCorner, const Vector3f& rSize);
        void setBoundBox(const BoundBox3f& rBox);

        const Vector3f& getCenter() const { return mCenter; }
        const Vector3f& getSize() const { return mSize; }
        const Color4f& getColor0() const { return mColor0; }
        const Color4f& getColor1() const { return mColor1; }

    private:
        Vector3f mCenter = Vector3f::zero;
        Vector3f mSize = Vector3f::ones;
        Color4f mColor0 = Color4f::cWhite;
        Color4f mColor1 = Color4f::cWhite;
    };

    class UVArg
    {
    public:
        UVArg(const Vector2f& rUVSrc, const Vector2f& rUVSize) : mUVSrc(rUVSrc), mUVSize(rUVSize)
        {
        }

        const Vector2f& getUVSrc() const { return mUVSrc; }
        const Vector2f& getUVSize() const { return mUVSize; }

    private:
        Vector2f mUVSrc;
        Vector2f mUVSize;
    };

    explicit PrimitiveDrawer(DrawContext* pDrawContext);
    virtual ~PrimitiveDrawer();

    static void prepareMgrFromBinary(Heap* pHeap, const void* pBinary, u32 binarySize);
    static PrimitiveDrawMgrNvn* createDrawMgrInstance_(Heap* pHeap);
    static void prepareMgr(Heap* pHeap, const SafeString& rPath);
    static void prepareMgrFromBinary(Heap* pHeap, const void* pBinary, u32 binarySize,
                                     u32 uniformBufferSize, bool isTextureEnable);
    static void prepareMgr(Heap* pHeap, const SafeString& rPath, u32 uniformBufferSize,
                           bool isTextureEnable);

    void setCamera(const Camera* pCamera);
    void setCameraViewMatrix(const Matrix34f* pViewMatrix);
    void setProjection(const Projection* pProjection);
    void setProjectionMatrix(const Matrix44f* pProjectionMatrix);
    void setModelMatrix(const Matrix34f* pModelMatrix);
    void setDrawContext(DrawContext* pDrawContext);
    void begin();
    static PrimitiveDrawMgrNvn* getDrawMgr_();
    void end();

    void drawQuad(const Color4f& rColor0, const Color4f& rColor1);
    void drawQuad(const Texture& rTexture, const Color4f& rColor0, const Color4f& rColor1,
                  const Vector2f& rUVSrc, const Vector2f& rUVSize);
    void drawBox(const Color4f& rColor0, const Color4f& rColor1);
    void drawCube(const Color4f& rColor0, const Color4f& rColor1);
    void drawWireCube(const Color4f& rColor0, const Color4f& rColor1);
    void drawLine(const Color4f& rColor0, const Color4f& rColor1);
    void drawSphere4x8(const Color4f& rNorth, const Color4f& rSouth);
    void drawSphere8x16(const Color4f& rNorth, const Color4f& rSouth);
    void drawDisk16(const Color4f& rCenter, const Color4f& rEdge);
    void drawDisk32(const Color4f& rCenter, const Color4f& rEdge);
    void drawCircle16(const Color4f& rEdge);
    void drawCircle32(const Color4f& rEdge);
    void drawCylinder16(const Color4f& rTop, const Color4f& rBottom);
    void drawCylinder32(const Color4f& rTop, const Color4f& rBottom);

    void drawQuad(const QuadArg& rArg);
    void drawQuad(const Texture& rTexture, const QuadArg& rArg, const UVArg& rUVArg);
    void drawBox(const QuadArg& rArg);
    void drawCube(const CubeArg& rArg);
    void drawWireCube(const CubeArg& rArg);
    void drawLine(const Vector3f& rFrom, const Vector3f& rTo, const Color4f& rColor0,
                  const Color4f& rColor1);
    void drawLine(const Vector3f& rFrom, const Vector3f& rTo, const Color4f& rColor);
    void drawSphere4x8(const Vector3f& rPos, f32 radius, const Color4f& rNorth,
                       const Color4f& rSouth);
    void drawSphere4x8(const Vector3f& rPos, f32 radius, const Color4f& rColor);
    void drawSphere8x16(const Vector3f& rPos, f32 radius, const Color4f& rNorth,
                        const Color4f& rSouth);
    void drawSphere8x16(const Vector3f& rPos, f32 radius, const Color4f& rColor);
    void drawDisk16(const Vector3f& rPos, f32 radius, const Color4f& rCenter,
                    const Color4f& rEdge);
    void drawDisk16(const Vector3f& rPos, f32 radius, const Color4f& rColor);
    void drawDisk32(const Vector3f& rPos, f32 radius, const Color4f& rCenter,
                    const Color4f& rEdge);
    void drawDisk32(const Vector3f& rPos, f32 radius, const Color4f& rColor);
    void drawCircle16(const Vector3f& rPos, f32 radius, const Color4f& rColor);
    void drawCircle32(const Vector3f& rPos, f32 radius, const Color4f& rColor);
    void drawCylinder16(const Vector3f& rPos, f32 radius, f32 height, const Color4f& rTop,
                        const Color4f& rBottom);
    void drawCylinder16(const Vector3f& rPos, f32 radius, f32 height, const Color4f& rColor);
    void drawCylinder32(const Vector3f& rPos, f32 radius, f32 height, const Color4f& rTop,
                        const Color4f& rBottom);
    void drawCylinder32(const Vector3f& rPos, f32 radius, f32 height, const Color4f& rColor);
    void drawAxis(const Vector3f& rPos, f32 scale);

private:
    const Matrix34f* mModelMatrix;
    const Matrix34f* mViewMatrix;
    const Matrix44f* mProjectionMatrix;
    DrawContext* mDrawContext;
};

class PrimitiveRenderer
{
    SEAD_SINGLETON_DISPOSER(PrimitiveRenderer)
public:
    PrimitiveRenderer();
    virtual ~PrimitiveRenderer();

    void prepareFromBinary(Heap* pHeap, const void* pBinary, u32 binarySize);
    void prepare(Heap* pHeap, const SafeString& rPath);
    void setCamera(const Camera& rCamera);
    void setProjection(const Projection& rProjection);
    void setModelMatrix(const Matrix34f& rModelMatrix);
    void begin();
    void end();

    void drawQuad(const Color4f& rColor0, const Color4f& rColor1);
    void drawQuad(const Texture& rTexture, const Color4f& rColor0, const Color4f& rColor1,
                  const Vector2f& rUVSrc, const Vector2f& rUVSize);
    void drawBox(const Color4f& rColor0, const Color4f& rColor1);
    void drawCube(const Color4f& rColor0, const Color4f& rColor1);
    void drawWireCube(const Color4f& rColor0, const Color4f& rColor1);
    void drawLine(const Color4f& rColor0, const Color4f& rColor1);
    void drawSphere4x8(const Color4f& rNorth, const Color4f& rSouth);
    void drawSphere8x16(const Color4f& rNorth, const Color4f& rSouth);
    void drawDisk16(const Color4f& rCenter, const Color4f& rEdge);
    void drawDisk32(const Color4f& rCenter, const Color4f& rEdge);
    void drawCircle16(const Color4f& rEdge);
    void drawCircle32(const Color4f& rEdge);
    void drawCylinder16(const Color4f& rTop, const Color4f& rBottom);
    void drawCylinder32(const Color4f& rTop, const Color4f& rBottom);

    void drawQuad(const PrimitiveDrawer::QuadArg& rArg);
    void drawQuad(const Texture& rTexture, const PrimitiveDrawer::QuadArg& rArg,
                  const PrimitiveDrawer::UVArg& rUVArg);
    void drawBox(const PrimitiveDrawer::QuadArg& rArg);
    void drawCube(const PrimitiveDrawer::CubeArg& rArg);
    void drawWireCube(const PrimitiveDrawer::CubeArg& rArg);
    void drawLine(const Vector3f& rFrom, const Vector3f& rTo, const Color4f& rColor0,
                  const Color4f& rColor1);
    void drawLine(const Vector3f& rFrom, const Vector3f& rTo, const Color4f& rColor);
    void drawSphere4x8(const Vector3f& rPos, f32 radius, const Color4f& rNorth,
                       const Color4f& rSouth);
    void drawSphere4x8(const Vector3f& rPos, f32 radius, const Color4f& rColor);
    void drawSphere8x16(const Vector3f& rPos, f32 radius, const Color4f& rNorth,
                        const Color4f& rSouth);
    void drawSphere8x16(const Vector3f& rPos, f32 radius, const Color4f& rColor);
    void drawDisk16(const Vector3f& rPos, f32 radius, const Color4f& rCenter,
                    const Color4f& rEdge);
    void drawDisk16(const Vector3f& rPos, f32 radius, const Color4f& rColor);
    void drawDisk32(const Vector3f& rPos, f32 radius, const Color4f& rCenter,
                    const Color4f& rEdge);
    void drawDisk32(const Vector3f& rPos, f32 radius, const Color4f& rColor);
    void drawCircle16(const Vector3f& rPos, f32 radius, const Color4f& rColor);
    void drawCircle32(const Vector3f& rPos, f32 radius, const Color4f& rColor);
    void drawCylinder16(const Vector3f& rPos, f32 radius, f32 height, const Color4f& rTop,
                        const Color4f& rBottom);
    void drawCylinder16(const Vector3f& rPos, f32 radius, f32 height, const Color4f& rColor);
    void drawCylinder32(const Vector3f& rPos, f32 radius, f32 height, const Color4f& rTop,
                        const Color4f& rBottom);
    void drawCylinder32(const Vector3f& rPos, f32 radius, f32 height, const Color4f& rColor);
    void drawAxis(const Vector3f& rPos, f32 scale);

private:
    PrimitiveDrawer mDrawer{nullptr};
    Matrix34f mViewMatrix = Matrix34f::ident;
    Matrix44f mProjectionMatrix = Matrix44f::ident;
    Matrix34f mModelMatrix = Matrix34f::ident;
};
static_assert(sizeof(PrimitiveRenderer) == 0xf0);

}  // namespace sead

#endif  // SEAD_PRIMITIVE_RENDERER_H_
