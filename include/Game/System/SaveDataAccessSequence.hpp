#pragma once

#include <basis/seadTypes.h>
#include "Library/Nerve/NerveExecutor.hpp"

class GameDataHolder;
class WindowProcessing;
class WindowSave;

namespace al {
class ErrorViewer;
class NetworkSystem;
class LayoutInitInfo;
} // namespace al

class SaveDataAccessSequence : public al::NerveExecutor {
  public:
    SaveDataAccessSequence(GameDataHolder* pHolder, al::ErrorViewer* pErrorViewer,
                           al::NetworkSystem* pNetworkSystem, const al::LayoutInitInfo& rInfo);
    bool update(bool isSuppressError);
    bool isDone() const;
    bool isWaitShowError() const;
    void startInit();
    void startInitSync();
    void startRead();
    void startReadSync();
    void startWrite(bool isSkipWaitWindowClose, int padPort);
    void startWriteNoMessage();
    void startWriteNoWindow(bool isSkipPlayingFile);
    void startWriteSync();
    void exeIdle();
    void exeInit();
    void exeReadGame();
    void exeReadGhost();
    int calcGhostSaveDataSize(int worldId) const;
    void exeWriteGame();
    void exeWriteGhost();
    void exeFlush();
    void exePreError();
    void exeError();
    void exeProcessEnd();
    void exeResult();
    void enableSave(bool enabled);
    bool isEnableHomeButtonMenu() const;
    bool isWindowProcessingActive() const;
    void makeSaveGhostWorldList();

    /**
     * @brief Check whether save operations are allowed.
     * @return True when saving has not been disabled.
     */
    bool isSaveEnabled() const { return !mSaveDisabled; }

    /**
     * @brief Mark the sequence as the development variant.
     */
    void setDevelop() { mDevelop = true; }

  private:
    GameDataHolder* mpHolder;            // 0x10
    s32 mPadPort;                        // 0x18 Pad port for the result window; -1 = auto.
    al::ErrorViewer* mpErrorViewer;      // 0x20
    al::NetworkSystem* mpNetworkSystem;  // 0x28
    s32 mUnknown30;                      // 0x30
    s32 mGhostWorldIndex;                // 0x34
    void* mUnknown38;                    // 0x38
    WindowProcessing* mpWindowProcessing; // 0x40
    WindowSave* mpWindowSave;            // 0x48
    s32* mpGhostWorldList;               // 0x50 One entry per world.
    s32 mUnknown58;                      // 0x58
    bool mIsSuppressError;               // 0x5c Set by update(); skips the error nerve.
    bool mSaveDisabled;                  // 0x5d
    bool mShowMessage;                   // 0x5e Show the result window after writing.
    bool mShowWindow;                    // 0x5f Show the processing window while writing.
    bool mDevelop;                       // 0x60 Development build: never touch save data.
    bool mIsSkipWaitWindowClose;         // 0x61 Finish without waiting for the window to close.
};
static_assert(sizeof(SaveDataAccessSequence) == 0x68);
