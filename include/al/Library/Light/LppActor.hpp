#pragma once

#include "Library/Light/LppBase.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Project/Light/ActorPrePassLightKeeper.hpp"

namespace al {

template <typename T>
class PrePassLightPlacementBase : public LiveActor {
public:
    PrePassLightPlacementBase(const char* pName) : LiveActor(pName), mLight(new T(pName)) {}

    ~PrePassLightPlacementBase() override {
        if (mLight != nullptr) {
            delete mLight;
            mLight = nullptr;
        }
    }

    void init(const ActorInitInfo& rInfo) override {
        mLight->init(rInfo);
        LppFunction::initLppActor(this, mLight, &mBaseMtx, rInfo);
        PrePassLightPlacementFuncImpl::makeMtxSRT(&mBaseMtx, this);
        mLight->calcClippingInfo(&mClippingPos, &mClippingRadius);
        PrePassLightPlacementFuncImpl::setClippingInfoImpl(this, mClippingRadius, &mClippingPos);
    }

    void makeActorAppeared() override {
        LiveActor::makeActorAppeared();
        mLight->appear();
    }

    void makeActorDead() override {
        LiveActor::makeActorDead();
        mLight->requestKill();
    }

    void startClipped() override {
        LiveActor::startClipped();
        mLight->requestKill();
    }

    void endClipped() override {
        LiveActor::endClipped();
        mLight->appear();
    }

    void control() override {
        PrePassLightPlacementFuncImpl::makeMtxSRT(&mBaseMtx, this);
        mLight->calcClippingInfo(&mClippingPos, &mClippingRadius);
    }

    T* getLight() const { return mLight; }

protected:
    T* mLight;
    sead::Matrix34f mBaseMtx;
    sead::Vector3f mClippingPos = {0.0f, 0.0f, 0.0f};
    f32 mClippingRadius = 100.0f;
};

}  // namespace al
