#pragma once
class GameDataHolder;
namespace SaveDataAccessFunction {
void startSaveDataInit(GameDataHolder* pHolder);
void startSaveDataInitSync(GameDataHolder* pHolder);
void startSaveDataReadSync(GameDataHolder* pHolder);
bool isWaitShowError(GameDataHolder* pHolder);
bool isEnableSave(const GameDataHolder* pHolder);
bool isEnableHomeButtonMenu(GameDataHolder* pHolder);
bool isWindowProcessingActive(GameDataHolder* pHolder);
void enableSave(GameDataHolder* pHolder, bool enabled);
bool updateSaveDataAccess(GameDataHolder* pHolder, bool option);
void startSaveDataRead(GameDataHolder* pHolder, bool wait);
void startSaveDataWriteNoMessage(GameDataHolder* pHolder, bool wait);
void startSaveDataWrite(GameDataHolder* pHolder, bool option, int fileId, bool wait);
void startSaveDataWriteNoWindow(GameDataHolder* pHolder, bool option, bool wait);
void startSaveDataWriteSync(GameDataHolder* pHolder, bool onlyWhenIdle);
} // namespace SaveDataAccessFunction
