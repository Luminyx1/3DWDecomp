#pragma once

#include "Library/Fog/FogParam.hpp"

namespace sead {
    class Camera;
}

namespace al {
    class YFogParam : public FogParam {
    public:
        YFogParam() = default;

        void init() override;
        f32 getStart() const override { return *mStart + mCameraYPos; }
        f32 getEnd() const override { return *mEnd + mCameraYPos; }

        bool operator==(const YFogParam& rOther) const;
        YFogParam& operator=(const YFogParam& rOther);
        void interp(const YFogParam& rA, const YFogParam& rB, f32 rate);
        void trySetCameraYPos(const sead::Camera* pCamera);

        void validateCameraYPos() { mIsValidCameraYPos = true; }

        void setCameraYPos(f32 y) { mCameraYPos = y; }

        agl::utl::Parameter<bool> mIsFollowCamera;
        bool mIsValidCameraYPos;
        f32 mCameraYPos;
    };

    static_assert(sizeof(YFogParam) == 0x110);
};  // namespace al
