#pragma once

#include "Library/Layout/LayoutActor.hpp"
#include <math/seadVector.h>

namespace al {
class LayoutInitInfo;
class DemoDirector;
}

class GuideBalloon : public al::LayoutActor {
public:
    GuideBalloon(const char* pName, const al::LayoutInitInfo& rInfo,
                 const sead::Vector3f* pTrans, const sead::Vector3f& rOffset,
                 bool isFollow, al::DemoDirector* pDemoDirector);
    GuideBalloon(const char* pName, const char* pLayoutName, const al::LayoutInitInfo& rInfo,
                 const sead::Vector3f* pTrans, const sead::Vector3f& rOffset,
                 bool isFollow, al::DemoDirector* pDemoDirector);

    void startShowGoalItem(int count);
    void startShowDrcTouch(bool);
    void startShowNew();
    void endShow();

    /**
     * @brief Sets the position the balloon follows.
     * @param pTrans Followed position; must outlive the balloon.
     */
    void setTrans(const sead::Vector3f* pTrans) { mTrans = pTrans; }

private:
    u8 _128[0x8];
    const sead::Vector3f* mTrans;  // 0x130
    u8 _138[0x20];
};

static_assert(sizeof(GuideBalloon) == 0x158);
