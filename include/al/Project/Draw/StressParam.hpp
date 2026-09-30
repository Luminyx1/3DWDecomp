#pragma once

#include <utility/aglParameter.h>
#include <utility/aglParameterObj.h>

namespace al {

class StressParam {
public:
    void init();
    bool operator==(const StressParam& rOther) const;
    StressParam& operator=(const StressParam& rOther);
    void interp(const StressParam& rA, const StressParam& rB, f32 rate);

    agl::utl::IParameterObj* getParamObj() { return &mParamObj; }

    bool isUsing16BitDepth() const { return *mIsUsing16BitDepth; }

    bool isUsingLppSpcMask() const { return *mIsUsingLppSpcMask; }

    bool isClearLightBuffer() const { return *mIsClearLightBuffer; }

    bool isClearGBufferViewNrm() const { return *mIsClearGBufferViewNrm; }

    bool isClearGBufferViewDepth() const { return *mIsClearGBufferViewDepth; }

    bool isLppLight() const { return *mIsLppLight; }

    s32 getAntiAliasingType() const { return *mAntiAliasingType; }

    s32 getAntiAliasDetectEdgeQuality() const { return *mAntiAliasDetectEdgeQuality; }

    s32 getScreenWidthScale() const { return *mScreenWidthScale; }

    s32 getScreenHeightScale() const { return *mScreenHeightScale; }

    f32 getReduceQualityPercentage() const { return *mReduceQualityPercentage; }

private:
    agl::utl::ParameterObj mParamObj;
    agl::utl::Parameter<bool> mIsUsing16BitDepth;
    agl::utl::Parameter<bool> mIsUsingLppSpcMask;
    agl::utl::Parameter<bool> mIsClearLightBuffer;
    agl::utl::Parameter<bool> mIsClearGBufferViewNrm;
    agl::utl::Parameter<bool> mIsClearGBufferViewDepth;
    agl::utl::Parameter<bool> mIsLppLight;
    agl::utl::Parameter<s32> mAntiAliasingType;
    agl::utl::Parameter<s32> mAntiAliasDetectEdgeQuality;
    agl::utl::Parameter<s32> mScreenWidthScale;
    agl::utl::Parameter<s32> mScreenHeightScale;
    agl::utl::Parameter<f32> mReduceQualityPercentage;
};

static_assert(sizeof(StressParam) == 0x190);

}  // namespace al
