#pragma once

#include <basis/seadTypes.h>

namespace al {
class LayoutInitInfo;
} // namespace al

/**
 * @brief "Now saving..." style system-message window.
 *
 * Only the parts used by already-decompiled code are modelled; the al::LayoutActor base
 * (with its virtual bases) is kept opaque.
 */
class WindowProcessing {
  public:
    WindowProcessing(const al::LayoutInitInfo& rInfo, const char* pName);
    void appearWithSystemMessage(const char* pCategory, const char* pLabel, int minFrame,
                                 bool isUseSound);
    bool isEnd() const;

    /**
     * @brief Check whether the window layout is currently shown.
     * @return True while the window is alive.
     */
    bool isAlive() const { return mIsAlive; }

    /**
     * @brief Ask the window to close once its minimum display time has elapsed.
     */
    void requestClose() { mIsRequestClose = true; }

  private:
    u8 mLayoutActor[0x120]; // al::LayoutActor base (opaque).
    bool mIsAlive;
    bool mIsRequestClose;
    u8 mUnreconstructed122[0xe];
};
static_assert(sizeof(WindowProcessing) == 0x130);
