#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

#include "Library/Math/MatrixUtil.hpp"

namespace al {

class JointMtxPtr {
public:
    JointMtxPtr();

    explicit JointMtxPtr(const sead::Matrix34f* pMtx) { set(pMtx); }

    void setNull();
    void set(const sead::Matrix34f* pMtx);
    void set(const Matrix43f* pMtx);
    void getTranslation(sead::Vector3f* pOut) const;
    void calcMtxScale(sead::Vector3f* pOut) const;
    void copyTo(sead::Matrix34f* pOut) const;

    bool isValid() const { return mMtx34 != nullptr; }

private:
    union {
        const sead::Matrix34f* mMtx34;
        const Matrix43f* mMtx43;
    };

    bool mIsMatrix43;
};

}  // namespace al
