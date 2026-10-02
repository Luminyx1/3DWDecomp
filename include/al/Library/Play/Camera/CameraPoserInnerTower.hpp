#pragma once

#include <math/seadVector.h>

#include "Library/Camera/CameraPoser_RS.hpp"

namespace al {
class CameraOffsetPreset;

class CameraPoserInnerTower : public CameraPoser_RS {
public:
    enum class Mode : s32 {
        AxisMoveFixTargetCenter = 0,
        AxisMoveFixLookDistance = 1,
        Follow = 2,
    };

    CameraPoserInnerTower(const char* pName);

    void init() override;
    void initByPlacementObj(const PlacementInfo& rInfo) override;
    void loadParam(const ByamlIter& rIter) override;
    void update() override;

    /**
     * Starts the camera by computing its pose right away.
     * @param rInfo Start info.
     */
    void start(const CameraStartInfo& rInfo) override { update(); }

private:
    sead::Vector3f mAxisPos = {0.0f, 0.0f, 0.0f};
    f32 mAxisHeight = 500.0f;
    f32 mDistance = 1400.0f;
    Mode mMode = Mode::AxisMoveFixTargetCenter;
    CameraOffsetPreset* mOffsetPreset = nullptr;
    bool mIsLimitFollowDistance = false;
    f32 mLimitFollowDistance = 2000.0f;
};

static_assert(sizeof(CameraPoserInnerTower) == 0x170);

}  // namespace al
