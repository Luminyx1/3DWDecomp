#pragma once

/// Playable characters.
enum EPlayerChara {};

/// Power-up forms, named after PlayerFigureDirector::getFigureName.
enum EPlayerFigure {
    EPlayerFigure_Super = 0,
    EPlayerFigure_Mini = 1,
    EPlayerFigure_Fire = 2,
    EPlayerFigure_Climb = 3,
    EPlayerFigure_RaccoonDog = 4,
    EPlayerFigure_Boomerang = 5,
    EPlayerFigure_RaccoonDogWhite = 6,
    EPlayerFigure_Manekineko = 7,
};

/// Items the player can hold or wear (propeller box, ...).
enum EPlayerEquipmentType : unsigned long {};

/// Actions an equipped item provides.
enum EPlayerEquipmentAction {};
