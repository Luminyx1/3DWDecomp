#pragma once

#include <basis/seadTypes.h>

// Platform glue carried over from the Wii U version; no header exists for these yet.
void ProcUIClearCallbacks();
s32 getPlatformRegion();

/**
 * @brief Main object of the embedded "Luigi Bros." mini-game.
 */
class LuigiBrosMain {
public:
    void init();
    void finalize();
    void control();
    void draw();

private:
    u8 _0[0x10];
};

/**
 * @brief Owner of the Luigi Bros. mini-game: creates, runs and destroys its main object.
 */
class LuigiBrosUtil {
public:
    void initLuigiBros();
    void killLuigiBros();
    void executeLuigiBros();
    void drawLuigiBros();
    static bool isExistLuigiUSaveData(u8* pData, s32 size);

private:
    LuigiBrosMain* mpMain;
};
