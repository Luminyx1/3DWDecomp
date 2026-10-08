#pragma once

#include <basis/seadTypes.h>

/**
 * @brief Control guide bar shown at the bottom of the title / file select screens.
 * @note Only the members used by already-decompiled callers are declared.
 */
class RCSControlGuideBar {
public:
    /** @brief Text sets the guide bar can display. */
    enum GuideBarMsgType : s32 {
        GuideBarMsgType_Title = 8,  ///< Guide text of the title screen.
    };

    void show();
    void hide();
    void appearTitle();
    void endTitle(bool isAnim);
    void appearIcon();
    void endIcon();
    void startSave();
    void startDelete();
    bool isOverlayFinished();
    void changeText(GuideBarMsgType type, s32 port, bool isForce);
};
