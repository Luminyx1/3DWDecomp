#pragma once

#include <container/seadPtrArray.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>
#include <prim/seadSafeString.h>

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

class ISeListenerPoser {
public:
    virtual ~ISeListenerPoser() {}
    virtual void calcListenerPose(sead::Matrix34f* pMtx, sead::Vector3f* pPos, const ISeListenerParam& rParam) = 0;
    virtual const sead::SafeString& getName() const = 0;
};

class SeListener {
public:
    SeListener(s32 poserNum);
    void addPoser(ISeListenerPoser* pPoser);
    void setCurrentPoserIndex(s32 index);
    void setCurrentPoser(const sead::SafeString& rName);
    void calcListenerMatrix(const ISeListenerParam& rParam);
    ISeListenerPoser* getCurrentPoser() const;

    const sead::Matrix34f& getListenerMatrix() const { return mListenerMatrix; }
    const sead::Vector3f& getListenerPos() const { return mListenerPos; }

private:
    sead::PtrArray<ISeListenerPoser> mPosers;
    s32 mCurrentPoserIndex = 0;
    sead::Matrix34f mListenerMatrix;
    sead::Vector3f mListenerPos;
};

static_assert(sizeof(SeListener) == 0x50);

}  // namespace al
