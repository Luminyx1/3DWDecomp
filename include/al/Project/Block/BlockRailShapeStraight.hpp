#pragma once

#include "Project/Block/BlockRailShape.hpp"

namespace al {
class BlockRailShapeStraight : public BlockRailShape {
public:
    BlockRailShapeStraight(const char* pName);

    void init(const sead::Quatf& rQuat, const sead::Vector3f& rTrans,
              const ByamlIter& rIter) override;
    bool isRide(f32* pRate, const sead::Vector3f& rPrevPos,
                const sead::Vector3f& rPos) const override;
    f32 getTotalLength() const override { return mLength; }
    void calcPos(sead::Vector3f* pPos, f32 rate) const override;
    void calcDir(sead::Vector3f* pDir, f32 rate) const override;
    void calcNearestParam(sead::Vector3f* pPos, f32* pRate,
                          const sead::Vector3f& rPos) const override;
    void calcOffset(const sead::Vector3f& rBaseTrans) override;
    void updateLinkedTrans(const sead::Vector3f& rBaseTrans) override;

    sead::Vector3f mDir = sead::Vector3f::ez;
    sead::Vector3f mUpDir = sead::Vector3f::ey;
    sead::Vector3f mStartPos = sead::Vector3f::zero;
    f32 mLength = 800.0f;
};

static_assert(sizeof(BlockRailShapeStraight) == 0x48);

class BlockRailShapeCurve : public BlockRailShape {
public:
    BlockRailShapeCurve(const char* pName);

    void init(const sead::Quatf& rQuat, const sead::Vector3f& rTrans,
              const ByamlIter& rIter) override;
    bool isRide(f32* pRate, const sead::Vector3f& rPrevPos,
                const sead::Vector3f& rPos) const override;
    f32 getTotalLength() const override { return mLength; }
    void calcPos(sead::Vector3f* pPos, f32 rate) const override;
    void calcDir(sead::Vector3f* pDir, f32 rate) const override;
    void calcNearestParam(sead::Vector3f* pPos, f32* pRate,
                          const sead::Vector3f& rPos) const override;
    void calcOffset(const sead::Vector3f& rBaseTrans) override;
    void updateLinkedTrans(const sead::Vector3f& rBaseTrans) override;

    sead::Vector3f mCenter = sead::Vector3f::zero;
    sead::Vector3f mSideAxis = sead::Vector3f::ex;
    sead::Vector3f mUpAxis = sead::Vector3f::ey;
    sead::Vector3f mFrontAxis = sead::Vector3f::ez;
    f32 mRadius = 600.0f;
    f32 mLength = 942.47784f;
};

static_assert(sizeof(BlockRailShapeCurve) == 0x58);
}  // namespace al
