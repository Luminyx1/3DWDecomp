#include <nn/ui2d/ui2d_ScreenManager.h>
#include <nn/ui2d/ui2d_Screen.h>
namespace nn::ui2d {
ScreenManager::ConstantBufferInitializeArgs::ConstantBufferInitializeArgs() : size(0x100000), memoryPoolRequired(false) {}
// screen is stored in the slot selected by index.
void ScreenManager::RegisterScreen_(Screen* screen, int index) { mScreens[index] = screen; }
// screen is the registered instance whose slot is returned, or -1 if absent.
int ScreenManager::FindScreenId(Screen* screen) const {
    for (int i = 0; i < 8; ++i) if (mScreens[i] == screen) return i;
    return -1;
}
// index selects the registration to clear.
void ScreenManager::UnregisterScreenById_(int index) { mScreens[index] = nullptr; }
// index selects the registration and active ID to reset.
void ScreenManager::ResetScreenId(int index) { mActiveIds[index] = -1; mScreens[index] = nullptr; }
// index selects the screen slot to exclude from active updates.
void ScreenManager::InactivateScreen(int index) { mActiveIds[index] = -1; }
// index selects the screen whose own ID becomes the slot's active ID.
void ScreenManager::ActivateScreen(int index) { mActiveIds[index] = mScreens[index]->mScreenId; }
}
