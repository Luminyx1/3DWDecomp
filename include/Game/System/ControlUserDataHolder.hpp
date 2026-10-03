#pragma once
#include "System/ControlUserData.hpp"
class ControlUserDataHolder {
  public:
    explicit ControlUserDataHolder(bool singleMode);
    void initialize(bool singleMode);
    void copy(const ControlUserDataHolder* pOther);
    s32 calcPlayablePlayerNum() const;
    const ControlUserData* getControlUserData(s32 userIndex) const;
    ControlUserData* getControlUserDataPtr(s32 userIndex);
    void recoverGameOver();
    void resetFigureType();
    void resetPlayerModel();
    void startCharacterSelect();
    bool entryPlayer(s32 userIndex, s32 characterType);
    void retirePlayer(s32 userIndex);
    void resetPlayerAll();
    void setPlayerModel(s32 userIndex, s32 characterType);
    void setPlayerFigureType(s32 userIndex, s32 figureType);
    void onSave();

  private:
    ControlUserData mUsers[4];
};
static_assert(sizeof(ControlUserDataHolder) == 0x60);
