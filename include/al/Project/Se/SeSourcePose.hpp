#pragma once

#include <math/seadVector.h>
#include <prim/seadRuntimeTypeInfo.h>
#include <prim/seadSafeString.h>

namespace al {
class SeSourcePose {
    SEAD_RTTI_BASE(SeSourcePose)

public:
    SeSourcePose(const sead::SafeString& rName);

    virtual ~SeSourcePose() {}
    virtual void update() = 0;
};

class SeSourcePose3D : public SeSourcePose {
    SEAD_RTTI_OVERRIDE(SeSourcePose3D, SeSourcePose)

public:
    SeSourcePose3D(const sead::SafeString& rName);

    virtual const sead::Vector3f* get3DPosPtr() const = 0;
    virtual const sead::Vector3f& get3DPos() const;
};
}  // namespace al
