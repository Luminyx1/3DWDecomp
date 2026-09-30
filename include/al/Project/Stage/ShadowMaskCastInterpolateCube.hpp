#pragma once

#include <prim/seadSafeString.h>

#include "Library/Shadow/ShadowMaskBase.hpp"

namespace al {
class MtxConnector;

class ShadowMaskCastInterpolateCube : public ShadowMaskBase {
public:
    ShadowMaskCastInterpolateCube(const char* pName);

    void declare(ShadowMaskDrawCategory category) override;
    void update() override;
    void calcShadowMatrix(sead::Matrix34f* pMtx) override;
    void createMtxConnector() override;
    void readParam(const ByamlIter& rIter) override;
    void updateMulti() override;
    void addMulti() override;

    ShadowMaskType getShadowMaskType() const override {
        return ShadowMaskType::CastInterpolateCube;
    }

    MtxConnector* getTargetMtxConnector() const { return mTargetMtxConnector; }

    const sead::SafeString& getTargetJointName() const { return mTargetJointName; }

private:
    u8 _ec[0x148 - 0xec];
    MtxConnector* mTargetMtxConnector;
    sead::FixedSafeString<64> mTargetJointName;
    u8 _1a8[0x8];
};

static_assert(sizeof(ShadowMaskCastInterpolateCube) == 0x1b0);

}  // namespace al
