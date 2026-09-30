#include "effect/aglStar.h"

#include <gfx/seadGraphicsContext.h>
#include <hostio/seadHostIOPropertyEvent.h>
#include <math/seadMathCalcCommon.h>
#include <math/seadQuat.h>
#include <random/seadRandom.h>

#include "common/aglDrawContext.h"
#include "common/aglShaderProgram.h"
#include "common/aglShaderProgramArchive.h"
#include "detail/aglPrivateResource.h"
#include "detail/aglShaderHolder.h"
#include "driver/aglGraphicsDriverMgr.h"

namespace agl::eft {

namespace {

struct EditParam {
    f32 mAttenuationInnerDeg;
    u32 mUnitNum;
    f32 mSizeMin;
    f32 mSizeMax;
};

struct EditParam2 {
    f32 mAttenuationOuterDeg;
    f32 mDirPitch;
    f32 mDirYaw;
};

EditParam sEditParam = {25.0f, 0, 0.0f, 0.0f};
EditParam2 sEditParam2;

void calcDirAngle(const sead::Vector3f& rDir)
{
    f32 len = rDir.length();

    if (len > 0.0f)
    {
        f32 inv = 1.0f / len;
        f32 x = inv * rDir.x;
        f32 y = inv * rDir.y;
        f32 z = inv * rDir.z;
        sEditParam2.mDirPitch = sead::Mathf::asin(-sead::Mathf::clamp(y, -1.0f, 1.0f));
        f32 lenXZ = sead::Mathf::sqrt(x * x + z * z);

        if (lenXZ > 0.0f)
        {
            f32 invXZ = 1.0f / lenXZ;
            sEditParam2.mDirYaw = sead::Mathf::atan2(-(x * invXZ), -(z * invXZ));
        }
        else
        {
            sEditParam2.mDirYaw = 0.0f;
        }
    }
    else
    {
        sEditParam2.mDirPitch = 0.0f;
        sEditParam2.mDirYaw = 0.0f;
    }
}

}  // namespace

void Star::setUpShader(ShaderProgramArchive* pArchive, sead::Heap* pHeap)
{
    ShaderProgram* pProgram =
        pArchive->getShaderProgramPtr(pArchive->searchShaderProgramIndex("star_render"));
    pProgram->createUniformBlock(2, pHeap);
    pProgram->setUniformBlockName(0, "Context");
    pProgram->setUniformBlockName(1, "UnitInfo");
}

Star::Star()
{
    setDefault();
}

/**
 * Restores the default star parameters.
 */
void Star::setDefault()
{
    mDir.set(0.0f, 1.0f, 0.0f);
    mBrightness = 1.5f;
    mPower = 2.5f;
    mAttenuationScale = 1.0f;
    mAttenuationInner = sead::Mathf::deg2rad(25.0f);
    mAttenuationOuter = 0.0f;
    mTranslate = sead::Vector3f::zero;
    mRotate = 0.0f;
}

/**
 * Frees the star resources.
 */
Star::~Star()
{
    if (mFlag.isOn(1 << 0))
    {
        finalize();
    }
}

/**
 * Frees the units, contexts and shader list.
 */
void Star::finalize()
{
    mProgram.freeBuffer();

    u32 num = mContext.size();

    for (u32 i = 0; i < num; i++)
    {
        mContext[i].mUniformBlock.destroy();
    }

    mContext.freeBuffer();

    removeUnit();
    mFlag.reset(1 << 0);
}

void Star::initialize(const InitializeArg& rArg)
{
    mProgram.tryAllocBuffer(1, rArg.mHeap);
    mProgram.fill(nullptr);
    mProgram(0) = detail::ShaderHolder::instance()->getShaderProgram(198);

    mBrightness = 1.5f;
    mPower = 2.5f;
    mAttenuationScale = 1.0f;
    mAttenuationInner = sead::Mathf::deg2rad(25.0f);
    mAttenuationOuter = 0.0f;
    mTranslate = sead::Vector3f::zero;
    mRotate = 0.0f;

    sEditParam.mAttenuationInnerDeg = sead::Mathf::rad2deg(mAttenuationInner);
    sEditParam2.mAttenuationOuterDeg = sead::Mathf::rad2deg(mAttenuationOuter);

    updateAttenuationParam(mAttenuationInner, mAttenuationOuter);
    f32 pitch = sead::Mathf::deg2rad(60.0f);
    f32 yaw = sead::Mathf::deg2rad(-5.0f);
    mDir.set(sead::Mathf::sin(yaw) * -sead::Mathf::cos(pitch), -sead::Mathf::sin(pitch),
             sead::Mathf::cos(yaw) * -sead::Mathf::cos(pitch));

    mContext.tryAllocBuffer(rArg.mContextNum, rArg.mHeap);

    for (u32 i = 0, n = mContext.size(); i < n; i++)
    {
        mContext[i].mEnable = true;
    }

    for (u32 i = 0, n = mContext.size(); i < n; i++)
    {
        UniformBlock& rBlock = mContext[i].mUniformBlock;

        if (i != 0)
        {
            rBlock.declare(mContext(0).mUniformBlock);
        }
        else
        {
            rBlock.startDeclare(7, rArg.mHeap);
            rBlock.declare(UniformBlock::cType_Vec4, 4);
            rBlock.declare(UniformBlock::cType_Vec4, 3);
            rBlock.declare(UniformBlock::cType_Vec2, 1);
            rBlock.declare(UniformBlock::cType_Float, 1);
            rBlock.declare(UniformBlock::cType_Float, 1);
            rBlock.declare(UniformBlock::cType_Float, 1);
            rBlock.declare(UniformBlock::cType_Vec4, 1);
        }

        rBlock.create(rArg.mHeap, 2, 1);
    }

    allocateUnit(rArg.mHeap, rArg.mUnitNum);
    updateUnit(rArg.mSizeMin, rArg.mSizeMax);
    mFlag.set(1 << 0);
    calcDirAngle(mDir);
    sEditParam.mUnitNum = rArg.mUnitNum;
    sEditParam.mSizeMin = rArg.mSizeMin;
    sEditParam.mSizeMax = rArg.mSizeMax;
}

/**
 * Recomputes the attenuation factors.
 * @param inner inner angle in radians
 * @param outer outer angle in radians
 */
void Star::updateAttenuationParam(f32 inner, f32 outer)
{
    f32 sinInner = sead::Mathf::sin(inner);
    f32 sinOuter = sead::Mathf::sin(outer);
    f32 min = sinOuter > sinInner ? sinInner : sinOuter;
    f32 diff = sinInner - min;
    f32 scale = diff > 1e-05f ? 1.0f / diff : 100000.0f;
    mAttenuationParam[0] = scale;
    mAttenuationParam[1] =
        -(min * scale) - mPower * sead::Mathf::clamp(1.0f - mAttenuationScale, 0.0f, 1.0f);
}

void Star::allocateUnit(sead::Heap* pHeap, u32 unitNum)
{
    if (unitNum == 0)
    {
        return;
    }

    mUnit.tryAllocBuffer(unitNum, pHeap);
    f32 num = f32(mUnit.size()) / f32(mUnitNumPerBlock);
    s32 blockNum = s32(num);

    if (num != f32(blockNum) && num >= 0.0f)
    {
        blockNum++;
    }

    mUnitBlock.tryAllocBuffer(blockNum, pHeap);

    u32 rest = mUnit.size();
    u32 blockSize = mUnitBlock.size();

    for (u32 i = 0; i < blockSize; i++)
    {
        UniformBlock& rBlock = mUnitBlock[i];

        if (i != 0 && mUnitNumPerBlock <= rest)
        {
            rBlock.declare(*mUnitBlock.unsafeGet(0));
        }
        else
        {
            u32 count = rest < mUnitNumPerBlock ? rest : mUnitNumPerBlock;
            rBlock.startDeclare(1, pHeap);
            rBlock.declareStruct(count, sizeof(Unit), 0x10);
        }

        rBlock.create(pHeap, 1, 1);
        rest -= mUnitNumPerBlock;
    }
}

void Star::updateUnit(f32 sizeMin, f32 sizeMax)
{
    sead::Random random(0xa120612a);
    u32 num = mUnit.size();

    for (u32 i = 0; i < num; i++)
    {
        Unit& rUnit = mUnit[i];
        sead::Vector3f dir;
        f32 len;
        do
        {
            dir.x = random.getF32Range(-1.0f, 1.0f);
            dir.y = random.getF32Range(-1.0f, 1.0f);
            dir.z = random.getF32Range(-1.0f, 1.0f);
            len = dir.normalize();
        } while (len == 0.0f);

        rUnit.mDir = dir;
        rUnit.mRandom = random.getF32();
        rUnit.mSize.x = random.getF32Range(sizeMin, sizeMax);
        rUnit.mSize.y = random.getF32Range(sizeMin, sizeMax);
        rUnit.mSize.z = random.getF32Range(sizeMin, sizeMax);
        rUnit.mPhase = random.getF32();
    }

    u32 rest = mUnit.size();
    s32 offset = 0;
    u32 blockNum = mUnitBlock.size();

    for (u32 i = 0; i < blockNum; i++)
    {
        u32 count = rest < mUnitNumPerBlock ? rest : mUnitNumPerBlock;
        UniformBlock& rBlock = mUnitBlock[i];
        rBlock.dcbz(0);
        rBlock.setDataStruct(0, &mUnit[offset], 0, count, sizeof(Unit));
        rBlock.flushCurrentBuffer();
        offset += mUnitNumPerBlock;
        rest -= mUnitNumPerBlock;
    }
}

/**
 * Frees the unit buffers.
 */
void Star::removeUnit()
{
    mUnit.freeBuffer();

    u32 num = mUnitBlock.size();

    for (u32 i = 0; i < num; i++)
    {
        mUnitBlock[i].destroy();
    }

    mUnitBlock.freeBuffer();
}

void Star::updateUBO(u32 index, const sead::Matrix44f& rProjMtx, const sead::Matrix34f& rViewMtx)
{
    if (!mFlag.isOn(1 << 1))
    {
        return;
    }

    if (index == 0 && mFlag.isOn(1 << 3))
    {
        mRotate += sead::Mathf::deg2rad(-0.1f);
    }

    Context& rContext = mContext[index];
    UniformBlock& rBlock = rContext.mUniformBlock;

    mDir.normalize();
    f32 half = mRotate * 0.5f;
    f32 c = sead::Mathf::cos(half);
    f32 s = sead::Mathf::sin(half);
    sead::Quatf q(c, mDir.x * s, mDir.y * s, mDir.z * s);

    sead::Matrix34f rotMtx;
    rotMtx.makeQT(q, sead::Vector3f::zero);

    sead::Matrix34f worldMtx = rotMtx;
    worldMtx.scaleBases(mScale, mScale, mScale);
    worldMtx.setTranslation(worldMtx.getTranslation() + mTranslate);

    sead::Matrix34f viewWorldMtx;
    viewWorldMtx.setMul(rViewMtx, worldMtx);
    sead::Matrix44f mtx;
    mtx.setMul(rProjMtx, viewWorldMtx);

    rBlock.setCurrentBufferIndex(u8(mBufferIndex));
    rBlock.dcbz(0);
    rBlock.setData(0, &mtx, 0, 4);
    rBlock.setData(1, &rotMtx, 0, 3);
    rBlock.setData(2, mAttenuationParam, 0, 1);
    f32 value = mBrightness;
    rBlock.setData(3, &value, 0, 1);
    value = mPower;
    rBlock.setData(4, &value, 0, 1);
    value = mTime;
    rBlock.setData(5, &value, 0, 1);
    rBlock.setData(6, &mColor, 0, 1);
    rBlock.flushCurrentBuffer();
}

/**
 * Draws the stars of one context.
 * @param pDrawContext draw context that receives the commands
 * @param index context index
 */
void Star::draw(DrawContext* pDrawContext, u32 index) const
{
    if (!mFlag.isOn(1 << 1))
    {
        return;
    }

    const Context& rContext = mContext[index];

    if (!rContext.mEnable)
    {
        return;
    }

    {
        sead::GraphicsContext graphicsContext;
        graphicsContext.setDepthEnable(true, false);
        graphicsContext.setAlphaTestEnable(false);
        graphicsContext.setBlendEnable(true);
        graphicsContext.apply(pDrawContext);
    }

    const ShaderProgram* pProgram = mProgram[0];
    pProgram->activate(pDrawContext, true);
    rContext.mUniformBlock.activate(pDrawContext, pProgram->getUniformBlockLocation(0));

    u32 rest = mUnit.size();
    u32 blockNum = mUnitBlock.size();

    for (u32 i = 0; i < blockNum; i++)
    {
        u32 count = rest < mUnitNumPerBlock ? rest : mUnitNumPerBlock;
        mUnitBlock[i].activate(pDrawContext, pProgram->getUniformBlockLocation(1));
        driver::GraphicsDriverMgr::instance()->setPointLimits(pDrawContext, 0.0f, 8191.875f);

        if (count != 0)
        {
            nvnCommandBufferDrawArrays(pDrawContext->getNvnCommandBuffer(),
                                       NVN_DRAW_PRIMITIVE_POINTS, 0, count);
        }

        rest -= mUnitNumPerBlock;
    }
}

/**
 * Does nothing.
 * @param pContext host IO context
 */
void Star::genMessage(sead::hostio::Context* pContext) {}

void Star::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent)
{
    const void* id = pEvent->getId();

    if ((pEvent->getType() & 2) == 0)
    {
        if ((id < &sEditParam2.mDirYaw && id >= &sEditParam2.mDirPitch) || (id < &sEditParam2.mDirYaw + 1 && id >= &sEditParam2.mDirYaw))
        {
            f32 cp = sead::Mathf::cos(sEditParam2.mDirPitch);
            f32 sp = sead::Mathf::sin(sEditParam2.mDirPitch);
            f32 cy = sead::Mathf::cos(sEditParam2.mDirYaw);
            f32 sy = sead::Mathf::sin(sEditParam2.mDirYaw);
            mDir.set(sy * -cp, -sp, cy * -cp);
            return;
        }

        if (id < &mDir + 1 && id >= &mDir)
        {
            calcDirAngle(mDir);
            return;
        }

        if ((id < &sEditParam.mAttenuationInnerDeg + 1 && id >= &sEditParam.mAttenuationInnerDeg) ||
            (id < &sEditParam2.mAttenuationOuterDeg + 1 && id >= &sEditParam2.mAttenuationOuterDeg) ||
            (id < &mAttenuationScale + 1 && id >= &mAttenuationScale))
        {
            mAttenuationInner = sead::Mathf::deg2rad(sEditParam.mAttenuationInnerDeg);
            mAttenuationOuter = sead::Mathf::deg2rad(sEditParam2.mAttenuationOuterDeg);
            updateAttenuationParam(mAttenuationInner, mAttenuationOuter);
            return;
        }
    }

    if (reinterpret_cast<uintptr_t>(id) == 101)
    {
        if (sEditParam.mUnitNum != mUnit.size())
        {
            removeUnit();
            allocateUnit(detail::PrivateResource::instance()->getDebugHeap(), sEditParam.mUnitNum);
        }

        updateUnit(sEditParam.mSizeMin, sEditParam.mSizeMax);
    }
}

}  // namespace agl::eft
