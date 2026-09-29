#pragma once

#include <basis/seadTypes.h>
#include <container/seadBuffer.h>
#include <gfx/seadColor.h>
#include <hostio/seadHostIONode.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>
#include <prim/seadBitFlag.h>

#include "common/aglUniformBlock.h"

namespace sead {
class Heap;
namespace hostio {
class Context;
class PropertyEvent;
}  // namespace hostio
}  // namespace sead

namespace agl {
class DrawContext;
class ShaderProgram;
class ShaderProgramArchive;
}  // namespace agl

namespace agl::eft {

class Star : public sead::hostio::Node {
public:
    struct InitializeArg {
        sead::Heap* mHeap;
        u8 _8[8];
        s32 mContextNum;
        u32 mUnitNum;
        f32 mSizeMin;
        f32 mSizeMax;
    };

    struct Unit {
        sead::Vector3f mDir;
        f32 mRandom;
        sead::Vector3f mSize;
        f32 mPhase;
    };
    static_assert(sizeof(Unit) == 0x20);

    struct Context {
        bool mEnable;
        UniformBlock mUniformBlock;
    };
    static_assert(sizeof(Context) == 0x80);

    Star();
    ~Star();

    static void setUpShader(ShaderProgramArchive* pArchive, sead::Heap* pHeap);

    void setDefault();
    void finalize();
    void initialize(const InitializeArg& rArg);
    void updateAttenuationParam(f32 inner, f32 outer);
    void allocateUnit(sead::Heap* pHeap, u32 unitNum);
    void updateUnit(f32 sizeMin, f32 sizeMax);
    void removeUnit();
    void updateUBO(u32 index, const sead::Matrix44f& rProjMtx, const sead::Matrix34f& rViewMtx);
    void draw(DrawContext* pDrawContext, u32 index) const;

    void genMessage(sead::hostio::Context* pContext);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);

private:
    sead::Buffer<const ShaderProgram*> mProgram;
    sead::Vector3f mDir = {0.0f, 0.0f, 0.0f};
    f32 mRotate = 0.0f;
    f32 mScale = 0.0f;
    f32 mBrightness = 0.0f;
    f32 mPower = 0.0f;
    f32 mAttenuationScale = 0.0f;
    f32 mAttenuationInner = 0.0f;
    f32 mAttenuationOuter = 0.0f;
    f32 mTime = 0.0f;
    sead::Color4f mColor;
    sead::Vector3f mTranslate = {0.0f, 0.0f, 0.0f};
    u32 mUnitNumPerBlock = 0x800;
    sead::Buffer<Unit> mUnit;
    u32 mBufferIndex = 0;
    f32 mAttenuationParam[2] = {0.0f, 0.0f};
    sead::Buffer<UniformBlock> mUnitBlock;
    sead::Buffer<Context> mContext;
    sead::BitFlag32 mFlag;
};
static_assert(sizeof(Star) == 0xb0);

}  // namespace agl::eft
