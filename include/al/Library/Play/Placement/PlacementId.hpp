#pragma once

#include <basis/seadTypes.h>

namespace al {
struct PlacementInfo;

class PlacementId {
public:
    PlacementId();
    PlacementId(const char* pId, const char* pLayerConfigName, const char* pUnitConfigName,
                const char* pZoneId);

    bool init(const PlacementInfo& rInfo);
    bool isEqual(const PlacementId& rOther) const;
    static bool isEqual(const PlacementId& rId, const PlacementId& rOther);

    const char* mPlacementID = nullptr;
    const char* mLayerConfigName = nullptr;
    const char* mUnitConfigName = nullptr;
    const char* mZoneID = nullptr;
    const char* mCommonID = nullptr;
};
}  // namespace al
