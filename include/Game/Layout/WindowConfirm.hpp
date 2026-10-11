#pragma once
#include "Library/Layout/LayoutActor.hpp"
#include "prim/seadEnum.h"
namespace al { class LayoutInitInfo; }
SEAD_ENUM(WindowConfirmType, Report, OneButton, TwoButton)

// Layout and interface required by course-select dialogs.
class WindowConfirm : public al::LayoutActor {
public:
    WindowConfirm(WindowConfirmType type, const al::LayoutInitInfo& rInfo, const char* pArchive, bool flag);
    void appearWithSystemMessage(const char* pLayout, const char* pMessage, int inputPort, const char* pButton);
    bool isDecideOne() const;
    bool isDecideLeft() const;
    bool isDecideRight() const;
private:
    void* mpButtonGroup;
    int mSelectedButton;
    int mInputPort;
    bool mIsDecided;
    bool mFlag;
};
static_assert(sizeof(WindowConfirm) == 0x140);
