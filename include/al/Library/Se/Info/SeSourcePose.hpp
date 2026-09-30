#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
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
    virtual const sead::Vector3f& get3DPos() const { return *get3DPosPtr(); }
};

class SeSourcePose3DMtxBase : public SeSourcePose3D {
    SEAD_RTTI_OVERRIDE(SeSourcePose3DMtxBase, SeSourcePose3D)

public:
    SeSourcePose3DMtxBase(const sead::SafeString& rName);

    virtual const sead::Matrix34f* get3DMtxPtr() const = 0;
    virtual const sead::Matrix34f& get3DMtx() const { return *get3DMtxPtr(); }
};

class SeSourcePose3DMtxOffsetPtr : public SeSourcePose3DMtxBase {
    SEAD_RTTI_OVERRIDE(SeSourcePose3DMtxOffsetPtr, SeSourcePose3DMtxBase)

public:
    SeSourcePose3DMtxOffsetPtr(const sead::Matrix34f* pMtx, const sead::Vector3f* pOffset);

    void update() override;
    const sead::Vector3f* get3DPosPtr() const override { return &mPos; }
    const sead::Matrix34f* get3DMtxPtr() const override { return &mMtx; }
    const sead::Matrix34f& get3DMtx() const override { return mMtx; }

private:
    const sead::Matrix34f* mMtxPtr;
    const sead::Vector3f* mOffset;
    sead::Matrix34f mMtx;
    sead::Vector3f mPos = {0.0f, 0.0f, 0.0f};
};

static_assert(sizeof(SeSourcePose3DMtxOffsetPtr) == 0x58);

class SeSourcePose3DMtxPtr : public SeSourcePose3DMtxBase {
    SEAD_RTTI_OVERRIDE(SeSourcePose3DMtxPtr, SeSourcePose3DMtxBase)

public:
    SeSourcePose3DMtxPtr(const sead::Matrix34f* pMtx);

    void update() override;
    const sead::Vector3f* get3DPosPtr() const override { return &mPos; }
    const sead::Matrix34f* get3DMtxPtr() const override { return mMtxPtr; }
    const sead::Matrix34f& get3DMtx() const override { return *get3DMtxPtr(); }

private:
    const sead::Matrix34f* mMtxPtr;
    sead::Vector3f mPos = {0.0f, 0.0f, 0.0f};
};

static_assert(sizeof(SeSourcePose3DMtxPtr) == 0x20);

class SeSourcePose3DPosPtr : public SeSourcePose3D {
    SEAD_RTTI_OVERRIDE(SeSourcePose3DPosPtr, SeSourcePose3D)

public:
    SeSourcePose3DPosPtr(const sead::Vector3f* pPos);

    void update() override;
    const sead::Vector3f* get3DPosPtr() const override { return mPosPtr; }

private:
    const sead::Vector3f* mPosPtr;
};

static_assert(sizeof(SeSourcePose3DPosPtr) == 0x10);

class SeSourcePoseNull : public SeSourcePose {
    SEAD_RTTI_OVERRIDE(SeSourcePoseNull, SeSourcePose)

public:
    SeSourcePoseNull();

    void update() override;
};

static_assert(sizeof(SeSourcePoseNull) == 0x8);
}  // namespace al
