#pragma once

class IUsePlayerCollision;
class IUsePlayerLifeControl;
class IUsePlayerModelChanger;
class IUsePlayerSubAction;
class Player;
class PlayerActionGraph;

/// Answers questions about what the player's action graph is currently doing.
class PlayerActionObserver {
public:
    PlayerActionObserver(const Player*, PlayerActionGraph*, IUsePlayerCollision*,
                         const IUsePlayerModelChanger*, const IUsePlayerSubAction*,
                         const IUsePlayerLifeControl*);

    void update();

    virtual bool isOnGround() const;
    virtual bool isDead() const;
    virtual bool isAbyss() const;
    virtual bool isJumpTrig() const;
    virtual bool isInBind() const;
    virtual bool isDeadDemo() const;
    virtual bool isChangeDemo() const;
    virtual bool isHipDropping() const;
    virtual bool isRollingOnGround() const;
    virtual bool isWaterAction() const;
    virtual bool isWaterSurfaceAction() const;
    virtual bool isFallBrakeTrig() const;
    virtual bool isWallAction() const;
    virtual bool isWallSnapAction() const;
    virtual bool isChangingStatue() const;
    virtual bool isSquatWalkAction() const;
    virtual bool isSquatAction() const;
    virtual bool isPossibleToWatch() const;
    virtual bool isSliding() const;
    virtual bool isTossableAction() const;
    virtual bool isWaitOrPivot() const;
    virtual bool isTurn() const;
};
