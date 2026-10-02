#pragma once

#include <basis/seadTypes.h>

namespace al {
class CameraHolder;
class CameraPoser;
class PlacementId;
class PlayerWatcher;

/**
 * Switches the active camera poser by placement id, keeping lower priority requests in a pool
 * so they can be resumed once the higher priority camera ends.
 */
class CameraSwitcher {
public:
    /** Maximum number of camera requests kept in the pool. */
    static constexpr s32 cPoolCameraNum = 4;

    CameraSwitcher(CameraPoser** ppActivePoser, const CameraHolder* pHolder,
                   PlayerWatcher* pPlayerWatcher, bool* pIsResetPoser);

    void start(const PlacementId& rId, s32 interpoleFrame);
    void addPoolCameraList(const PlacementId& rId);
    void end(const PlacementId& rId, s32 interpoleFrame);
    void removePoolCameraList(const PlacementId& rId);
    bool tryStartCameraFromPoolCameraList();
    bool isCameraCurrent(const CameraPoser* pPoser) const;
    void offLookAtStop();

    CameraPoser* getCurrentPoser() const { return *mCurrentPoser; }

    bool isChanged() const { return mIsChanged; }

    s32 getInterpoleFrame() const { return mInterpoleFrame; }

private:
    void setInterpoleFrame(s32 interpoleFrame);

    const CameraHolder* mHolder;
    PlayerWatcher* mPlayerWatcher;
    CameraPoser** mActivePoser;
    CameraPoser** mCurrentPoser;
    const PlacementId** mPoolCameraIds;
    bool* mIsResetPoser;
    bool mIsChanged = false;
    s32 mInterpoleFrame = 60;

public:
    bool mIsRequestOffGyroMode = false;

private:
    bool mIsEndFollowCamera = false;
    bool mIsStartFollowCamera = false;
};

static_assert(sizeof(CameraSwitcher) == 0x40);

}  // namespace al
