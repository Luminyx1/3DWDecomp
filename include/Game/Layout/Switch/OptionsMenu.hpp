#pragma once

#include <basis/seadTypes.h>
#include "Library/Layout/LayoutActor.hpp"

namespace al {
class CameraDirector;
class CameraDirector_RS;
class LayoutInitInfo;
}  // namespace al

/**
 * @brief Options sub-menu of the pause menu (camera, controls and the Bowser Jr. assist demo).
 * @note Only the members used by already-decompiled callers are declared.
 */
class OptionsMenu : public al::LayoutActor {
public:
    OptionsMenu(const al::LayoutInitInfo& rInfo, al::CameraDirector* pCameraDirector,
                bool isKinopioBrigade);
    OptionsMenu(const al::LayoutInitInfo& rInfo, al::CameraDirector_RS* pCameraDirector);

    void appear(s32 port);
    void appearKoopaJrDemo(s32 port);
    bool isKoopaJrDemoEnd();
    bool isEnding();
    void forceExit();

    /**
     * @brief Access the Bowser's Fury camera director.
     * @return The camera director passed at construction.
     */
    al::CameraDirector_RS* getCameraDirectorRS() const { return mCameraDirectorRS; }

private:
    u8 mUnreconstructed128[0x38];
    al::CameraDirector_RS* mCameraDirectorRS;
    u8 mUnreconstructed168[0x8];
};
static_assert(sizeof(OptionsMenu) == 0x170);
