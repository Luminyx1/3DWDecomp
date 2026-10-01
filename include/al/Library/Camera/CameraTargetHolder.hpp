#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>

namespace al {
class CameraSubTargetBase;
class CameraTargetBase;
class PlayerHolder;

using CameraTargetArray = sead::PtrArray<CameraTargetBase>;
using CameraSubTargetArray = sead::PtrArray<CameraSubTargetBase>;

struct ViewTargetInfo {
    CameraTargetBase* target = nullptr;
    s8 hasTargetChanged = false;
};

static_assert(sizeof(ViewTargetInfo) == 0x10);

struct ViewSubTargetInfo {
    CameraSubTargetBase* target = nullptr;
    s8 hasTargetChanged = false;
};

class CameraTargetHolder {
public:
    CameraTargetHolder(s32 maxTargets);

    void initAfterPlacement(const PlayerHolder* pPlayerHolder);
    CameraTargetBase* tryGetViewTarget(s32 index) const;
    void update();
    s32 tryFindIndex(const CameraTargetBase* pTarget,
                     const CameraTargetArray& rArray);
    s32 tryFindIndex(const CameraSubTargetBase* pTarget,
                     const CameraSubTargetArray& rArray);
    bool tryRemovePtr(const CameraTargetBase* pTarget, CameraTargetArray& rArray);
    bool tryRemovePtr(const CameraSubTargetBase* pTarget,
                      CameraSubTargetArray& rArray);
    void addTarget(CameraTargetBase* pTarget);
    void removeTarget(CameraTargetBase* pTarget);
    CameraTargetBase* getViewTarget(s32 index) const;
    bool isChangeViewTarget(s32 index) const;
    CameraSubTargetBase* getTopSubTarget() const;
    void addSubTarget(CameraSubTargetBase* pTarget);
    void removeSubTarget(CameraSubTargetBase* pTarget);
    void addPlacementSubTarget(CameraSubTargetBase* pTarget);
    void removePlacementSubTarget(CameraSubTargetBase* pTarget);

    const ViewSubTargetInfo& getTopSubTargetInfo() const { return mTopSubTargetInfo; }

    void setViewTarget(CameraTargetBase* pTarget, s32 index) { mViewTargetArray[index] = pTarget; }

private:
    s32 mViewTargetSize = 0;
    CameraTargetBase** mViewTargetArray = nullptr;
    ViewTargetInfo* mViewTargetInfo = nullptr;
    CameraTargetArray mTargetArray;
    ViewSubTargetInfo mTopSubTargetInfo;
    CameraSubTargetArray mSubTargetArray;
    CameraSubTargetArray mPlacementSubTargetArray;
    const PlayerHolder* mPlayerHolder = nullptr;
    bool _60 = false;
};

static_assert(sizeof(CameraTargetHolder) == 0x68);

}  // namespace al
