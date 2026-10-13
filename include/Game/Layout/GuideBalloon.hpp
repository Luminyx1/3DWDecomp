#pragma once

#include "Library/Layout/LayoutActor.hpp"
#include <math/seadVector.h>

namespace al {
class LayoutInitInfo;
class DemoDirector;
}

namespace neko {
enum ColorType : s32;
}  // namespace neko

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
    void startShowNeko(neko::ColorType colorType, int foundNum, int maxNum);
    void startShow(const char16_t* pMessage);
    void endShow();
    const sead::Vector3f& getOffset() const { return mOffset; }

    /**
     * @brief Sets the position the balloon follows.
     * @param pTrans Followed position; must outlive the balloon.
     */
    void setTrans(const sead::Vector3f* pTrans) { mTrans = pTrans; }

private:
    u8 _128[0x8];
    const sead::Vector3f* mTrans;  // 0x130
    sead::Vector3f mOffset;  // 0x138
    u8 _144[0x14];
};

static_assert(sizeof(GuideBalloon) == 0x158);
