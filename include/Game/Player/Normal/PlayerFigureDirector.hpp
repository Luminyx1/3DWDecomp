#pragma once

#include <basis/seadTypes.h>

#include "Player/PlayerDef.hpp"

class IUsePlayerAudio;
class IUsePlayerFigureChangeObserver;

/// The player's current power-up and changes between them.
class PlayerFigureDirector {
public:
    PlayerFigureDirector(IUsePlayerAudio*, bool);

    void set(EPlayerFigure);
    void change(EPlayerFigure);
    void forceChange(EPlayerFigure);
    void lose();
    void update();
    const char* getFigureName() const;
    const char* getFigureName(EPlayerFigure) const;
    const char* getRealFigureName() const;
    const char* getOldFigureName();

    EPlayerFigure getFigure() const { return mFigure; }
    EPlayerFigure getNextFigure() const { return mNextFigure; }
    bool isNextFigureRequested() const { return mIsNextFigureRequested; }

    void setChangeObserver(IUsePlayerFigureChangeObserver* pObserver) {
        mChangeObserver = pObserver;
    }

private:
    EPlayerFigure mFigure;  // 0x0
    EPlayerFigure mNextFigure;  // 0x4
    EPlayerFigure mOldFigure;  // 0x8
    unsigned char _c[0x20 - 0xc];
    s32 mIsNextFigureRequested;  // 0x20
    unsigned char _24[0x28 - 0x24];
    IUsePlayerAudio* mAudio;  // 0x28
    IUsePlayerFigureChangeObserver* mChangeObserver;  // 0x30
    unsigned char _38[0x40 - 0x38];
};
