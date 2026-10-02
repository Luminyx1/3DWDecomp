#include "Library/Layout/EuiScreenFactory.hpp"

#include <heap/seadHeapMgr.h>

#include "Library/Layout/EuiScreen.hpp"

namespace al {
/**
 * Allocates the screen name and user data tables, with a last slot reserved for the viewer.
 * @param pHeap heap to allocate the tables in
 * @param screenNum number of screens
 */
void EuiScreenFactory::initialize(sead::Heap* pHeap, s32 screenNum) {
    mScreenNames.tryAllocBuffer(screenNum + 1, pHeap);
    mScreenNames.fill(nullptr);
    mScreenNames[screenNum] = "Viewer";
    mScreenUserData.tryAllocBuffer(screenNum + 1, pHeap);
    mScreenUserData.fill(nullptr);
}

/**
 * Creates a screen.
 * @param pHeap heap to create the screen in
 * @param screenId screen id
 * @return the new screen
 */
eui::Screen* EuiScreenFactory::createScreen(sead::Heap* pHeap, s32 screenId) {
    sead::ScopedCurrentHeapSetter setter(pHeap);
    EuiScreen* screen = new EuiScreen();
    mScreenUserData[screenId] = nullptr;
    return screen;
}

/**
 * Gets the name of a screen.
 * @param screenId screen id
 * @return the screen name
 */
const char* EuiScreenFactory::getScreenName(s32 screenId) const {
    return mScreenNames[screenId];
}

/**
 * Gets the number of screens, without the viewer slot.
 * @return the screen number
 */
s32 EuiScreenFactory::getScreenNum() const {
    return mScreenNames.size() - 1;
}

/**
 * Finds the id of a screen by its name pointer.
 * @param pScreenName screen name
 * @return the screen id, or -1 if not found
 */
s32 EuiScreenFactory::findScreenId(const char* pScreenName) const {
    for (auto it = mScreenNames.begin(); it != mScreenNames.end(); ++it) {
        if (*it == pScreenName) {
            return it.getIndex();
        }
    }

    return -1;
}

/**
 * Assigns a screen name to the first free slot, unless it is already assigned.
 * @param pScreenName screen name
 * @return the screen id, or -1 if there is no free slot
 */
s32 EuiScreenFactory::assignScreenName(const char* pScreenName) {
    s32 screenId = findScreenId(pScreenName);

    if (screenId >= 0) {
        return screenId;
    }

    for (auto it = mScreenNames.begin(); it != mScreenNames.end(); ++it) {
        if (*it == nullptr) {
            *it = pScreenName;
            return it.getIndex();
        }
    }

    return -1;
}
}  // namespace al
