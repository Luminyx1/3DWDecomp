#pragma once

#include "Library/Nerve/NerveStateBase.hpp"
#include <math/seadQuat.h>

namespace al { class MtxConnector; }

struct ItemAssistRotateParam {
    int mSpinFrames;
    float mSpinSpeed;
    bool mUseLerp;
    float mEndSpeed;
    int mLerpFrames;
};

class ItemStateAssistRotate : public al::ActorStateBase {
public:
    ItemStateAssistRotate(al::LiveActor* pActor, const ItemAssistRotateParam* pParam);
    void init() override;
    void appear() override;
    void setRotateDegree(float degree);
    void exeSpin();
    void rotate(float speed);
    void exeLerp();

private:
    const ItemAssistRotateParam* mParam;
    al::MtxConnector* mConnector = nullptr;
    sead::Quatf mBaseQuat = sead::Quatf::unit;
    float* mRotateDegree = nullptr;
    float mLocalRotateDegree = 0.0f;
};
