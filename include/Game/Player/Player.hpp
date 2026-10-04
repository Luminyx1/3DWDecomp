#pragma once

#include <basis/seadTypes.h>

class IUsePlayerCeilingCheck;
class IUsePlayerCharaQuery;
class IUsePlayerCollision;
class IUsePlayerDamageInvalidCheck;
class IUsePlayerDashChecker;
class IUsePlayerLifeControl;
class IUsePlayerPropellerJumpPhase;
class PlayerActionGraph;
class PlayerActionGraphRestarter;
class PlayerEquipmentDirector;
class PlayerFigureDirector;
class PlayerGiantDirector;
class PlayerGigaDirector;
class PlayerInvincibleState;
class PlayerKiller;
struct PlayerProperty;

/// The player's action logic, shared by all the characters' actors.
class Player {
public:
    IUsePlayerDamageInvalidCheck* getDamageInvalidater() const;
    IUsePlayerCeilingCheck* getCeilingCheck();

    PlayerProperty* getProperty() const { return mProperty; }
    IUsePlayerCollision* getCollision() const { return mCollision; }
    PlayerFigureDirector* getFigureDirector() const { return mFigureDirector; }
    PlayerKiller* getKiller() const { return mKiller; }
    PlayerActionGraph* getActionGraph() const { return mActionGraph; }
    IUsePlayerLifeControl* getLifeControl() const { return mLifeControl; }
    PlayerInvincibleState* getInvincibleState() const { return mInvincibleState; }
    PlayerActionGraphRestarter* getRestarter() const { return mRestarter; }
    PlayerEquipmentDirector* getEquipmentDirector() const { return mEquipmentDirector; }
    IUsePlayerPropellerJumpPhase* getPropellerJumpPhase() const { return mPropellerJumpPhase; }
    const IUsePlayerCharaQuery* getCharaQuery() const { return mCharaQuery; }
    IUsePlayerDashChecker* getDashChecker() const { return mDashChecker; }
    PlayerGiantDirector* getGiantDirector() const { return mGiantDirector; }
    PlayerGigaDirector* getGigaDirector() const { return mGigaDirector; }

private:
    PlayerProperty* mProperty;  // 0x0
    u8 _8[0x28 - 0x8];
    IUsePlayerCollision* mCollision;  // 0x28
    u8 _30[0x88 - 0x30];
    PlayerFigureDirector* mFigureDirector;  // 0x88
    u8 _90[0x98 - 0x90];
    PlayerKiller* mKiller;  // 0x98
    PlayerActionGraph* mActionGraph;  // 0xa0
    u8 _a8[0xb0 - 0xa8];
    IUsePlayerLifeControl* mLifeControl;  // 0xb0
    PlayerInvincibleState* mInvincibleState;  // 0xb8
    u8 _c0[0x120 - 0xc0];
    PlayerActionGraphRestarter* mRestarter;  // 0x120
    u8 _128[0x160 - 0x128];
    PlayerEquipmentDirector* mEquipmentDirector;  // 0x160
    u8 _168[0x1b8 - 0x168];
    IUsePlayerPropellerJumpPhase* mPropellerJumpPhase;  // 0x1b8
    u8 _1c0[0x1d8 - 0x1c0];
    const IUsePlayerCharaQuery* mCharaQuery;  // 0x1d8
    u8 _1e0[0x208 - 0x1e0];
    IUsePlayerDashChecker* mDashChecker;  // 0x208
    u8 _210[0x280 - 0x210];
    PlayerGiantDirector* mGiantDirector;  // 0x280
    PlayerGigaDirector* mGigaDirector;  // 0x288
};
