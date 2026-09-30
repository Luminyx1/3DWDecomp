#pragma once

#include "Library/LiveActor/LiveActor.hpp"

namespace al {
class CollisionParts;
class CollisionPartsConnector;

class FootPrint : public LiveActor {
public:
    FootPrint(const ActorInitInfo& rInfo, const char* pArchiveName);

    void appear() override;
    void control() override;

    void startDisappear();
    bool isDisappear() const;
    void exeAppear();
    void exeDisappear();
    void setAnimationByMaterial(const char* pAnimName);
    void setAnimationByCharacter(const char* pAnimName);
    void setAnimationByMetamorphosis(const char* pAnimName);
    void setFollowCollisionParts(const CollisionParts* pParts);

    CollisionPartsConnector* mConnector = nullptr;
    const CollisionParts* mCollisionParts = nullptr;
    void* _158 = nullptr;
    s32 _160 = 0;
    const char* mMclAnimName = nullptr;
};

static_assert(sizeof(FootPrint) == 0x170);
}  // namespace al
