#pragma once
#include <nn/types.h>
namespace nn::ui2d {
class Screen;
// Partial interface for existing instances; virtual extension hooks remain unreconstructed.
class ScreenManager {
public:
    struct ConstantBufferInitializeArgs {
        ConstantBufferInitializeArgs();
        size_t size;
        bool memoryPoolRequired;
    };
    virtual ~ScreenManager();
    void RegisterScreen_(Screen* screen, int index);
    int FindScreenId(Screen* screen) const;
    void UnregisterScreenById_(int index);
    void ResetScreenId(int index);
    void InactivateScreen(int index);
    void ActivateScreen(int index);
    u8 _08[0x1f8];
    Screen* mScreens[8];
    int mActiveIds[8];
};
}
