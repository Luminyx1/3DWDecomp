#pragma once

/// The player's life: damage, death and revival (implemented by PlayerLifeControl).
class IUsePlayerLifeControl {
public:
    virtual void damage() = 0;
    virtual void forceVanish() = 0;
    virtual void startDamageInvalidTimer() = 0;
    virtual bool isDying() const = 0;
    virtual bool isVanishDying() const = 0;
    virtual void revive() = 0;
};
