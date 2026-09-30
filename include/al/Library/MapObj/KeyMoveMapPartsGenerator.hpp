#pragma once

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
template <class T>
class DeriveActorGroup;
class KeyMoveMapParts;

class KeyMoveMapPartsGenerator : public LiveActor {
public:
    KeyMoveMapPartsGenerator(const char* pName);

    void init(const ActorInitInfo& rInfo) override;

    void exeDelay();
    void exeGenerate();

    DeriveActorGroup<KeyMoveMapParts>* mKeyMoveMapPartsGroup = nullptr;
    sead::Vector3f mClippingTrans = sead::Vector3f::zero;
    s32 mDelayTime = 0;
    s32 mGenerateInterval = 60;
};
}  // namespace al
