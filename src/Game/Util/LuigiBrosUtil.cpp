#include "Util/LuigiBrosUtil.hpp"
#include <heap/seadExpHeap.h>
#include <heap/seadHeapMgr.h>
#include "Library/Memory/SceneHeapSetter.hpp"

/**
 * @brief Creates the Luigi Bros. heap and the mini-game main object inside it, then initializes it.
 */
void LuigiBrosUtil::initLuigiBros() {
    al::SceneHeapSetter sceneHeapSetter;
    sead::ExpHeap* heap = sead::ExpHeap::create(0, "LuigiBrosHeap", nullptr);
    sead::ScopedCurrentHeapSetter heapSetter(heap);

    ProcUIClearCallbacks();
    mpMain = new LuigiBrosMain;
    mpMain->init();
}

/**
 * @brief Finalizes and destroys the Luigi Bros. main object.
 */
void LuigiBrosUtil::killLuigiBros() {
    mpMain->finalize();

    if (mpMain != nullptr) {
        delete mpMain;
        mpMain = nullptr;
    }
}

/**
 * @brief Runs one frame of the Luigi Bros. mini-game.
 */
void LuigiBrosUtil::executeLuigiBros() {
    mpMain->control();
}

/**
 * @brief Draws the Luigi Bros. mini-game.
 */
void LuigiBrosUtil::drawLuigiBros() {
    mpMain->draw();
}

/**
 * @brief Checks whether New Super Luigi U save data exists (Wii U leftover).
 * @param pData Save data buffer (unused on this platform).
 * @param size Size of the save data buffer (unused on this platform).
 * @return Always false: this check is a stub on Switch.
 */
bool LuigiBrosUtil::isExistLuigiUSaveData(u8* pData, s32 size) {
    getPlatformRegion();
    return false;
}
