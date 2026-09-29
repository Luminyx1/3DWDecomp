#pragma once

#include <basis/seadTypes.h>
#include <hostio/seadHostIOPropertyEvent.h>
#include "utility/aglParameter.h"

namespace sead::hostio {
class Context;
class PropertyEvent;
class Reflexible;
}  // namespace sead::hostio

namespace agl::proc {

class LUT {
public:
    struct GenerateArg {
        s32 mNum;
    };

    struct TransformArg {
        const f32* mpSrc;
        s32 mWidth;
        s32 mHeight;
    };

    LUT();
    LUT(const sead::SafeString& rName, utl::IParameterObj* pObj);
    LUT(const sead::SafeString& rName, const sead::SafeString& rLabel, utl::IParameterObj* pObj);
    ~LUT();

    void generate(f32* pDst, s32 stride, const GenerateArg& rArg) const;
    void transform(f32* pDst, s32 stride, const TransformArg& rArg) const;

    void genMessageParameters(sead::hostio::Context* pContext);
    void listenPropertyEventParameters(sead::hostio::Reflexible* pReflexible,
                                       const sead::hostio::PropertyEvent* pEvent);

    void setChannelNum(s32 num) { mChannelNum = num; }
    s32 getChannelNum() const { return mChannelNum; }
    bool isSingleCurve() const { return mIsSingleCurve; }
    utl::ParameterCurve<4>& getCurve() { return mCurve; }
    const utl::ParameterCurve<4>& getCurve() const { return mCurve; }

private:
    static bool isEventTarget_(const sead::hostio::PropertyEvent* pEvent, const void* pData,
                               size_t size)
    {
        const u8* id = static_cast<const u8*>(pEvent->getId());
        return id >= pData && id < static_cast<const u8*>(pData) + size;
    }

    f32 interpolate_(s32 index, f32 t) const
    {
        auto& curve = const_cast<sead::hostio::Curve<f32>&>(mCurve.getCurve(index));
        return curve.sead::hostio::Curve<f32>::interpolateToF32(t);
    }

    utl::ParameterCurve<4> mCurve;
    s32 mChannelNum = 4;
    bool mIsSingleCurve = false;
};
static_assert(sizeof(LUT) == 0x280);

}  // namespace agl::proc
