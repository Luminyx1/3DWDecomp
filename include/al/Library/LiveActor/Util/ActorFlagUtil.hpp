#pragma once

namespace al {
    class LiveActor;

    bool isAlive(const LiveActor*);
    bool isDead(const LiveActor*);
    bool isDeadAlive(LiveActor*);
    bool isCountAsAlive(LiveActor*);
    bool isNoCollide(const LiveActor*);
    void onCalcAnim(LiveActor*);
    void offCalcAnim(LiveActor*);
    void validateShadow(LiveActor*);
    void invalidateShadow(LiveActor*);
    void onCollide(LiveActor*);
    void offCollide(LiveActor*);
    void validateMaterialCode(LiveActor*);
    void validateCeilWallFloorMaterialCode(LiveActor*);
    bool isAreaTarget(const LiveActor*);
    void onAreaTarget(LiveActor*);
    void offAreaTarget(LiveActor*);
    bool isUpdateMovementEffectAudioCollision(const LiveActor*);
    void onUpdateMovementEffectAudioCollision(LiveActor*);
    void offUpdateMovementEffectAudioCollision(LiveActor*);
};  // namespace al
