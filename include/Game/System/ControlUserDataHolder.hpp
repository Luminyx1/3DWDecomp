#pragma once
#include "System/ControlUserData.hpp"

namespace sead {
class ReadStream;
class WriteStream;
} // namespace sead

class GameDataHolder;

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
    void shufflePlayerModel(GameDataHolder* pHolder);
    void setPlayerModel(s32 userIndex, s32 characterType);
    void setPlayerFigureType(s32 userIndex, s32 figureType);
    s32 tryCalcControlUserIdByCharacterType(s32 characterType);
    bool readFromStream(sead::ReadStream* pStream);
    void writeToStream(sead::WriteStream* pStream, bool isSkip) const;
    void onSave();

  private:
    ControlUserData mUsers[4];
};
static_assert(sizeof(ControlUserDataHolder) == 0x60);
