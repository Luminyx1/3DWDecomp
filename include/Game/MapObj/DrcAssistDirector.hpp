#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace al {
class IUseCamera;
}  // namespace al

namespace rc {
class StampDirector;
}  // namespace rc

class DrcTouchAssistInfo;

/**
 * @brief Scene object driving the touch-screen (DRC) assist: touch pointers, gyro pointing and
 *        the stamp director.
 * @note Minimal declaration: only what the reconstructed code reads is declared so far.
 */
class DrcAssistDirector {
public:
    DrcAssistDirector(s32, bool);

    bool tryCalcTouchPointerSlideDirOnWorld(sead::Vector3f* pDir, const al::IUseCamera* pCamera);
    bool tryCalcTouchPointerSlideDirOnScreen(sead::Vector2f* pDir);
    const DrcTouchAssistInfo* getTouchAssistInfo();

    s32 getTouchPadPort() const { return mTouchPadPort; }

    rc::StampDirector* getStampDirector() const { return mStampDirector; }

private:
    u8 _0[0xcc];
    s32 mTouchPadPort;                   // 0xCC
    u8 _d0[0x8];
    rc::StampDirector* mStampDirector;   // 0xD8
};
