#pragma once

#include <basis/seadTypes.h>

namespace al {
struct PlacementInfo;

class PlacementHolder {
public:
    PlacementHolder();
    PlacementHolder(const char* pId, const char* pUnitConfigName, const char* pZoneId);

    bool init(const PlacementInfo& rInfo);
    void copyFromParent(const PlacementHolder& rParent);
    bool isEqual(const PlacementHolder& rOther) const;

    static bool isEqual(const PlacementHolder& rHolder, const PlacementHolder& rOther);

    const char* getId() const { return mId; }
    const char* getUnitConfigName() const { return mUnitConfigName; }
    const char* getZoneId() const { return mZoneId; }
    const char* getCommonId() const { return mCommonId; }
    s32 getZoneNo() const { return mZoneNo; }
    s32 getLayerId() const { return mLayerId; }

private:
    const char* mId = nullptr;
    const char* mUnitConfigName = nullptr;
    const char* mZoneId = nullptr;
    const char* mCommonId = nullptr;
    s32 mZoneNo;
    s32 mLayerId = -1;
};

static_assert(sizeof(PlacementHolder) == 0x28);
}  // namespace al
