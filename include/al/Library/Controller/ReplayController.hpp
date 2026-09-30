#pragma once

#include <controller/seadControllerWrapper.h>

namespace al {
class IUsePadDataReader;
class IUsePadDataWriter;

class ReplayController : public sead::ControllerWrapper {
    SEAD_RTTI_OVERRIDE(ReplayController, sead::ControllerWrapper)

public:
    ReplayController(sead::Controller* pController);

    void unregist();
    void startReplay();
    void pauseReplay();
    void endReplay();
    bool isReplaying() const;
    bool isRecording() const;
    s32 getReplayRemainFrame() const;
    void calc(u32 prevHold, bool prevPointerOn) override;
    void startRecord();
    void endRecord();

    sead::Controller* getController() const { return mController; }
    void setPadDataReader(IUsePadDataReader* pReader) { mPadDataReader = pReader; }
    void setPadDataWriter(IUsePadDataWriter* pWriter) { mPadDataWriter = pWriter; }
    bool isValidPadReplay() const { return mIsValidPadReplay; }
    void setValidPadReplay(bool isValid) { mIsValidPadReplay = isValid; }
    bool isReadPadReplayData() const { return mIsReadPadReplayData; }

private:
    IUsePadDataReader* mPadDataReader = nullptr;
    bool mIsReplaying = false;
    IUsePadDataWriter* mPadDataWriter = nullptr;
    bool mIsRecording = false;
    bool mIsValidPadReplay = true;
    bool mIsReadPadReplayData = false;
};
}  // namespace al
