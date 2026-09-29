#include "postfx/aglProcLUT.h"

#include <container/seadSafeArray.h>
#include <hostio/seadHostIOPropertyEvent.h>

namespace agl::proc {

LUT::LUT()
{
}

LUT::LUT(const sead::SafeString& rName, utl::IParameterObj* pObj)
    : mCurve(rName, rName, pObj)
{
}

LUT::LUT(const sead::SafeString& rName, const sead::SafeString& rLabel, utl::IParameterObj* pObj)
    : mCurve(rName, rLabel, pObj)
{
}

LUT::~LUT() = default;

void LUT::generate(f32* pDst, s32 stride, const GenerateArg& rArg) const
{
    if (mIsSingleCurve)
    {
        for (s32 i = 0; i < rArg.mNum; i++)
        {
            f32 t = f32(i) / f32(rArg.mNum - 1);
            for (s32 j = 0; j < stride; j++)
            {
                pDst[j] = interpolate_(0, t);
            }
            pDst += stride;
        }
    }
    else
    {
        for (s32 i = 0; i < rArg.mNum; i++)
        {
            f32 t = f32(i) / f32(rArg.mNum - 1);
            for (s32 j = 0; j < stride; j++)
            {
                pDst[j] = interpolate_(j, t);
            }
            pDst += stride;
        }
    }
}

void LUT::transform(f32* pDst, s32 stride, const TransformArg& rArg) const
{
    s32 num = rArg.mWidth < stride ? rArg.mWidth : stride;
    if (mIsSingleCurve)
    {
        for (s32 y = 0; y < rArg.mHeight; y++)
        {
            u32 base = rArg.mWidth * y;
            for (s32 x = 0; x < num; x++)
            {
                pDst[x] = interpolate_(0, rArg.mpSrc[base + x]);
            }
            pDst += stride;
        }
    }
    else
    {
        for (s32 y = 0; y < rArg.mHeight; y++)
        {
            u32 base = rArg.mWidth * y;
            for (s32 x = 0; x < num; x++)
            {
                pDst[x] = interpolate_(x, rArg.mpSrc[base + x]);
            }
            pDst += stride;
        }
    }
}

void LUT::genMessageParameters(sead::hostio::Context* pContext)
{
    if (mIsSingleCurve)
    {
        sead::SafeString label = sead::SafeString::cEmptyString.isEmpty() ?
                                     mCurve.getLabel() :
                                     sead::SafeString::cEmptyString;
    }
    else
    {
        for (s32 i = 0; i < mChannelNum; i++)
        {
            sead::SafeString label = sead::SafeString::cEmptyString.isEmpty() ?
                                         mCurve.getLabel() :
                                         sead::SafeString::cEmptyString;
        }
    }
}

void LUT::listenPropertyEventParameters(sead::hostio::Reflexible* pReflexible,
                                        const sead::hostio::PropertyEvent* pEvent)
{
    const void* id = pEvent->getId();
    if (!(pEvent->getType() & 2) && id < &mIsSingleCurve + 1 && id >= &mIsSingleCurve)
    {
        return;
    }

    if (!((id >= &mCurve.getCurve(0) && &mCurve.getCurve(1) > id) ||
          (id >= &mCurve.getCurve(1) && &mCurve.getCurve(2) > id) ||
          (id >= &mCurve.getCurve(2) && &mCurve.getCurve(3) > id) ||
          (id >= &mCurve.getCurve(3) && &mCurve.getCurve(3) + 1 > id)))
    {
        return;
    }

    if (!mIsSingleCurve)
    {
        return;
    }

    auto& data = reinterpret_cast<sead::SafeArray<f32, 128>&>(mCurve.getCurveData(0));
    f32 buf[30];
    u8 num = mCurve.getCurve(0).mInfo.numUse;
    for (s32 i = 0; i < num; i++)
    {
        buf[i] = data[i + 2];
    }

    u8 type = mCurve.getCurve(0).mInfo.curveType;
    for (s32 c = 1; c < 4; c++)
    {
        mCurve.getCurve(c).mInfo.numUse = num;
        mCurve.getCurve(c).mInfo.curveType = type;
        mCurve.getCurveData(c).curveType = type;
        mCurve.getCurveData(c).numUse = num;
        for (s32 i = 0; i < num; i++)
        {
            mCurve.getCurveData(c).f[i] = buf[i];
        }
    }
}

}  // namespace agl::proc
