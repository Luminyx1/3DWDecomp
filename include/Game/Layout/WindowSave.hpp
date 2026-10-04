#pragma once

#include <basis/seadTypes.h>

namespace al {
class LayoutInitInfo;
} // namespace al

/**
 * @brief Save-result window.
 *
 * Only the parts used by already-decompiled code are modelled; the al::LayoutActor base
 * (with its virtual bases) is kept opaque.
 */
class WindowSave {
  public:
    WindowSave(const al::LayoutInitInfo& rInfo);
    void appearWindow(int padPort);

    /**
     * @brief Check whether the window layout is currently shown.
     * @return True while the window is alive.
     */
    bool isAlive() const { return mIsAlive; }

  private:
    u8 mLayoutActor[0x120]; // al::LayoutActor base (opaque).
    bool mIsAlive;
    u8 mUnreconstructed121[0x17];
};
static_assert(sizeof(WindowSave) == 0x138);
