#pragma once

#include <nn/types.h>

namespace nn::ui2d {
class LayoutPaneFactory;

struct Ui2dInitializationParamaters {
    void SetDefault();
    LayoutPaneFactory* paneFactory;
    u32 randomSeed;
};

void Initialize(void* (*allocate)(size_t, size_t, void*), void (*free)(void*, void*), void* userData);
void InitializeWithParamaters(void* (*allocate)(size_t, size_t, void*),
                              void (*free)(void*, void*), void* userData,
                              const Ui2dInitializationParamaters& parameters);
}
