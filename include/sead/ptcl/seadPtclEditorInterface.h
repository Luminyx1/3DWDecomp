#pragma once

#include <basis/seadTypes.h>
#include <gfx/seadColor.h>
#include <math/seadMatrix.h>
#include <prim/seadSafeString.h>

namespace sead {
namespace ptcl {
class PtclWorldScale {
public:
    PtclWorldScale() {
        mPosRange = 100.0f;
        mScaleRange = 1.0f;
        mVelRange = 10.0f;
        mEmissionRange = 10.0f;
        mEmissionRateMax = 1000;
        update();
    }

    void update() {
        mPosOption.format("Min=%5.1f,Max=%5.1f", -mPosRange, mPosRange);
        mPosAbsOption.format("Min=0,Max=%5.1f", mPosRange);
        mScaleOption.format("Min=%5.1f,Max=%5.1f", -mScaleRange, mScaleRange);
        mScaleAbsOption.format("Min=0,Max=%5.1f", mScaleRange);
        mScaleAbsSmallOption.format("Min=0,Max=%5.1f", mScaleRange / 10.0f);
        mVelOption.format("Min=%5.1f,Max=%5.1f,Menu=True,MenuDefault=Auto", -mVelRange,
                          mVelRange);
        mVelAbsOption.format("Min=0,Max=%5.1f,Menu=True,MenuDefault=Auto", mVelRange);
        mEmissionOption.format("Min=%5.1f,Max=%5.1f,Menu=True,MenuDefault=Auto",
                               -mEmissionRange, mEmissionRange);
        mEmissionAbsOption.format("Min=0,Max=%5.1f,Menu=True,MenuDefault=Auto", mEmissionRange);
        mRateOption.format("Min=0,Max=%d", mEmissionRateMax);
        mRateNonZeroOption.format("Min=1,Max=%d", mEmissionRateMax);
        mPosDisableOption.format("%s,IsEnable=False", mPosOption.cstr());
        mPosAbsDisableOption.format("%s,IsEnable=False", mPosAbsOption.cstr());
        mScaleAbsDisableOption.format("%s,IsEnable=False", mScaleAbsOption.cstr());
        mVelDisableOption.format("%s,IsEnable=False", mVelOption.cstr());
        mEmissionDisableOption.format("%s,IsEnable=False", mEmissionOption.cstr());
        mEmissionAbsDisableOption.format("%s,IsEnable=False", mEmissionAbsOption.cstr());
        mRateDisableOption.format("%s,IsEnable=False", mRateOption.cstr());
        mVelAbsDisableOption.format("%s,IsEnable=False", mVelAbsOption.cstr());
        mRateNonZeroDisableOption.format("%s,IsEnable=False", mRateNonZeroOption.cstr());
        mScaleAbsMenuOption.format("%s,Menu=True,MenuDefault=Auto", mScaleAbsOption.cstr());
        mRateLargeOption.format("Min=0,Max=%d", mEmissionRateMax * 10);
        mRateLargeNonZeroOption.format("Min=1,Max=%d", mEmissionRateMax * 10);
        mRateLargeDisableOption.format("%s,IsEnable=False", mRateLargeOption.cstr());
        mRateLargeNonZeroDisableOption.format("%s,IsEnable=False",
                                              mRateLargeNonZeroOption.cstr());
    }

private:
    f32 mPosRange;
    f32 mVelRange;
    f32 mScaleRange;
    f32 mEmissionRange;
    s32 mEmissionRateMax;
    FixedSafeString<64> mPosOption;
    FixedSafeString<64> mPosAbsOption;
    FixedSafeString<64> mScaleOption;
    FixedSafeString<64> mScaleAbsOption;
    FixedSafeString<64> mScaleAbsSmallOption;
    FixedSafeString<96> mScaleAbsMenuOption;
    FixedSafeString<96> mVelOption;
    FixedSafeString<96> mVelAbsOption;
    FixedSafeString<96> mEmissionOption;
    FixedSafeString<96> mEmissionAbsOption;
    FixedSafeString<64> mRateOption;
    FixedSafeString<64> mRateNonZeroOption;
    FixedSafeString<64> mPosDisableOption;
    FixedSafeString<64> mPosAbsDisableOption;
    FixedSafeString<64> mScaleAbsDisableOption;
    FixedSafeString<96> mVelDisableOption;
    FixedSafeString<96> mVelAbsDisableOption;
    FixedSafeString<64> mEmissionDisableOption;
    FixedSafeString<96> mEmissionAbsDisableOption;
    FixedSafeString<64> mRateDisableOption;
    FixedSafeString<64> mRateNonZeroDisableOption;
    FixedSafeString<64> mRateLargeOption;
    FixedSafeString<64> mRateLargeNonZeroOption;
    FixedSafeString<64> mRateLargeDisableOption;
    FixedSafeString<64> mRateLargeNonZeroDisableOption;
};

static_assert(sizeof(PtclWorldScale) == 0x9b0);

class PtclEditorInterface {
public:
    PtclEditorInterface() {
        mMtx = Matrix34f::ident;
        mLocalMtx = Matrix34f::ident;
        mScale = 1.0f;
        _8 = nullptr;

        for (s32 i = 0; i < cParamNum; i++) {
            mParamNames[i].format("[%d]", i);
        }

        mValue1Name = "数値１";
        mValue2Name = "数値２";
        mColor = Color4f(1.0f, 1.0f, 1.0f, 1.0f);
    }

    static const s32 cParamNum = 16;

private:
    void* _0;
    void* _8;
    PtclWorldScale mWorldScale;
    Matrix34f mMtx;
    Color4f mColor;
    s32 _a00 = 0;
    Matrix34f mLocalMtx;
    f32 mScale;
    FixedSafeString<64> mParamNames[cParamNum];
    FixedSafeString<64> mValue1Name;
    FixedSafeString<64> mValue2Name;
};

static_assert(sizeof(PtclEditorInterface) == 0x1068);
}  // namespace ptcl
}  // namespace sead
