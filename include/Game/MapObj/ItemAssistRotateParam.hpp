#pragma once

#include <basis/seadTypes.h>

/// Parameters of the assist (Drc touch / mic) rotation of items such as coins.
/// Field names are guesses; only the layout is known.
class ItemAssistRotateParam {
public:
    ItemAssistRotateParam();
    ItemAssistRotateParam(s32 frame, f32 speed, bool isUseSubRotate, f32 subSpeed, s32 subFrame);

    s32 mFrame;           // _0
    f32 mSpeed;           // _4
    bool mIsUseSubRotate; // _8
    f32 mSubSpeed;        // _C
    s32 mSubFrame;        // _10
};
