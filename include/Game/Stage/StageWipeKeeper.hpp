#pragma once

#include <basis/seadTypes.h>

/**
 * @brief Keeps the wipes (fades, retry wipe, boot wipe...) shown between scenes.
 * @note Only the members used by already-decompiled callers are declared.
 */
class StageWipeKeeper {
public:
    void closeNoResultWipeRestartTitleWhiteFade(bool isSkip);
    bool isCloseEndNoResultWipe() const;
    bool isActiveNoResultWipe() const;
    bool isActiveBootWipe() const;
    void closeWipeFadeBlack(s32 frame);
    bool isCloseEndFadeBlack() const;
    void closeRetryWipe(bool isSkip);
    bool isCloseEndRetryWipe() const;
    void openRetryWipe();
};
