#pragma once

#include "Library/Layout/LayoutActionFunction.hpp"
#include "Library/Layout/LayoutActor.hpp"

namespace al {
class LayoutInitInfo;

class WipeSimple : public LayoutActor {
public:
    WipeSimple(const char* pName, const char* pLayoutName, const LayoutInitInfo& rInfo,
               const char* pArchiveName);

    void startClose(s32 frames);
    void tryStartClose(s32 frames);
    void startCloseEnd();
    void startOpen(s32 frames);
    void startOpenDelay(s32 delay, s32 frames);
    void tryStartOpen(s32 frames);
    bool isCloseEnd() const;

    void exeClose();
    void exeCloseEnd();
    void exeOpen();
    void exeDelayOpen();

    s32 getWipeFrameNum() const;
    void appear() override;

private:
    void updateFrameRate() {
        if (mFrames <= 0) {
            setActionFrameRate(this, 1.0f);
        } else {
            setActionFrameRate(this, getActionFrameMax(this, nullptr) / mFrames);
        }
    }

    s32 mFrames = -1;
    s32 mDelay = -1;
};
}  // namespace al
