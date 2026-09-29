#pragma once

namespace al {
class CameraRail;

/// Points a camera poser at a camera rail.
class CameraRailInfo {
public:
    CameraRailInfo();

    CameraRail* mRail;  // _0
    bool mIsValid;      // _8
};
}  // namespace al
