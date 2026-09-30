#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Math/MatrixUtil.hpp"

namespace al {

class JointMtxPtr {
public:
    JointMtxPtr();

    void setNull();
    void set(const sead::Matrix34f* pMtx);
    void set(const Matrix43f* pMtx);
    void getTranslation(sead::Vector3f* pOut) const;
    void calcMtxScale(sead::Vector3f* pOut) const;
    void copyTo(sead::Matrix34f* pOut) const;

private:
    const void* mMtx;
    bool mIsMatrix43;
};

}  // namespace al
