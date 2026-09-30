#pragma once

#include <basis/seadTypes.h>

namespace al {
class AreaObj;
class AreaObjDirector;
class AreaObjGroup;
class PlayerHolder;

class ClippingFarAreaObserver {
public:
    ClippingFarAreaObserver(const AreaObjDirector* pAreaObjDirector,
                            const PlayerHolder* pPlayerHolder);

    void setDefaultFarClipDistance(f32 distance);
    void setDefaultFarClipDistanceSub(f32 distance);
    void endInit();
    void update();

    const AreaObjDirector* mAreaObjDirector;
    const PlayerHolder* mPlayerHolder;
    AreaObjGroup* mAreaObjGroup = nullptr;
    AreaObj* mCurrentArea = nullptr;
    f32 mFarClipDistance = 7000.0f;
    f32 mDefaultFarClipDistance = 7000.0f;
    f32 mFarClipDistanceSub = 4000.0f;
    f32 mDefaultFarClipDistanceSub = 4000.0f;
    f32 mFarClipRate = 1.0f;
};
}  // namespace al
