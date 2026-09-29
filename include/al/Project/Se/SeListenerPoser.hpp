#pragma once

#include <math/seadMatrix.h>
#include <math/seadVector.h>
#include <prim/seadSafeString.h>

namespace al {
class ISeListenerParam;

class ISeListenerPoser {
public:
    virtual ~ISeListenerPoser() = default;
    virtual void calcListenerPose(sead::Matrix34f* pMtx, sead::Vector3f* pPos, const ISeListenerParam& rParam) = 0;
    virtual const sead::SafeString& getName() const = 0;
};

class SeListenerPoser : public ISeListenerPoser {
public:
    SeListenerPoser(const sead::SafeString& rName, const sead::SafeString& rUnused);

    ~SeListenerPoser() override {}

    const sead::SafeString& getName() const override { return mName; }

    static bool tryCalcViewMatrix(sead::Matrix34f* pMtx, const sead::Vector3f&, const sead::Vector3f&,
                                  const sead::Vector3f&);

    sead::SafeString mName;  // _8
};

class SeListenerPoserViewPos : public SeListenerPoser {
public:
    SeListenerPoserViewPos(const sead::SafeString& rName, const sead::SafeString& rUnused);

    void calcListenerPose(sead::Matrix34f* pMtx, sead::Vector3f* pPos, const ISeListenerParam& rParam) override;
};

class SeListenerPoserViewPosOffset : public SeListenerPoser {
public:
    SeListenerPoserViewPosOffset(const sead::SafeString& rName, const sead::SafeString& rUnused,
                                 const sead::Vector3f& rOffset);

    void calcListenerPose(sead::Matrix34f* pMtx, sead::Vector3f* pPos, const ISeListenerParam& rParam) override;

    sead::Vector3f mOffset;  // _18
};

class SeListenerPoserViewPosOffsetFovy : public SeListenerPoserViewPosOffset {
public:
    SeListenerPoserViewPosOffsetFovy(const sead::SafeString& rName, const sead::SafeString& rUnused);

    void calcListenerPose(sead::Matrix34f* pMtx, sead::Vector3f* pPos, const ISeListenerParam& rParam) override;
};
class SeListenerPoserMiddlePos : public SeListenerPoser {
public:
    SeListenerPoserMiddlePos(const sead::SafeString& rName, const sead::SafeString& rUnused,
                             f32 baseToMiddleRatio);

    void setBaseToMiddleRatio(f32 ratio);
    void calcListenerPose(sead::Matrix34f* pMtx, sead::Vector3f* pPos, const ISeListenerParam& rParam) override;

    f32 mBaseToMiddleRatio;  // _18
};

class SeListenerPoserAdjustMiddlePos : public SeListenerPoserMiddlePos {
public:
    SeListenerPoserAdjustMiddlePos(const sead::SafeString& rName, const sead::SafeString& rUnused);

    void calcListenerPose(sead::Matrix34f* pMtx, sead::Vector3f* pPos, const ISeListenerParam& rParam) override;

    f32 mLastFovyDegree = 0.0f;  // _1C
};
}  // namespace al
