#pragma once

#include <gfx/seadColor.h>
#include <utility/aglParameter.h>
#include <utility/aglParameterObj.h>

namespace al {

class LightStreakParam {
public:
    void init();
    void initSystem();
    bool isEnable() const;
    bool operator==(const LightStreakParam& rOther) const;
    LightStreakParam& operator=(const LightStreakParam& rOther);
    void interp(const LightStreakParam& rA, const LightStreakParam& rB, f32 rate);

    agl::utl::IParameterObj* getParamObj() { return &mParamObj; }

    f32 getIntensity() const { return *mIntensity; }

    void setIntensity(f32 intensity) { *mIntensity = intensity; }

    f32 getStreakScale() const { return *mStreakScale; }

    f32 getAttn() const { return *mAttn; }

    f32 getThreshold() const { return *mThreshold; }

    f32 getRotateDegree() const { return *mRotateDegree; }

    s32 getStreakType() const { return *mStreakType; }

    s32 getPassNum() const { return *mPassNum; }

    const sead::Color4f& getStreakColor1() const { return *mStreakColor1; }

    const sead::Color4f& getStreakColor2() const { return *mStreakColor2; }

    const sead::Color4f& getStreakColor3() const { return *mStreakColor3; }

private:
    agl::utl::ParameterObj mParamObj;
    agl::utl::Parameter<f32> mIntensity;
    agl::utl::Parameter<f32> mStreakScale;
    agl::utl::Parameter<f32> mAttn;
    agl::utl::Parameter<f32> mThreshold;
    agl::utl::Parameter<f32> mRotateDegree;
    agl::utl::Parameter<s32> mStreakType;
    agl::utl::Parameter<s32> mPassNum;
    agl::utl::Parameter<sead::Color4f> mStreakColor1;
    agl::utl::Parameter<sead::Color4f> mStreakColor2;
    agl::utl::Parameter<sead::Color4f> mStreakColor3;
};

static_assert(sizeof(LightStreakParam) == 0x188);

}  // namespace al
