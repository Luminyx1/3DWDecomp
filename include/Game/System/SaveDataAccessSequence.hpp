#pragma once
#include "Library/Nerve/NerveExecutor.hpp"
class SaveDataAccessSequence : public al::NerveExecutor {
  public:
    bool isDone() const;
    bool isWaitShowError() const;
    void startInit();
    void startInitSync();
    void startRead();
    void startReadSync();
    void startWrite(bool option, int fileId);
    void startWriteNoMessage();
    void startWriteNoWindow(bool option);
    void startWriteSync();
    bool update(bool option);
    void enableSave(bool enabled);
    bool isEnableHomeButtonMenu() const;
    bool isWindowProcessingActive() const;
    int calcGhostSaveDataSize(int worldId) const;
    void exeIdle();
    void makeSaveGhostWorldList();
    /**
     * @brief Check whether save operations are allowed.
     * @return True when saving has not been disabled.
     */
    bool isSaveEnabled() const { return !mSaveDisabled; }

  private:
    u8 mUnreconstructed10[0x4c]; // Save buffers, windows, error handling, and file selection.
    bool mUpdateOption;
    bool mSaveDisabled;
    bool mShowMessage;
    bool mShowWindow;
    bool mDevelop;
    bool mWriteOption;
};
static_assert(sizeof(SaveDataAccessSequence) == 0x68);
