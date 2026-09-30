#pragma once

#include "Project/AreaObj/AreaObj.hpp"

namespace al {

class CameraStartParamArea : public AreaObj {
public:
    bool isValidParam() const { return mIsValidParam; }

    bool isOneTime() const { return mIsOneTime; }

    void invalidateParam() { mIsValidParam = false; }

    const f32* getAngleH() const { return mAngleH; }

    const f32* getAngleV() const { return mAngleV; }

private:
    bool mIsValidParam;
    bool mIsOneTime;
    const f32* mAngleH;
    const f32* mAngleV;
};

}  // namespace al
