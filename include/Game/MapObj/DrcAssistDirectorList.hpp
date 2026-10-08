#pragma once

#include <basis/seadTypes.h>

#include "Library/Scene/ISceneObj.hpp"

namespace rc {
class StampDirector;
}  // namespace rc

class DrcAssistDirector;

/**
 * @brief Scene object holding the touch-screen (DRC) assist director of every pad port.
 * @note Minimal declaration: only what the reconstructed code uses is declared so far.
 */
class DrcAssistDirectorList : public al::ISceneObj {
public:
    explicit DrcAssistDirectorList(s32 maxNum);

    void addTouchAssist(s32 port, bool isEnable);
    void setEnable(bool isEnable);
    DrcAssistDirector* getDrcAssist(s32 port);
    void setStampDirector(rc::StampDirector* pStampDirector);

private:
    u8 _8[0x40 - 0x8];
};

static_assert(sizeof(DrcAssistDirectorList) == 0x40);
