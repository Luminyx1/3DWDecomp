#pragma once

#include <basis/seadTypes.h>

#include "Player/PlayerDef.hpp"

class IUsePlayerAudio;

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

private:
    EPlayerFigure mFigure;  // 0x0
    s32 _4;
    EPlayerFigure mOldFigure;  // 0x8
    unsigned char _c[0x1c];
    IUsePlayerAudio* mAudio;  // 0x28
};
