#pragma once

#include "Library/Layout/LayoutActor.hpp"

namespace al {
class LayoutInitInfo;
}

/** @brief System-message window displayed while an operation is in progress. */
class WindowProcessing : public al::LayoutActor {
public:
    WindowProcessing(const al::LayoutInitInfo& rInfo, const char* pName);
    void appearWithSystemMessage(const char* pCategory, const char* pLabel, int minFrame,
                                 bool isUseSound);
    bool isEnd() const;
    void exeAppear();
    void exeWait();
    void exeEnd();

    /** @brief Requests closure after the minimum display time has elapsed. */
    void requestClose() { mIsRequestClose = true; }

private:
    bool mIsRequestClose = false;
    s32 mMinFrame = 0;
    bool mIsUseSound = true;
};
static_assert(sizeof(WindowProcessing) == 0x130);
