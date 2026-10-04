#pragma once

#include <prim/seadEnum.h>

/// Playable characters, in the order of rc::getPlayerCharacterNameTrue.
SEAD_ENUM(EPlayerChara, Mario, Luigi, Peach, Kinopio, Rosetta, KinopioBrigade, KinopioBrigadeMember,
          KinopioBrigadeMember1, KinopioBrigadeMember2)

/// Power-up forms, named after PlayerFigureDirector::getRealFigureName.
SEAD_ENUM(EPlayerFigure, Super, Mini, Fire, Climb, RaccoonDog, Boomerang, RaccoonDogWhite, Manekineko,
          ClimbWhite, ClimbGiga)

/// Items the player can hold or wear (propeller box, ...).
enum EPlayerEquipmentType : unsigned long {};

/// Actions an equipped item provides.
enum EPlayerEquipmentAction {};
