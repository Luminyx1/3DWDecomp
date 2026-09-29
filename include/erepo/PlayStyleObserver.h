#pragma once

#include <container/seadSafeArray.h>

#include <erepo/Data/AtomicBitFlag.h>
#include <erepo/ObserverBase.h>

namespace erepo {

class PlayStyleObserver : public ObserverBase {
public:
    SEAD_ENUM(EFlag, cUpdated)

    PlayStyleObserver();

    void initialize(sead::Heap* pHeap) override;
    const char* getName() const override { return "PlayStyle"; }
    void load() override;
    void save(SaveData* pData) const override;
    void update(const Manager::UpdateArg& rArg) override;
    bool report(const StringId& rId) override;

private:
    struct UseInfo {
        f32 time;
        f32 nonActiveTime;
    };

    using UseInfoTable = sead::SafeArray<sead::SafeArray<UseInfo, 5>, 3>;

    void clearAllUseInfo_();
    void checkCurrentStyle_(const Manager::UpdateArg& rArg, bool isForce);
    bool isControllerActive_() const;

    UseInfoTable mUseInfo;
    UseInfoTable mSavedUseInfo;
    EPlayStyle mPlayStyle = EPlayStyle::Unknown;
    EControllerStyle mControllerStyle = EControllerStyle::Unknown;
    u32 _100 = 0;
    AtomicBitFlag<EFlag> mFlags;
};

}  // namespace erepo
