#pragma once

#include "Project/Block/BlockRailShape.hpp"

namespace al {
class BlockRailShapeTerminater : public BlockRailShape {
public:
    BlockRailShapeTerminater(const char* pName);

    void init(const sead::Quatf& rQuat, const sead::Vector3f& rTrans,
              const ByamlIter& rIter) override;
    void calcPos(sead::Vector3f* pPos, f32 rate) const override;
    void calcDir(sead::Vector3f* pDir, f32 rate) const override;
    void calcNearestParam(sead::Vector3f* pPos, f32* pRate,
                          const sead::Vector3f& rPos) const override;
    bool isTerminate() const override;
    void calcOffset(const sead::Vector3f& rBaseTrans) override;
    void updateLinkedTrans(const sead::Vector3f& rBaseTrans) override;

    sead::Vector3f mPos = sead::Vector3f::zero;
    sead::Vector3f mDir = sead::Vector3f::ez;
};

static_assert(sizeof(BlockRailShapeTerminater) == 0x38);
}  // namespace al
