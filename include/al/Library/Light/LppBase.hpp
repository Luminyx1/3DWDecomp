#pragma once

#include "Library/Light/PrePassLightBase.hpp"

namespace al {

template <typename T>
class PrePassLight : public PrePassLightBase {
public:
    PrePassLight(const char* pName) : PrePassLightBase(pName) {}

    void init(const ActorInitInfo& rInfo) override {
        PrePassLightBase::init(rInfo);
        mParam.initByInfo(rInfo);
        declareLpp();
    }

    void execute() override {
        if (isActive()) {
            LppFunction::requestLpp(&mParam, this);
        }
    }

    void declareLpp() override { LppFunction::declareLpp(mParam, *this, 1); }

    void calcClippingInfo(sead::Vector3f* pPos, f32* pRadius) override {
        LppFunction::calcClippingInfoLpp(&mParam, this, pPos, pRadius);
    }

    void trySetupShadow(s32 view, PrePassLightKeeper* pKeeper, ShadowDirector* pDirector) override {
        if ((getLightType() == LppLightType::スポットライト ||
             getLightType() == LppLightType::投影 || getLightType() == LppLightType::正射影) &&
            isActive()) {
            LppFunction::trySetupShadowLpp(pKeeper, pDirector, &mParam, view);
        }
    }

    s32 getLightType() const override { return T::cLightType; }

    T mParam;
};

class LppPoint : public PrePassLight<LppPointParam> {
public:
    LppPoint(const char* pName) : PrePassLight(pName) {}
};

class LppLine : public PrePassLight<LppLineParam> {
public:
    LppLine(const char* pName) : PrePassLight(pName) {}
};

class LppSpot : public PrePassLight<LppSpotParam> {
public:
    LppSpot(const char* pName) : PrePassLight(pName) {}
};

class LppProj : public PrePassLight<LppProjParam> {
public:
    LppProj(const char* pName) : PrePassLight(pName) {}
};

class LppProjOrtho : public PrePassLight<LppProjOrthoParam> {
public:
    LppProjOrtho(const char* pName) : PrePassLight(pName) {}
};

static_assert(sizeof(LppPoint) == 0x118);
static_assert(sizeof(LppLine) == 0x118);
static_assert(sizeof(LppSpot) == 0x148);
static_assert(sizeof(LppProj) == 0x290);
static_assert(sizeof(LppProjOrtho) == 0x290);

}  // namespace al
