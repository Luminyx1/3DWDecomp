#pragma once

#include <gfx/seadColor.h>
#include "utility/aglParameter.h"
#include "utility/aglParameterObj.h"

namespace al {
    class FogParam {
    public:
        FogParam() = default;

        virtual void init();
        virtual f32 getStart() const { return *mStart; }
        virtual f32 getEnd() const { return *mEnd; }

        void initSystem();
        bool operator==(const FogParam& rOther) const;
        FogParam& operator=(const FogParam& rOther);
        void interp(const FogParam& rA, const FogParam& rB, f32 rate);

        agl::utl::IParameterObj* getParamObj() { return &mParamObj; }

        agl::utl::ParameterObj mParamObj;
        agl::utl::Parameter<sead::Color4f> mColor;
        agl::utl::Parameter<sead::Color4f> mMulColor;
        agl::utl::Parameter<f32> mIntensityMax;
        agl::utl::Parameter<f32> mStart;
        agl::utl::Parameter<f32> mEnd;
    };

    static_assert(sizeof(FogParam) == 0xE8);
};  // namespace al
