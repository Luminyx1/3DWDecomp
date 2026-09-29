#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace al {
class ISeListenerParam {
public:
    virtual const sead::Vector3f& getViewPos() const = 0;
    virtual const sead::Matrix34f& getViewMatrix() const = 0;
    virtual f32 getFovyDegree() const = 0;
    virtual const sead::Vector3f& getTargetPos() const = 0;

    void calcLookAtDirNormFromViewMatrix(sead::Vector3f* pDir) const;
    void calcUpDirNormFromViewMatrix(sead::Vector3f* pDir) const;
};
}  // namespace al
